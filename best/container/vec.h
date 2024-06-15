#ifndef BEST_CONTAINER_VEC_H_
#define BEST_CONTAINER_VEC_H_

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>

#include "best/container/object.h"
#include "best/container/option.h"
#include "best/container/span.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/math/bit.h"
#include "best/math/overflow.h"
#include "best/memory/allocator.h"
#include "best/memory/layout.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

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
    best::same<std::remove_cvref_t<V>,
               best::vec<typename V::type, V::MaxInline, typename V::alloc>>;

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
/// In addition to all of the `best::span` functions, `best::vec` offers the
/// usual complement of push, insert, remove, etc. functionality.
template <best::relocatable T,
          size_t max_inline =
              [] {
                // This is at most 24, so we only need to take a single byte
                // for the size.
                auto best_possible =
                    best::layout::of_struct<best::span<T>, size_t>().size() /
                    best::size_of<T>;
                return best::saturating_sub(best_possible, 1);
              }(),
          best::allocator A = best::malloc>
class vec final {
 public:
  /// Helper type aliases.
  using type = T;
  using value_type = std::remove_cv_t<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

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
  /// Constructs an empty vector.
  vec()
    requires best::constructible<alloc>
      : vec(alloc{}) {}

  /// # `vec::vec(alloc)`
  ///
  /// Constructs an empty vector using the given allocator.
  explicit vec(alloc alloc)
      : size_(0), alloc_(best::in_place, std::move(alloc)) {}

  /// # `vec::vec(range)`
  ///
  /// Constructs an owned copy of a range.
  template <contiguous Range>
  explicit vec(Range&& range)
    requires best::constructible<T, decltype(*std::data(BEST_FWD(range)))> &&
             (!best::ref_type<T, best::ref_kind::Rvalue>) &&
             best::constructible<alloc>
      : vec(alloc{}, BEST_FWD(range)) {}

  /// # `vec::vec(alloc, range)`
  ///
  /// Constructs an owned copy of a range using the given allocator.
  template <contiguous Range>
  vec(alloc alloc, Range&& range)
    requires best::constructible<T, decltype(*std::data(BEST_FWD(range)))> &&
             (!best::ref_type<T, best::ref_kind::Rvalue>)
  {
    assign(range);
    // TODO: move optimization?
  }

  /// # `vec::vec{...}`
  ///
  /// Constructs a vector via initializer list.
  vec(std::initializer_list<value_type> range)
    requires best::constructible<alloc> && best::object_type<T>
      : vec(alloc{}, range) {}

  /// # `vec::vec(alloc, {...})`
  ///
  /// Constructs a vector via initializer list, using the given allocator.
  vec(alloc alloc, std::initializer_list<T> range)
    requires best::object_type<T>
      : vec(std::move(alloc)) {
    assign(range);
  }

  /// # `vec::vec(vec)`
  ///
  /// Vectors are copyable if their elements are copyable; vectors are
  /// always moveable.
  vec(const vec& that)
    requires best::copyable<T> && best::copyable<alloc>;
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
  best::object_ptr<const T> data() const;
  best::object_ptr<T> data();

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
    if (auto heap = on_heap()) {
      return heap->size();
    }
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

  /// # `vec::as_span()`
  ///
  /// Returns a span over the array this vector manages.
  best::span<const T> as_span() const { return *this; }
  best::span<T> as_span() { return *this; }

  /// # `vec::first()`
  ///
  /// Returns the first, or first `m`, elements of this vector, or `best::none`
  /// if there are not enough elements.
  best::option<const T&> first() const { return as_span().first(); }
  best::option<T&> first() { return as_span().first(); }
  template <size_t m>
  best::option<best::span<const T, m>> first(best::index_t<m> i = {}) const {
    return as_span().first(i);
  }
  template <size_t m>
  best::option<best::span<T, m>> first(best::index_t<m> i = {}) {
    return as_span().first(i);
  }

  /// # `vec::last()`
  ///
  /// Returns the last, or last `m`, elements of this vector, or `best::none` if
  /// there are not enough elements.
  best::option<const T&> last() const { return as_span().last(); }
  best::option<T&> last() { return as_span().last(); }
  template <size_t m>
  best::option<best::span<const T, m>> last(best::index_t<m> i = {}) const {
    return as_span().last(i);
  }
  template <size_t m>
  best::option<best::span<T, m>> last(best::index_t<m> i = {}) {
    return as_span().last(i);
  }

  /// # `vec[idx]`
  ///
  /// Extracts a single element. Crashes if the requested index is
  /// out-of-bounds.
  const T& operator[](best::track_location<size_t> idx) const {
    return as_span()[idx];
  }
  T& operator[](best::track_location<size_t> idx) { return as_span()[idx]; }

