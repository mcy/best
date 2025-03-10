/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

#ifndef BEST_CONTAINER_VEC_H_
#define BEST_CONTAINER_VEC_H_

#include <cstddef>
#include <initializer_list>

#include "best/base/tags.h"
#include "best/container/box.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/func/arrow.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/math/bit.h"
#include "best/math/overflow.h"
#include "best/memory/allocator.h"
#include "best/memory/layout.h"
#include "best/memory/span.h"
#include "best/meta/init.h"

//! Dynamically sized sequences.
//!
//! best::vec<T> is a vector, a growable, contiguous array, typically residing
//! in the heap.
//!
//! This header also provides concepts and traits for working with contiguous
//! ranges, i.e., ranges that can be represented as spans.

namespace best {
template <best::relocatable, size_t, best::allocator>
class vec;

/// # `best::is_vec`
///
/// Determines whether `V` is some `best::vec<...>`.
template <typename V>
concept is_vec =
  best::same<best::as_auto<V>,
             best::vec<typename V::type, V::MaxInline, typename V::alloc>>;

/// # `best::vec_inline_default()`
///
/// The default inlining parameter for `best::vec`.
template <typename T>
constexpr size_t vec_inline_default() {
  // This is at most 24, so we only need to take a single byte
  // for the size.
  auto best_possible =
    best::layout::of_struct<best::span<T>, size_t>().size() / best::size_of<T>;
  return best::saturating_sub(best_possible, 1);
}

/// # `best::vec<T>`
///
/// A possibly-heap-allocated sequence, with built-in SSO (i.e., inline storage)
/// and custom allocator support. This is the `best` `std::vector`.
///
/// `best::vec` is very similar to `std::vector`, in that it can be constructed
/// from a range or an initializer_list. It readily converts into its
/// corresponding span type. Most common span functions are re-implemented on
/// `best::vec`, for convenience.
///
/// Note that `best::vec` only provides a subset of the `best::span` functions.
/// To access the full suite of span operations, you must access them through
/// `->`, e.g., `vec->sort()`.
template <best::relocatable T, size_t max_inline = vec_inline_default<T>(),
          best::allocator A = best::malloc>
class vec final {
 public:
  /// Helper type aliases.
  using type = T;
  using value_type = best::un_qual<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_raw_ptr<const type>;
  using ptr = best::as_raw_ptr<type>;

  /// # `vec::alloc`
  ///
  /// This vector's allocator type, which may be a reference.
  using alloc = A;

  /// # `vec::MaxInline`
  ///
  /// The maximum number of elements in inline storage.
  static constexpr size_t MaxInline = max_inline;

  /// # `vec::vec()`
  ///
  /// Constructs an empty vector using the given allocator.
  vec() : vec(alloc{}) {}
  explicit vec(alloc alloc) : alloc_(best::in_place, std::move(alloc)) {}

  /// # `vec::vec(range)`
  ///
  /// Constructs an owned copy of a range.
  template <contiguous Range>
  explicit vec(Range&& range)
    requires best::constructible<T, best::data_type<Range>> &&
             best::constructible<alloc>
    : vec(alloc{}, BEST_FWD(range)) {}
  template <contiguous Range>
  vec(alloc alloc, Range&& range)
    requires best::constructible<T, best::data_type<Range>>
    : vec(std::move(alloc)) {
    assign(range);
    // TODO: move optimization?
  }

  /// # `vec::vec(iterator)`
  ///
  /// Constructs a vector by emptying an iterator.
  template <is_iter Iter>
  explicit vec(Iter&& iter)
    requires best::constructible<T, best::iter_type<Iter>> &&
             best::constructible<alloc>
    : vec(alloc{}, BEST_FWD(iter)) {}
  template <is_iter Iter>
  vec(alloc alloc, Iter&& iter)
    requires best::constructible<T, best::iter_type<Iter>>
    : vec(std::move(alloc)) {
    reserve(iter.size_hint().lower);
    for (auto&& elem : iter) { push(BEST_FWD(elem)); }
  }

