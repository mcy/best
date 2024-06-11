#ifndef BEST_CONTAINER_VEC_H_
#define BEST_CONTAINER_VEC_H_

#include <algorithm>
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

template <typename V>
concept is_vec =
    best::same<std::remove_cvref_t<V>,
               best::vec<typename V::type, V::MaxInline, typename V::alloc>>;

/// A possibly-heap-allocated sequence.
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
  using type = T;
  using value_type = std::remove_cv_t<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  using alloc = A;

  static constexpr size_t MaxInline = max_inline;

  /// Creates a new, empty vector.
  vec()
    requires best::constructible<alloc>
      : vec(alloc{}) {}

  /// Creates a new, empty vector using the given allocator.
  explicit vec(alloc alloc)
      : size_(0), alloc_(best::in_place, std::move(alloc)) {}

  /// Creates a new vector by copying out of a range.
  template <contiguous Range>
  vec(Range&& range)
    requires best::constructible<T, decltype(*std::data(BEST_FWD(range)))> &&
             (!best::ref_type<T, best::ref_kind::Rvalue>) &&
             best::constructible<alloc>
      : vec(alloc{}, BEST_FWD(range)) {}

  /// Creates a new vector by copying out of a range using the given allocator.
  template <contiguous Range>
  vec(alloc alloc, Range&& range)
    requires best::constructible<T, decltype(*std::data(BEST_FWD(range)))> &&
             (!best::ref_type<T, best::ref_kind::Rvalue>)
  {
    copy_from(range);
    // TODO: move optimization?
  }

  /// Creates a new vector via initializer list.
  vec(std::initializer_list<T> range)
    requires best::constructible<alloc>
      : vec(alloc{}, range) {}

  /// Creates a new vector via initializer list using the given allocator.
  vec(alloc alloc, std::initializer_list<T> range) : vec(std::move(alloc)) {
    copy_from(range);
  }

  vec(const vec& that)
    requires best::copyable<T> && best::copyable<alloc>;
  vec& operator=(const vec& that)
    requires best::copyable<T> && best::copyable<alloc>;
  vec(vec&& that);
  vec& operator=(vec&& that);
  ~vec() { destroy(); }

  /// Returns this vector's data pointer.
  best::object_ptr<const T> data() const;
  best::object_ptr<T> data();

  /// Returns this vector's size.
  size_t size() const;

  /// Sets the size of this vector.
  ///
  /// `new_size` must be less than `capacity()`.
  void set_size(unsafe_t, size_t new_size);

  /// Returns whether this vector is empty.
  bool is_empty() const { return size() == 0; }

  /// Returns this vector's capacity (the number of elements it can hold before
  /// being forced to resize).
  size_t capacity() const {
    if (auto heap = on_heap()) {
      return heap->size();
    }
    return max_inline;
  }

  /// Returns a reference tot his vector's allocator.
  best::as_ref<const alloc> allocator() const { return *alloc_; }
  best::as_ref<alloc> allocator() { return *alloc_; }

  /// Returns which storage mode this vector is in.
  bool is_inlined() const { return on_heap().is_empty(); }
  bool is_on_heap() { return on_heap().has_value(); }

  /// Forces this vector to be in heap mode instead of inlined mode.
  void spill_to_heap();

  /// Returns a span over this vector.
  best::span<const T> as_span() const { return *this; }
  best::span<T> as_span() { return *this; }

  /// Extracts a single element.
  ///
  /// Crashes if the requested index is out-of-bounds.
  cref operator[](best::track_location<size_t> idx) const {
    return as_span()[idx];
  }
  ref operator[](best::track_location<size_t> idx) { return as_span()[idx]; }

  /// Extracts a subspan.
  ///
  /// Crashes if the requested range is out-of-bounds.
  best::span<const T> operator[](best::bounds::with_location range) const {
    return as_span()[range];
  }
  best::span<T> operator[](best::bounds::with_location range) {
    return as_span()[range];
  }

  /// Extracts a single element.
  ///
  /// If the requested index is out-of-bounds, returns best::none.
  best::option<cref> at(size_t idx) const { return as_span().at(idx); }
  best::option<ref> at(size_t idx) { return as_span().at(idx); }

  /// Extracts a subspan.
  ///
  /// If the requested range is out-of-bounds, returns best::none.
  best::option<best::span<const T>> at(best::bounds range) const {
    return as_span().at(range);
  }
  best::option<best::span<T>> at(best::bounds range) {
    return as_span().at(range);
  }

  // Vectors are iterable.
  using citer = best::span<const T>::iter;
  citer begin() const { return as_span().begin(); }
  citer end() const { return as_span().end(); }

  using iter = best::span<T>::iter;
  iter begin() { return as_span().begin(); }
  iter end() { return as_span().end(); }

  /// Constructs a new value at the end of this vector, in-place.
  template <typename... Args>
  ref push(Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    return insert(size(), BEST_FWD(args)...);
  }

  /// Constructs a new value at a specific index of this vector, in-place.
  template <typename... Args>
  ref insert(size_t idx, Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    auto ptr = insert_uninit(best::unsafe, idx, 1);
    ptr.construct_in_place(BEST_FWD(args)...);
    return *ptr;
  }

  /// Clears this vector.
  ///
  /// This resizes it to zero without changing the capacity.
  void clear() {
    if (!best::destructible<T, trivially>) {
      for (size_t i = 0; i < size(); ++i) {
        (data() + i).destroy_in_place();
      }
    }
    set_size(best::unsafe, 0);
  }

  /// Replaces this vector's elements with the values in `that`, by copying.
  template <contiguous Range>
  void copy_from(const Range& that);

  /// Prepares a slots for initialization.
  ///
  /// In particular, this initializes count slots starting at index start,
  /// which must be in range of size().
  ///
  /// Returns a pointer to the start of the created range.
  ///
  /// The caller is responsible for ensuring that the slots are actually
  /// initialized before performing further vector operations, such as
  /// pushing or calling the destructor.
  best::object_ptr<T> insert_uninit(best::unsafe_t, size_t start, size_t count);

  /// Resizes this vector to have new_size elements.
  ///
  /// Destroys any elements that would be truncated off by the resizing, but
  /// does not initialize newly added slots, beware!
  void resize_uninit(best::unsafe_t, size_t new_size);

  /// Ensures that pushing `count` elements would not cause this vector to
  /// resize, by resizing eagerly.
  void reserve(size_t count);

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
    : vec(that.alloc()) {
  copy_from(that);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
auto vec<T, max_inline, A>::operator=(const vec& that) -> vec&
  requires best::copyable<T> && best::copyable<alloc>
{
  copy_from(that);
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
    resize_uninit(best::unsafe, new_size);
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
void vec<T, max_inline, A>::set_size(unsafe_t, size_t new_size) {
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
template <contiguous Range>
void vec<T, max_inline, A>::copy_from(const Range& that) {
  if (best::addr_eq(this, &that)) return;

  if constexpr (best::is_vec<Range>) {
    if (!that.on_heap() && best::copyable<T, trivially> &&
        best::same<T, typename Range::type> && MaxInline == Range::MaxInline) {
      std::memcpy(this, &that, that.size() * size_of<T>);
      set_size(best::unsafe, that.size());
      return;
    }
  }

  auto old_size = size();
  auto new_size = std::size(that);
  resize_uninit(best::unsafe, new_size);

  for (size_t i = 0; i < new_size; ++i) {
    (data() + i).copy_from(std::data(that) + i, /*is_init=*/i < old_size);
  }
}

template <best::relocatable T, size_t max_inline, best::allocator A>
best::object_ptr<T> vec<T, max_inline, A>::insert_uninit(best::unsafe_t,
                                                         size_t start,
                                                         size_t count) {
  (void)as_span()[{.start = start}];  // Trigger a bounds check.
  if (count == 0) return data() + start;

  // TODO(mcyoung): Ideally, this operation should be fused with reserve,
  // since there are cases where we can perform a memmove as-we-go when we
  // resize.
  reserve(count);

  /// Relocate elements to create an empty space.
  auto end = start + count;
  auto sz = size();
  if (best::relocatable<T, trivially> && start < sz) {
    std::memmove(data() + end, data() + start, best::size_of<T> * count);
  } else {
    for (size_t i = start; i < end && i < sz; ++i) {
      (data() + i + count).construct_in_place(std::move(data()[i]));
      (data() + i).destroy_in_place();
    }
  }
  set_size(best::unsafe, sz + count);
  return data() + start;
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::resize_uninit(best::unsafe_t, size_t new_size) {
  auto size = this->size();
  if (new_size < size) {
    for (size_t i = new_size; i < size; ++i) {
      (data() + i).destroy_in_place();
    }
  } else if (new_size > size) {
    reserve(new_size - size);
  }
  set_size(best::unsafe, new_size);
}
template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::spill_to_heap() {
  if (on_heap()) {
    return;
  }

  // Snap to a new power of 2, or 32, whichever is smaller.
  size_t new_size = best::wrapping_next_pow2(capacity());
  if (size() < 32) {
    new_size = 32;
  }
  auto new_layout = best::layout::array<T>(new_size);

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
  // construct_at instead of assignment, since raw_ may contain garbage.
  std::construct_at(&raw_, new_data, new_size);
  size_ = ~old_size;  // Update the size to the "on heap" form.
}

template <best::relocatable T, size_t max_inline, best::allocator A>
void vec<T, max_inline, A>::reserve(size_t count) {
  if (count == 0) return;

  auto requested_bytes = (best::overflow(size()) + count) * size_of<T>;
  if (requested_bytes.overflowed ||
      requested_bytes.value > max_of<size_t> / 2) {
    best::crash_internal::crash(
        "attempted to allocate more than max_of<size_t>/2 bytes");
  }

  size_t new_size = size() + count;
  if (capacity() >= new_size) {
    return;
  }

  // Always snap to a new power of 2.
  new_size = best::wrapping_next_pow2(new_size);

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
  // construct_at instead of assignment, since raw_ may contain garbage.
  std::construct_at(&raw_, new_data, new_size);
  size_ = ~old_size;  // Update the size to the "on heap" form.
}
}  // namespace best

#endif  // BEST_CONTAINER_VEC_H_