  /// # `vec[{.start = ...}]`
  ///
  /// Extracts a subspan. Crashes if the requested range is out-of-bounds.
  best::span<const T> operator[](best::bounds::with_location range) const {
    return as_span()[range];
  }
  best::span<T> operator[](best::bounds::with_location range) {
    return as_span()[range];
  }

  /// # `vec::at(idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// returns best::none.
  best::option<const T&> at(size_t idx) const { return as_span().at(idx); }
  best::option<T&> at(size_t idx) { return as_span().at(idx); }

  /// # `vec::at(unsafe, idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// Undefined Behavior.
  const T& at(unsafe u, size_t idx) const { return as_span().at(u, idx); }
  T& at(unsafe u, size_t idx) { return as_span().at(u, idx); }

  /// # `vec::at({.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds, returns
  /// best::none.
  best::option<best::span<const T>> at(best::bounds range) const {
    return as_span().at(range);
  }
  best::option<best::span<T>> at(best::bounds range) {
    return as_span().at(range);
  }

  /// # `vec::at(unsafe, {.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds, returns
  /// best::none.
  best::span<const T> at(unsafe u, best::bounds range) const {
    return as_span().at(u, range);
  }
  best::span<T> at(unsafe u, best::bounds range) {
    return as_span().at(u, range);
  }

  /// # `vec::reverse()`
  ///
  /// Reverses the order of the elements in this vector, in-place.
  void reverse() { as_span().reverse(); }

  /// # `vec::contains()`
  ///
  /// Performs a linear search for a matching element.
  bool contains(const best::equatable<T> auto& needle) const {
    return as_span().contains(needle);
  }

  /// # `vec::starts_with()`
  ///
  /// Checks if this vector starts with a particular pattern.
  template <best::equatable<T> U = T>
  constexpr bool starts_with(best::span<const U> needle) const {
    return as_span().starts_with(needle);
  }

  /// # `vec::ends_with()`
  ///
  /// Checks if this vector ends with a particular pattern.
  template <best::equatable<T> U = T>
  constexpr bool ends_with(best::span<const U> needle) const {
    return as_span().ends_with(needle);
  }

  /// # `vec::strip_prefix()`
  ///
  /// If this vector starts with `prefix`, removes it and returns the rest;
  /// otherwise returns `best::none`.
  template <best::equatable<T> U = T>
  best::option<best::span<const T>> strip_prefix(
      best::span<const U> prefix) const {
    return as_span().strip_prefix(prefix);
  }
  template <best::equatable<T> U = T>
  best::option<best::span<T>> strip_prefix(best::span<const U> prefix) {
    return as_span().strip_prefix(prefix);
  }

  /// # `vec::strip_suffix()`
  ///
  /// If this vector ends with `suffix`, removes it and returns the rest;
  /// otherwise returns `best::none`.
  template <best::equatable<T> U = T>
  best::option<best::span<const T>> strip_suffix(
      best::span<const U> suffix) const {
    return as_span().strip_prefix(suffix);
  }
  template <best::equatable<T> U = T>
  best::option<best::span<T>> strip_suffix(best::span<const U> suffix) {
    return as_span().strip_prefix(suffix);
  }

  /// # `span::sort()`
  ///
  /// Sorts the vector in place. See span::sort() for more information on
  /// the three overloads.
  ///
  /// Because this is implemented using the <algorithm> header, which would
  /// pull in a completely unacceptable amount of this, the implementations of
  /// these functions live in `//best/container/span_sort.h`, which must be
  /// included separately.
  void sort()
    requires best::comparable<T>
  {
    as_span().sort();
  }
  void sort(best::callable<void(const T&)> auto&& get_key) {
    as_span().sort(BEST_FWD(get_key));
  }
  void sort(
      best::callable<std::partial_ordering(const T&, const T&)> auto&& cmp) {
    as_span().sort(BEST_FWD(cmp));
  }

  /// # `vec::stable_sort()`
  ///
  /// Identical to `sort()`, but uses a stable sort which guarantees that equal
  /// items are not reordered past each other. This usually means the algorithm
  /// is slower.
  void stable_sort()
    requires best::comparable<T>
  {
    as_span().stable_sort();
  }
  void stable_sort(best::callable<void(const T&)> auto&& get_key) {
    as_span().stable_sort(BEST_FWD(get_key));
  }
  void stable_sort(
      best::callable<std::partial_ordering(const T&, const T&)> auto&& cmp) {
    as_span().stable_sort(BEST_FWD(cmp));
  }