  /// # `vec::vec{...}`
  ///
  /// Constructs a vector via initializer list.
  vec(std::initializer_list<value_type> range)
    requires best::constructible<alloc> && best::is_object<T>
    : vec(alloc{}, range) {}

  /// # `vec::vec(alloc, {...})`
  ///
  /// Constructs a vector via initializer list, using the given allocator.
  vec(alloc alloc, std::initializer_list<T> range) requires best::is_object<T>
    : vec(std::move(alloc)) {
    assign(range);
  }

  /// # `vec::vec(box)`
  ///
  /// Constructs a vector from an array box, by taking ownership of the
  /// allocation.
  vec(box<T[], alloc>&& box)
    : alloc_(best::in_place, BEST_MOVE(box.allocator())) {
    raw_ = BEST_MOVE(box).leak();
    store_size(~raw_.size());
  }

  /// # `vec::vec(vec)`
  ///
  /// Vectors are copyable if their elements are copyable; vectors are
  /// always moveable.
  vec(const vec& that) requires best::copyable<T> && best::copyable<alloc>;
  vec& operator=(const vec& that)
    requires best::copyable<T> && best::copyable<alloc>;
  vec(vec&& that);
  vec& operator=(vec&& that);

  /// # `vec::~vec()`
  ///
  /// Vectors automatically destroy their contents.
  ~vec() { destroy(); }

  /// # `vec::data()`.
  ///
  /// Returns a pointer to the start of the array this vector manages.
  best::ptr<const T> data() const;
  best::ptr<T> data();

  /// # `vec::size()`.
  ///
  /// Returns the number of elements in this vector.
  size_t size() const;

  /// # `vec::is_empty()`.
  ///
  /// Returns whether this is an empty vector.
  bool is_empty() const { return size() == 0; }

  /// # `vec::capacity()`
  ///
  /// Returns this vector's capacity (the number of elements it can hold before
  /// being forced to resize).
  size_t capacity() const {
    if (auto heap = on_heap()) { return heap->size(); }
    return max_inline;
  }

  /// # `vec::allocator()`
  ///
  /// Returns a reference to his vector's allocator.
  best::as_ref<const alloc> allocator() const { return *alloc_; }
  best::as_ref<alloc> allocator() { return *alloc_; }

  /// # `vec::is_inlined()`, `vec::is_on_heap()`.
  ///
  /// Returns which storage mode this vector is in. Whether the vector stays
  /// in either of these states when mutated is an implementation detail.
  bool is_inlined() const { return on_heap().is_empty(); }
  bool is_on_heap() { return on_heap().has_value(); }

  /// # `vec::as_span()`, `vec::operator->()`
  ///
  /// Returns a span over the array this vector manages.
  ///
  /// All of the span methods, including those not explicitly delegated, are
  /// accessible through `->`. For example, `my_vec->size()` works.
  best::span<const T> as_span() const { return *this; }
  best::span<T> as_span() { return *this; }
  best::arrow<best::span<const T>> operator->() const { return as_span(); }
  best::arrow<best::span<T>> operator->() { return as_span(); }

  /// # `vec::spare_capacity()`
  ///
  /// Returns a span over the uninitialized spare capacity.
  best::span<const T> spare_capacity() const {
    return best::span(data() + size(), capacity() - size());
  }
  best::span<T> spare_capacity() {
    return best::span(data() + size(), capacity() - size());
  }

  /// # `vec::first()`, `vec::last()`
  ///
  /// Returns the first or last element of the vector, or `best::none` if the
  /// vector is empty.
  best::option<const T&> first() const { return as_span().first(); }
  best::option<T&> first() { return as_span().first(); }
  best::option<const T&> last() const { return as_span().last(); }
  best::option<T&> last() { return as_span().last(); }

  /// # `vec[idx]`, `vec[{.start = ...}]`
  ///
  /// Extracts a single element or a subspan. Crashes if the requested index is
  /// out-of-bounds.
  const T& operator[](best::track_location<size_t> idx) const {
    return as_span()[idx];
  }
  T& operator[](best::track_location<size_t> idx) { return as_span()[idx]; }
  best::span<const T> operator[](best::bounds::with_location range) const {
    return as_span()[range];
  }
  best::span<T> operator[](best::bounds::with_location range) {
    return as_span()[range];
  }

