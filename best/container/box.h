#ifndef BEST_CONTAINER_BOX_H_
#define BEST_CONTAINER_BOX_H_

#include <utility>

#include "best/base/hint.h"
#include "best/base/tags.h"
#include "best/memory/allocator.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/init.h"

//!

namespace best {
/// # `best::box<T>`
///
/// A non-null pointer to a value on the heap.
template <typename T, typename A = best::malloc>
class BEST_RELOCATABLE box final {
 public:
  /// # `ptr::type`.
  ///
  /// The wrapped type; `ptr<T>` is nominally a `T*`.
  using type = T;

  /// # `ptr::pointee`.
  ///
  /// The "true" pointee type. Internally, an `box` stores a `pointee*`.
  using pointee = best::ptr<T>::pointee;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  using alloc = A;

  box() = delete;

  /// # `box::box(box)`
  ///
  /// Trivially relocatable. Copies perform memory allocations.
  box(const box& that)
    requires best::copyable<T> && best::copyable<alloc>
      : box(that.alloc_, best::in_place, *that) {}
  box& operator=(const box& that)
    requires best::copyable<T>
  {
    **this = *that;
  }
  box(box&& that)
    requires best::moveable<alloc>
      : ptr_(std::exchange(that.ptr_), best::ptr<T>::dangling()),
        alloc_(BEST_MOVE(that.alloc_)) {}
  box& operator=(box&& that)
    requires best::moveable<alloc>
  {
    if (best::equal(this, &that)) return;
    this->~box();
    new (this) box(BEST_MOVE(that));
  }

  /// # `box::box(value)`
  ///
  /// Wraps a value by copying/moving out from it.
  explicit box(const T& from) : box(best::in_place, from) {}
  explicit box(T&& from) : box(best::in_place, BEST_MOVE(from)) {}
  explicit box(alloc alloc, const T& from)
      : box(BEST_MOVE(alloc), best::in_place, from) {}
  explicit box(alloc alloc, T&& from)
      : box(BEST_MOVE(alloc), best::in_place, BEST_MOVE(from)) {}

  /// # `box::box(...)`
  ///
  /// Constructs a box by calling a constructor in-place.
  explicit box(best::in_place_t, auto&&... args)
      : box(alloc{}, best::in_place, BEST_FWD(args)...) {}
  explicit box(alloc alloc, best::in_place_t, auto&&... args)
      : alloc_(BEST_MOVE(alloc)) {
    ptr_ = alloc_.alloc(best::layout::of<T>());
    ptr_.construct(BEST_FWD(args)...);
  }

  /// # `box::box(unsafe, ptr)`
  ///
  /// Wraps a pointer in a box. This pointer MUST have been allocated using
  /// the given allocated, with the layout of `T`.
  explicit box(unsafe u, best::ptr<T> ptr) : box(u, alloc{}, ptr) {}
  explicit box(unsafe u, alloc alloc, best::ptr<T> ptr)
      : ptr_(ptr), alloc_(BEST_MOVE(alloc)) {}

  ~box() {
    if (ptr_ == best::ptr<T>::dangling()) return;

    ptr_.destroy();
    alloc_.dealloc(ptr_.raw(), best::layout::of<T>());
  }

  /// # `box::as_ptr()`
  ///
  /// Returns the underlying `best::ptr`.
  best::ptr<T> as_ptr() const { return ptr_; }
  
  /// # `box::allocator()`
  ///
  /// Returns a reference to the box's allocator.
  const alloc& allocator() const { return alloc_; }
  alloc& allocator() { return alloc_; }

  /// # `box::into_raw()`
  ///
  /// Explodes this box into its raw parts, and inhibits the destructor. The
  /// resulting pointer must be freed using the returned allocator at the end of
  /// the object's lifetime.
  best::row<best::ptr<T>, alloc> into_raw() && {
    auto ptr = std::exchange(ptr_, best::ptr<T>::dangling());
    return {ptr, BEST_MOVE(alloc_)};
  }

  /// # `box::operator*, box::operator->`
  ///
  /// Dereferences the box. Note that boxes cannot be null!
  ///
  /// Unlike `std::unique_ptr<T>`, dereferncing preserves the constness and
  /// value category of the box.
  cref operator*() const& { return *ptr_; }
  ref operator*() & { return *ptr_; }
  crref operator*() const&& { return BEST_MOVE(*ptr_); }
  rref operator*() && { return BEST_MOVE(*ptr_); }
  cptr operator->() const { return ptr_.operator->(); }
  ptr operator->() { return ptr_.operator->(); }

  friend void BestFmt(auto& fmt, const box& box)
    requires requires { fmt.format(*box); }
  {
    fmt.format(*box);
  }
  friend constexpr void BestFmtQuery(auto& query, box*) {
    query = query.template of<T>;
  }

  template <best::equatable<T> U>
  bool operator==(const best::box<U>& that) const {
    return **this == *that;
  }
  template <best::equatable<T> U>
  bool operator==(const U& u) const {
    return **this == u;
  }

  template <best::comparable<T> U>
  best::order_type<T, U> operator<=>(const best::box<U>& that) const {
    return **this <=> *that;
  }
  template <best::comparable<T> U>
  best::order_type<T, U> operator<=>(const U& u) const {
    return **this <=> u;
  }

 private:
  best::ptr<T> ptr_;
  [[no_unique_address]] alloc alloc_;
};

template <typename T>
box(T&&) -> box<best::as_auto<T>>;

/// # `best::box<T[]>`
///
/// A non-null pointer to an array on the heap.
template <typename T, typename A>
class BEST_RELOCATABLE box<T[], A> final {

};
}  // namespace best

#endif  // BEST_CONTAINER_BOX_H_