  /// # `vec::copy_from()`
  ///
  /// Copies values from src. This has the same semantics as Go's `copy()`
  /// builtin: if the lengths are not equal, only the overlapping part is
  /// copied.
  template <typename U = T>
  void copy_from(best::span<const U> src) {
    as_span().copy_from(src);
  }

  /// # `vec::citer`, `vec::iter`, `vec::begin()`, `vec::end()`.
  ///
  /// Spans are iterable exactly how you'd expect.
  using citer = best::span<const T>::iter;
  using iter = best::span<T>::iter;
  citer begin() const { return as_span().begin(); }
  citer end() const { return as_span().end(); }
  iter begin() { return as_span().begin(); }
  iter end() { return as_span().end(); }

  /// # `vec::reserve()`.
  ///
  /// Ensures that pushing an additional `count` elements would not cause this
  /// vector to resize, by resizing the internal array eagerly.
  void reserve(size_t count) {
    unsafe::in([&](auto u) { resize_uninit(u, size() + count); });
  }

  /// # `vec::truncate()`.
  ///
  /// Shortens the vector to be at most `count` elements long.
  /// If `count > size()`, this function does nothing.
  void truncate(size_t count) {
    if (count > size()) return;
    unsafe::in([&](auto u) { resize_uninit(u, count); });
  }

  /// # `vec::set_size()`.
  ///
  /// Sets the size of this vector. `new_size` must be less than `capacity()`.
  void set_size(unsafe, size_t new_size);

  /// # `vec::push()`.
  ///
  /// Constructs a new value at the end of this vector, in-place.
  template <typename... Args>
  ref push(Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    return insert(size(), BEST_FWD(args)...);
  }

  /// # `vec::insert()`.
  ///
  /// Constructs a new value at a specific index of this vector, in-place.
  /// This will need to shift forward all other elements in this vector, which
  /// is O(n) work.
  template <typename... Args>
  ref insert(size_t idx, Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    auto ptr = unsafe::in([&](auto u) { return insert_uninit(u, idx, 1); });
    ptr.construct_in_place(BEST_FWD(args)...);
    return *ptr;
  }

  /// # `vec::append()`.
  ///
  /// Appends a range by copying to the end of this vector.
  template <best::contiguous Range = best::span<const T>>
  void append(const Range& range)
    requires best::constructible<T, decltype(*std::data(range))>
  {
    splice(size(), range);
  }

  /// # `vec::splice()`.
  ///
  /// Splices a range by copying into this vector, starting at idx.
  template <best::contiguous Range = best::span<const T>>
  void splice(size_t idx, const Range& range)
    requires best::constructible<T, decltype(*std::data(range))>
  {
    auto ptr = unsafe::in(
        [&](auto u) { return insert_uninit(u, idx, std::size(range)); });
    best::span(ptr, std::size(range)).emplace_from(best::span(range));
  }

  /// # `vec::clear()`.
  ///
  /// Clears this vector. This resizes it to zero without changing the capacity.
  void clear() {
    if (!best::destructible<T, trivially>) {
      for (size_t i = 0; i < size(); ++i) {
        (data() + i).destroy_in_place();
      }
    }
    unsafe::in([&](auto u) { set_size(u, 0); });
  }

  /// # `best::pop()`
  ///
  /// Removes the last element from this vector, if it is nonempty.
  best::option<T> pop() {
    if (is_empty()) return best::none;
    return remove(size() - 1);
  }

  /// # `best::remove()`
  ///
  /// Removes a single element at `idx`. Crashes if `idx` is out of bounds.
  T remove(size_t idx) {
    T at = std::move(operator[](idx));
    erase({.start = idx, .count = 1});
    return at;
  }

  /// # `best::erase()`
  ///
  /// Removes all elements within `bounds` from the vector.
  void erase(best::bounds bounds) {
    auto range = operator[](bounds);
    range.destroy_in_place();
    best::unsafe::in([&](auto u) {
      shift_within(u, bounds.start, bounds.start + range.size(),
                   size() - bounds.start + range.size());
      set_size(u, size() - range.size());
    });
  }

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
  best::object_ptr<T> insert_uninit(unsafe, size_t start, size_t count);

  /// # `vec::resize_uninit()`
  ///
  /// Resizes this vector such that its size can be set to `new_size`. If
  /// the requested size is smaller than the current size, this destroys
  /// elements as-needed. If the requested size is greater than the capacity,
  /// this resizes the underlying buffer. Otherwise, does nothing.
  void resize_uninit(unsafe, size_t new_size);