  /// # `vec::at(idx)`, `vec::at({.start = ...})`
  ///
  /// Extracts a single element or a subspan. If the requested index is
  /// out-of-bounds, returns `best::none`.
  best::option<const T&> at(size_t idx) const { return as_span().at(idx); }
  best::option<T&> at(size_t idx) { return as_span().at(idx); }
  best::option<const T&> at(best::bounds b) const { return as_span().at(b); }
  best::option<T&> at(best::bounds b) { return as_span().at(b); }

  /// # `vec::citer`, `vec::iter`, `vec::begin()`, `vec::end()`.
  ///
  /// Spans are iterable exactly how you'd expect.
  using const_iterator = best::span<const T>::iterator;
  using iterator = best::span<T>::iterator;
  const_iterator iter() const { return as_span().iter(); }
  iterator iter() { return as_span().iter(); }
  auto begin() const { return as_span().begin(); }
  auto end() const { return as_span().end(); }
  auto begin() { return as_span().begin(); }
  auto end() { return as_span().end(); }

  /// # `vec::to_box()`
  ///
  /// Converts this vector into a box by taking ownership of the allocation.
  best::box<T[], alloc> to_box() && {
    if (is_empty()) { return best::box<T[], alloc>(); }

    spill_to_heap(size(), true);
    auto span = as_span();
    store_size(0);  // This disables the destructor by marking this as an empty
                    // inlined vector.
    return best::box<T[], alloc>(
      unsafe("we are marking this vector completely empty in this function"),
      *BEST_MOVE(alloc_), best::ptr<T[]>(span.data(), span.size()));
  }

  /// # `vec::reserve()`.
  ///
  /// Ensures that pushing an additional `count` elements would not cause this
  /// vector to resize, by resizing the internal array eagerly.
  void reserve(size_t count) { resize_uninit(size() + count); }

  /// # `vec::shrink_to_fit()`
  ///
  /// Resizes the underlying allocation such that `capacity == size`.
  void shrink_to_fit() {
    if (size() == capacity() || !on_heap()) { return; }
    spill_to_heap(size());
  }

  /// # `vec::truncate()`.
  ///
  /// Shortens the vector to be at most `count` elements long.
  /// If `count > size()`, this function does nothing.
  void truncate(size_t count);

  /// # `vec::set_size()`.
  ///
  /// Sets the size of this vector. `new_size` must be less than `capacity()`.
  void set_size(unsafe, size_t new_size);

  /// # `vec::push()`.
  ///
  /// Constructs a new value at the end of this vector, in-place.
  ref push(auto&&... args) requires best::constructible<T, decltype(args)&&...>
  {
    return insert(size(), BEST_FWD(args)...);
  }

  /// # `vec::insert()`.
  ///
  /// Constructs a new value at a specific index of this vector, in-place.
  /// This will need to shift forward all other elements in this vector, which
  /// is O(n) work.
  ref insert(size_t idx, auto&&... args)
    requires best::constructible<T, decltype(args)&&...>
  {
    auto ptr =
      insert_uninit(unsafe("we call construct immediately after this"), idx, 1);
    ptr.construct(BEST_FWD(args)...);
    return *ptr;
  }

  /// # `vec::append()`.
  ///
  /// Appends a range by copying to the end of this vector.
  template <best::contiguous Range = best::span<const T>>
  void append(const Range& range)
    requires best::constructible<T, best::data_type<Range>>
  {
    splice(size(), range);
  }

  /// # `vec::splice()`.
  ///
  /// Splices a range by copying into this vector, starting at idx.
  ///
  /// "Splicing from within" is supported: `range` may be this vector itself or
  /// a subspan thereof.
  template <best::contiguous Range = best::span<const T>>
  void splice(size_t idx, const Range& range)
    requires best::constructible<T, best::data_type<Range>>
  {
    best::span that = range;
    if (as_span().has_subarray(that)) {
      splice_within(idx, that.data().raw() - data().raw(), that.size());
      return;
    }

    auto ptr = insert_uninit(unsafe("we perform copies immediately after this"),
                             idx, that.size());
    best::span(ptr, that.size()).emplace_from(that);
  }

  /// # `vec::clear()`.
  ///
  /// Clears this vector. This resizes it to zero without changing the capacity.
  void clear();

  /// # `best::pop()`
  ///
  /// Removes the last element from this vector, if it is nonempty.
  best::option<T> pop();
  /// # `best::remove()`
  ///
  /// Removes a single element at `idx`. Crashes if `idx` is out of bounds.
  T remove(size_t idx);

  /// # `best::erase()`
  ///
  /// Removes all elements within `bounds` from the vector.
  void erase(best::bounds bounds);

  /// # `vec::assign()`.
  ///
  /// Replaces this vector's elements with the values in `that`, by copying.
  void assign(const contiguous auto& that);

  /// # `vec::insert_uninit()`.
  ///
  /// Prepares a slots for initialization. In particular, this initializes
  /// `count` slots starting at index `start`, which must be in range of
  /// `size()`.
  ///
  /// Returns a pointer to the start of the created range.
  ///
  /// The caller is responsible for ensuring that the slots are actually
  /// initialized before performing further vector operations, such as
  /// pushing or calling the destructor.
  best::ptr<T> insert_uninit(unsafe, size_t start, size_t count);

  /// # `vec::resize_uninit()`
  ///
  /// Resizes this vector such that its size can be set to `new_size`. If
  /// the requested size is smaller than the current size, this destroys
  /// elements as-needed. If the requested size is greater than the capacity,
  /// this resizes the underlying buffer. Otherwise, does nothing.
  void resize_uninit(size_t new_size);

  /// # `vec::spill_to_heap()`
  ///
  /// Forces this vector to be in heap mode instead of inlined mode.
  ///
  /// The caller may also request a larger capacity than the one that would
  /// be ordinarily used. If a hint is specified, the result is guaranteed
  /// to have a backing array at least that large.
  ///
  /// If exact is specified, the new capacity will be exactly `*capacity_hint`
  /// if specified.
  void spill_to_heap(best::option<size_t> capacity_hint = best::none,
                     bool exact = false);

  bool operator==(const contiguous auto& range) const
    requires best::equatable<T, decltype(*best::data(range))>
  {
    return as_span() == best::span(range);
  }
  auto operator<=>(const contiguous auto& range) const
    requires best::equatable<T, decltype(*best::data(range))>
  {
    return as_span() <=> best::span(range);
  }

 private:
  template <best::relocatable, size_t, best::allocator>
  friend class vec;

  // Destroys the underlying array and its elements.
  void destroy();

  // Implements the move constructor and move assignment, which share a lot of
  // code but are not identical.
  void move_construct(vec&&, bool assign);

  void splice_within(size_t idx, size_t start, size_t count);

  best::option<best::span<T>> on_heap() const {
    if (best::to_signed(load_size()) < 0) { return raw_; }
    return best::none;
  }

  /// The number of bytes needed to store the size of an inlined vector.
  static constexpr size_t SizeBytes = [] {
    size_t bits = best::bits_for(max_inline);
    // We need to pay one extra bit, which is the sign bit of the MSB of size_,
    // which is used to tell whether or not this is an inlined string or not.
    return best::ceildiv(bits, 8).wrap();
  }();

  /// The amount of padding after raw_ to get the total needed space for
  /// inlining.
  static constexpr size_t Padding = [] {
    // We assume size_t and best::span<T> have the same alignment for
    // simplicity. This means there is no padding between raw_ and size_ except
    // for the padding we add ourselves.
    static_assert(align_of<size_t> == align_of<best::span<T>>);

    // We need to include these many bytes in our inline region.
    size_t total = (best::overflow(size_of<T>) * max_inline).strict();

    // We can subtract off the ones we get from size.
    total = best::saturating_sub(total, size_of<best::span<T>>);

    // size_ also gives us some bytes, but we need to keep some for the length.
    size_t from_size = size_of<size_t> - SizeBytes;
    total = best::saturating_sub(total, from_size);

    // What's left is the number of padding bytes necessary.
    return total;
  }();