  /// # `vec::shift_within()`
  ///
  /// Performs an internal `memmove()`. This relocates `count` elements starting
  /// at `src` to `dst`.
  ///
  /// NOTE! This function assumes that the destination range is uninitialized,
  /// *and* that the source range is initialized. It will not update the
  /// size of the vector; the caller is responsible for doing that themselves.
  void shift_within(unsafe u, size_t dst, size_t src, size_t count) {
    as_span().shift_within(u, dst, src, count);
  }

  /// # `vec::spill_to_heap()`
  ///
  /// Forces this vector to be in heap mode instead of inlined mode. The vector
  /// may ignore this request.
  ///
  /// The caller may also request a larger capacity than the one that would
  /// be ordinarily used. If a hint is specified, the result is guaranteed
  /// to have a backing array at least that large.
  void spill_to_heap(best::option<size_t> capacity_hint = best::none);

  bool operator==(const contiguous auto& range) const
    requires best::equatable<T, decltype(*std::data(range))>
  {
    return as_span() == best::span(range);
  }
  auto operator<=>(const contiguous auto& range) const
    requires best::equatable<T, decltype(*std::data(range))>
  {
    return as_span() <=> best::span(range);
  }

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, const vec& sp) {
    return os << sp.as_span();
  }

 private:
  template <best::relocatable, size_t, best::allocator>
  friend class vec;

  // Destroys the underlying array and its elements.
  void destroy();

  // Implements the move constructor and move assignment, which share a lot of
  // code but are not identical.
  void move_construct(vec&&, bool assign);

  best::option<best::span<T>> on_heap() const {
    if (best::to_signed(size_) < 0) {
      return raw_;
    }
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

  using padding = std::conditional_t<Padding == 0, best::empty,
                                     // Can't write char[0] so we make sure that
                                     // when padding == 0 we get char[1].
                                     char[Padding + (Padding == 0)]>;

  // How big the area where inlined values live is, including the size.
  size_t inlined_region_size() const {
    return reinterpret_cast<const char*>(&size_) -
           reinterpret_cast<const char*>(this) + best::size_of<size_t>;
  }

  static constexpr size_t InternalAlignment =
      max_inline > 0 && best::align_of<T> > best::align_of<best::span<T>>
          ? best::align_of<T>
          : best::align_of<best::span<T>>;

  alignas(InternalAlignment) best::span<T> raw_;
  [[no_unique_address]] padding padding_;
  ssize_t size_;  // Signed so we get "reasonable" debugging prints in gdb.
  [[no_unique_address]] best::object<alloc> alloc_;
};

template <typename T>
vec(std::initializer_list<T>) -> vec<T>;

// --- IMPLEMENTATION DETAILS BELOW ---