  using padding = best::select<Padding == 0, best::empty,
                               // Can't write char[0] so we make sure that
                               // when padding == 0 we get char[1].
                               char[Padding + (Padding == 0)]>;

  // How big the area where inlined values live is, including the size.
  size_t inlined_region_size() const {
    return reinterpret_cast<const char*>(
             &size_do_not_use_directly_or_else_ub_) -
           reinterpret_cast<const char*>(this) + best::size_of<size_t>;
  }
  size_t load_size() const {
    size_t size;
    std::memcpy(&size, &size_do_not_use_directly_or_else_ub_, sizeof(size));
    return size;
  }
  void store_size(size_t size) {
    std::memcpy(&size_do_not_use_directly_or_else_ub_, &size, sizeof(size));
  }

  static constexpr size_t InternalAlignment =
    max_inline > 0 && best::align_of<T> > best::align_of<best::span<T>>
      ? best::align_of<T>
      : best::align_of<best::span<T>>;

  alignas(InternalAlignment) best::span<T> raw_;
  [[no_unique_address]] padding padding_;
  ssize_t  // Signed so we get "reasonable"
           // debugging prints in gdb.
    size_do_not_use_directly_or_else_ub_ = 0;
  [[no_unique_address]] best::object<alloc> alloc_;
};

template <typename T>
vec(std::initializer_list<T>) -> vec<T>;
template <contiguous Range>
vec(Range) -> vec<best::as_auto<typename best::data_type<Range>>>;
template <is_iter Iter>
requires (!contiguous<Iter>)
vec(Iter) -> vec<best::as_auto<typename best::iter_type<Iter>>>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
namespace iter_internal {
template <typename T>
struct vec {
  using type = best::vec<T>;
};
}  // namespace iter_internal

template <best::relocatable T, size_t max_inline, best::allocator A>
vec<T, max_inline, A>::vec(const vec& that)
  requires best::copyable<T> && best::copyable<alloc>
  : vec(that.allocator()) {
  assign(that);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
auto vec<T, max_inline, A>::operator=(const vec& that)
  -> vec& requires best::copyable<T> && best::copyable<alloc>
{
  assign(that);
  return *this;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
vec<T, max_inline, A>::vec(vec&& that) : vec(std::move(that.allocator())) {
  move_construct(std::move(that), false);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
auto vec<T, max_inline, A>::operator=(vec&& that) -> vec& {
  move_construct(std::move(that), true);
  return *this;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::move_construct(vec&& that, bool assign) {
  if (best::equal(this, &that)) { return; }

  if (auto heap = that.on_heap()) {
    destroy();

    // construct_at because raw_ may contain garbage.
    std::construct_at(&raw_, *heap);
    store_size(~that.size());  // Need to explicitly flip to on-heap mode here.
  } else if (best::relocatable<T, trivially>) {
    destroy();

    // This copies the values and the size, too.
    std::memcpy(this, &that, inlined_region_size());
  } else if (!assign || alloc_ == that.alloc_) {
    auto old_size = size();
    auto new_size = that.size();
    resize_uninit(new_size);
    if (old_size < new_size) {
      data().relo_assign(that.data(), old_size);
      (data() + old_size).relo(that.data() + old_size, new_size - old_size);
    } else {
      data().relo_assign(that.data(), new_size);
    }
  } else {
    destroy();
    auto new_size = that.size();
    reserve(new_size);
    data().relo(that.data(), new_size);
  }

  if (assign) { alloc_ = std::move(that.alloc_); }

  that.store_size(0);  // This resets `that` to being empty and inlined.
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::destroy() {
  clear();
  if (auto heap = on_heap()) {
    alloc_->dealloc(heap->data(), layout::array<T>(capacity()));
  }

  store_size(0);  // This returns `this` to being empty and inlined.
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::ptr<const T> vec<T, max_inline, A>::data() const {
  if (auto heap = on_heap()) { return heap->data(); }
  return best::ptr(this).cast(best::types<const T>);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::ptr<T> vec<T, max_inline, A>::data() {
  if (auto heap = on_heap()) { return heap->data(); }
  return best::ptr(this).cast(best::types<T>);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
size_t vec<T, max_inline, A>::size() const {
  auto size = load_size();
  if constexpr (max_inline == 0) {
    // size_ is implicitly always zero if off-heap.
    return on_heap() ? ~size : 0;
  } else {
    return on_heap() ? ~size : size >> (bits_of<size_t> - SizeBytes * 8);
  }
}
template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::set_size(unsafe, size_t new_size) {
  if (new_size > capacity()) {
    best::crash_internal::crash("set_len(): %zu (new_size) > %zu (capacity)",
                                new_size, capacity());
  }

  if (on_heap()) {
    store_size(~new_size);
  } else {
    if constexpr (max_inline == 0) {
      // size_ is implicitly always zero if off-heap, so we don't have to do
      // anything here.
    } else {
      auto offset = new_size << (bits_of<size_t> - SizeBytes * 8);
      auto mask = ~size_t{0} << (bits_of<size_t> - SizeBytes * 8);

      // XXX: we cannot touch size_ directly; it may have been overwritten
      // partly via the data() pointer, and thus reading through size_ directly
      // is a strict aliasing violation.
      //
      // There are tests that will fail with optimizations turned on if this we
      // are not careful here.
      size_t size = load_size();
      size &= ~mask;
      size |= offset;
      store_size(size);
    }
  }
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::assign(const contiguous auto& that) {
  if (best::equal(this, &that)) { return; }

  using Range = best::un_ref<decltype(that)>;
  if constexpr (best::is_vec<Range>) {
    if (!on_heap() && !that.on_heap() && best::copyable<T, trivially> &&
        best::same<T, typename Range::type> && MaxInline == Range::MaxInline) {
      std::memcpy(this, &that, inlined_region_size());
      return;
    }
  }

  auto that_data = best::ptr(best::data(that));
  auto old_size = size();
  auto new_size = best::size(that);
  resize_uninit(new_size);

  if (old_size < new_size) {
    data().copy_assign(that_data, old_size);
    (data() + old_size).copy(that_data + old_size, new_size - old_size);
  } else {
    data().copy_assign(that_data, new_size);
  }
  set_size(unsafe("updating size to that of the memcpy'd range"), new_size);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::splice_within(size_t idx, size_t start,
                                          size_t count) {
  // If we are self-splicing, we need to make two copies: the outer chunk
  // and the inner chunk. After resizing, this vector looks like this:
  //
  // | xxxxxx | ------ | ------ | yyyyyy |
  //
  // and we want to update it to look like this:
  //
  // | xxxxxx | xxx | yyy | yyyyyy |
  //
  // There are two cases: either `idx` lies inside of the self-spliced
  // range, in which case we need to perform two copies, or it does not,
  // in which case we only perform one copy.

  auto end = start + count;

  insert_uninit(unsafe("we perform copies immediately after this"), idx, count);

  unsafe u(
    "no bounds checks required; has_subarray and insert_uninit verify "
    "all relevant bounds for us");
  if (end <= idx) {
    // The spliced-from region is before the insertion point, so we have
    // one loop.
    as_span()
      .at(u, {.start = idx, .count = count})
      .emplace_from(as_span().at(u, {.start = start, .end = end}));
  } else if (idx < start) {
    // The spliced-from region is after the insertion point. This is the
    // same as above, but we need to offset the slice operation by
    // `that.size()`.

    as_span()
      .at(u, {.start = idx, .count = count})
      .emplace_from(
        as_span().at(u, {.start = start + count, .end = end + count}));
  } else {
    // The annoying case. We need to do the copy in two parts.
    size_t before = idx - start;
    size_t after = count - before;

    as_span()
      .at(u, {.start = idx, .count = before})
      .emplace_from(as_span().at(u, {.start = start, .count = before}));
    as_span()
      .at(u, {.start = idx + before, .count = after})
      .emplace_from(as_span().at(u, {.start = idx + count, .count = after}));
  }
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::truncate(size_t count) {
  if (count > size()) { return; }
  resize_uninit(count);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::clear() {
  if (!best::destructible<T, trivially>) {
    for (size_t i = 0; i < size(); ++i) { (data() + i).destroy(); }
  }
  set_size(unsafe("we just destroyed all elements"), 0);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::option<T> vec<T, max_inline, A>::pop() {
  if (is_empty()) { return best::none; }
  return remove(size() - 1);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
T vec<T, max_inline, A>::remove(size_t idx) {
  T at = std::move(operator[](idx));
  erase({.start = idx, .count = 1});
  return at;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::erase(best::bounds bounds) {
  auto range = operator[](bounds);
  range.destroy();

  size_t start = bounds.start;
  size_t end = bounds.start + range.size();
  size_t len = size() - end;
  as_span().shift_within(
    unsafe("shifting elements over the ones we just destroyed"), start, end,
    len);
  set_size(unsafe("updating length to exclude the range we just deleted"),
           size() - range.size());
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::ptr<T> vec<T, max_inline, A>::insert_uninit(unsafe u, size_t start,
                                                  size_t count) {
  (void)as_span()[{.start = start}];  // Trigger a bounds check.
  if (count == 0) { return data() + start; }

  // TODO(mcyoung): Ideally, this operation should be fused with reserve,
  // since there are cases where we can perform a memmove as-we-go when we
  // resize.
  reserve(count);

  /// Relocate elements to create an empty space.
  auto end = start + count;
  if (start < size()) { as_span().shift_within(u, end, start, size() - start); }
  set_size(u, size() + count);
  return data() + start;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::resize_uninit(size_t new_size) {
  auto old_size = this->size();
  if (new_size <= capacity()) {
    if (new_size < old_size) {
      as_span()
        .at(unsafe{"we just did a bounds check (above)"}, {.start = new_size})
        .destroy();
      set_size(unsafe("elements beyond new_size destroyed above"), new_size);
    }
    return;
  }

  // We do not have sufficient capacity, so we allocate some. This will trigger
  // a spill to heap.
  spill_to_heap(new_size);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::spill_to_heap(best::option<size_t> capacity_hint,
                                          bool exact) {
  if ((on_heap() && !exact && capacity_hint <= capacity()) ||
      (on_heap() && capacity_hint < size()) ||
      (size() == 0 && !capacity_hint)) {
    return;
  }

  size_t new_size = best::max(size(), capacity_hint.value_or(capacity()));
  // Always snap to a power of 2.
  if (!best::is_pow2(new_size) && (!exact || !capacity_hint)) {
    new_size = best::next_pow2(new_size);
  }

  // If we don't hint at a capacity, pick 32 as a "good default".
  if (size() < 32 && !capacity_hint) { new_size = 32; }

  auto old_layout = best::layout::array<T>(capacity());
  auto new_layout = best::layout::array<T>(new_size);

  // If we're on-heap, we can resize somewhat more intelligently.
  if (on_heap() && best::relocatable<T, trivially>) {
    auto grown = alloc_->realloc(data(), old_layout, new_layout);
    // construct_at instead of assignment, since raw_ may contain garbage.
    std::construct_at(&raw_, grown.cast(best::types<T>), new_size);
    return;
  }

  // In the general case, we need to allocate new memory, relocate the values,
  // destroy the moved-from values, and free the old buffer if it is on-heap.
  auto new_data = alloc_->alloc(new_layout).cast(best::types<T>);

  size_t old_size = size();
  new_data.relo(data(), old_size);
  if (on_heap()) { alloc_->dealloc(data(), old_layout); }

  // construct_at instead of assignment, since raw_ may contain garbage, so
  // calling its operator= is UB.
  std::construct_at(&raw_, new_data, new_size);

  store_size(~old_size);  // Update the size to the "on heap" form.
}
}  // namespace best

#endif  // BEST_CONTAINER_VEC_H_