template <best::relocatable T, size_t max_inline, best::allocator A>
vec<T, max_inline, A>::vec(const vec& that)
  requires best::copyable<T> && best::copyable<alloc>
    : vec(that.allocator()) {
  assign(that);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
auto vec<T, max_inline, A>::operator=(const vec& that) -> vec&
  requires best::copyable<T> && best::copyable<alloc>
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
  if (best::addr_eq(this, &that)) return;

  if (auto heap = that.on_heap()) {
    destroy();

    // construct_at because raw_ may contain garbage.
    std::construct_at(&raw_, *heap);
    size_ = ~that.size();  // Need to explicitly flip to on-heap mode here.
  } else if (best::relocatable<T, trivially>) {
    destroy();

    // This copies the values and the size, too.
    std::memcpy(this, &that, inlined_region_size());
  } else if (!assign || alloc_ == that.alloc_) {
    auto old_size = size();
    auto new_size = that.size();
    best::unsafe::in([&](auto u) { resize_uninit(u, new_size); });
    for (size_t i = 0; i < new_size; ++i) {
      (data() + i).relocate_from(that.data() + i, /*is_init=*/i < old_size);
    }
  } else {
    destroy();
    auto new_size = that.size();
    reserve(new_size);
    for (size_t i = 0; i < new_size; ++i) {
      (data() + i).relocate_from(that.data() + i, false);
    }
  }

  if (assign) {
    alloc_ = std::move(that.alloc_);
  }

  that.size_ = 0;  // This resets `that` to being empty and inlined.
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::destroy() {
  clear();
  if (auto heap = on_heap()) {
    alloc_->dealloc(heap->data(), layout::array<T>(capacity()));
  }

  size_ = 0;  // This returns `this` to being empty and inlined.
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::object_ptr<const T> vec<T, max_inline, A>::data() const {
  if (auto heap = on_heap()) {
    return heap->data();
  }
  return reinterpret_cast<const best::object_ptr<T>::pointee*>(this);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::object_ptr<T> vec<T, max_inline, A>::data() {
  if (auto heap = on_heap()) {
    return heap->data();
  }
  return reinterpret_cast<best::object_ptr<T>::pointee*>(this);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
size_t vec<T, max_inline, A>::size() const {
  return on_heap() ? ~size_ : size_ >> (bits_of<size_t> - SizeBytes * 8);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::set_size(unsafe, size_t new_size) {
  if (new_size > capacity()) {
    best::crash_internal::crash("set_len(): %zu (new_size) > %zu (capacity)",
                                new_size, capacity());
  }

  if (on_heap()) {
    size_ = ~new_size;
  } else {
    auto offset = new_size << (bits_of<size_t> - SizeBytes * 8);
    auto mask = ~size_t{0} << (bits_of<size_t> - SizeBytes * 8);

    // TODO(mcyoung): This is a strict aliasing violation...
    size_ &= ~mask;
    size_ |= offset;
  }
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::assign(const contiguous auto& that) {
  if (best::addr_eq(this, &that)) return;
  best::unsafe::in([&](auto u) {
    using Range = best::as_deref<decltype(that)>;
    if constexpr (best::is_vec<Range>) {
      if (!that.on_heap() && best::copyable<T, trivially> &&
          best::same<T, typename Range::type> &&
          MaxInline == Range::MaxInline) {
        std::memcpy(this, &that, that.size() * size_of<T>);
        set_size(u, that.size());
        return;
      }
    }

    auto old_size = size();
    auto new_size = std::size(that);
    resize_uninit(u, new_size);

    for (size_t i = 0; i < new_size; ++i) {
      (data() + i).copy_from(std::data(that) + i, /*is_init=*/i < old_size);
    }
    set_size(u, new_size);
  });
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::object_ptr<T> vec<T, max_inline, A>::insert_uninit(unsafe u, size_t start,
                                                         size_t count) {
  (void)as_span()[{.start = start}];  // Trigger a bounds check.
  if (count == 0) return data() + start;

  // TODO(mcyoung): Ideally, this operation should be fused with reserve,
  // since there are cases where we can perform a memmove as-we-go when we
  // resize.
  reserve(count);

  /// Relocate elements to create an empty space.
  auto end = start + count;
  if (start < size()) {
    shift_within(u, end, start, count);
  }
  set_size(u, size() + count);
  return data() + start;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::resize_uninit(unsafe u, size_t new_size) {
  auto old_size = this->size();
  if (new_size <= capacity()) {
    if (new_size < old_size) {
      at(u, {.start = new_size}).destroy_in_place();
    }
    return;
  }

  // We do not have sufficient capacity, so we allocate some. This will trigger
  // a spill to heap.
  spill_to_heap(new_size);
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::spill_to_heap(best::option<size_t> capacity_hint) {
  if ((on_heap() && capacity_hint <= capacity()) ||
      (size() == 0 && !capacity_hint)) {
    return;
  }

  size_t new_size = best::max(capacity(), capacity_hint.value_or(0));
  // Always snap to a power of 2.
  if (!best::is_pow2(new_size)) {
    new_size = best::next_pow2(new_size);
  }

  // If we don't hint at a capacity, pick 32 as a "good default".
  if (size() < 32 && !capacity_hint) {
    new_size = 32;
  }

  auto old_layout = best::layout::array<T>(capacity());
  auto new_layout = best::layout::array<T>(new_size);

  // If we're on-heap, we can resize somewhat more intelligently.
  if (on_heap() && best::relocatable<T, trivially>) {
    void* grown = alloc_->realloc(data(), old_layout, new_layout);
    // construct_at instead of assignment, since raw_ may contain garbage.
    std::construct_at(&raw_, static_cast<best::object_ptr<T>::pointee*>(grown),
                      new_size);
    return;
  }

  // In the general case, we need to allocate new memory, relocate the values,
  // destroy the moved-from values, and free the old buffer if it is on-heap.
  best::object_ptr<T> new_data =
      static_cast<best::object_ptr<T>::pointee*>(alloc_->alloc(new_layout));

  size_t old_size = size();
  if (best::relocatable<T, trivially>) {
    std::memcpy(new_data, data(), old_size * size_of<T>);
  } else {
    for (size_t i = 0; i < old_size; ++i) {
      (new_data + i).move_from(data() + i, false);
    }
  }

  destroy();  // Deallocate the old block.

  // construct_at instead of assignment, since raw_ may contain garbage, so
  // calling its operator= is UB.
  std::construct_at(&raw_, new_data, new_size);

  size_ = ~old_size;  // Update the size to the "on heap" form.
}
}  // namespace best

#endif  // BEST_CONTAINER_VEC_H_