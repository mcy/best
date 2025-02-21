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

#ifndef BEST_CONTAINER_BOX_H_
#define BEST_CONTAINER_BOX_H_

#include "best/base/hint.h"
#include "best/base/ord.h"
#include "best/base/tags.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/memory/allocator.h"
#include "best/memory/dyn.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/init.h"

//! Value boxing.
//!
//! Boxes are pointers to heap-allocated objects, essentially fulfilling the
//! purpose of `std::unique_ptr` but with semantics closer to Rust's `Box<T>`
//! type. In particular, unlike `std::unique_ptr`, they are copyable.

namespace best {
/// # `best::is_box`
///
/// Whether `T` is some `best::box<U>`.
template <typename T>
concept is_box = requires {
  requires !std::is_class_v<typename best::as_auto<T>::type> ||
             !std::is_abstract_v<typename best::as_auto<T>::type>;
  requires best::same<best::as_auto<T>,
                      best::box<typename best::as_auto<T>::type,
                                typename best::as_auto<T>::alloc>>;
};

/// # `best::box<T>`
///
/// A non-null pointer to a value on the heap.
template <typename T, typename A = best::malloc>
class BEST_RELOCATABLE box final {
 public:
  /// # `box::type`
  ///
  /// The wrapped type; `box<T>` is nominally a `T*`.
  using type = T;

  /// # `box::ptr`
  ///
  /// The pointer type for this box.
  using ptr = best::ptr<T>;

  /// # `box::pointee`, `box::meta`
  ///
  /// The pointer component types for this box.
  using pointee = ptr::pointee;
  using metadata = ptr::metadata;

  /// # `box::alloc`
  ///
  /// The allocator type for this box.
  using alloc = A;

 private:
  // Helper for making requires clauses cleaner.
  static constexpr bool thin = ptr::is_thin();

 public:
  /// # `box::box(box)`
  ///
  /// Trivially relocatable. Copies perform memory allocations.
  constexpr box(const box& that) requires requires(ptr p) {
    requires best::copyable<alloc>;
    p.copy(p);
  };
  constexpr box& operator=(const box& that)
    requires requires(ptr p) { p.copy_assign(p); };
  constexpr box(box&& that) requires best::moveable<alloc>;
  constexpr box& operator=(box&& that) requires best::moveable<alloc>;

  /// # `box::box(...)`
  ///
  /// Constructs a box by calling a constructor in-place.
  constexpr explicit box(auto&&... args)
    requires best::constructible<alloc> &&
             best::ptr_constructible<T, decltype(args)&&...> &&
             (!best::is_box<decltype(args)> && ...)
    : box(alloc{}, best::in_place, BEST_FWD(args)...) {}
  template <typename U>
  constexpr explicit box(std::initializer_list<U> il, auto&&... args)
    requires best::constructible<alloc> &&
             best::ptr_constructible<T, std::initializer_list<U>,
                                     decltype(args)&&...>
    : box(alloc{}, best::in_place, il, BEST_FWD(args)...) {}
  constexpr explicit box(best::in_place_t, auto&&... args)
    requires best::constructible<alloc> &&
             best::ptr_constructible<T, decltype(args)&&...>
    : box(alloc{}, best::in_place, BEST_FWD(args)...) {}
  constexpr explicit box(alloc alloc, best::in_place_t, auto&&... args)
    requires best::ptr_constructible<T, decltype(args)&&...>
    : alloc_(best::in_place, BEST_FWD(alloc)) {
    ptr null{nullptr, ptr::meta_for(BEST_FWD(args)...)};

    ptr_ = {
      allocator().alloc(null.layout()).cast(best::types<pointee>),
      null.meta(),
    };
    ptr_.construct(BEST_FWD(args)...);
  }

  template <best::ptr_losslessly_converts_to<T> U>
  constexpr box(box<U>&& that)
    : box(unsafe("this is ok, because we only allow lossless conversions"),
          BEST_MOVE(that.allocator()), BEST_MOVE(that).leak()) {}

  /// # `box::box(unsafe, ptr)`
  ///
  /// Wraps a pointer in a box. This pointer MUST have been allocated using
  /// the given allocated, with the layout of `T`.
  constexpr explicit box(unsafe u, best::ptr<T> ptr) : box(u, alloc{}, ptr) {}
  constexpr explicit box(unsafe u, alloc alloc, best::ptr<T> ptr)
    : ptr_(ptr), alloc_(best::in_place, BEST_FWD(alloc)) {}

  /// # `box::~box()`
  ///
  /// Boxes automatically destroy their contents.
  constexpr ~box() {
    if (ptr_.raw() == best::ptr<T>::dangling().raw()) { return; }

    ptr_.destroy();
    allocator().dealloc(ptr_.raw(), ptr_.layout());
  }

  /// # `box::as_ptr()`
  ///
  /// Returns the underlying `best::ptr`.
  constexpr best::ptr<T> as_ptr() const { return ptr_; }

  /// # `box::allocator()`
  ///
  /// Returns a reference to the box's allocator.
  constexpr const alloc& allocator() const { return *alloc_; }
  constexpr alloc& allocator() { return *alloc_; }

  /// # `box::raw()`, `box::meta()`
  ///
  /// Returns the raw underlying pointer and its metadata.
  constexpr pointee* raw() const { return as_ptr().raw(); }
  constexpr metadata meta() const { return as_ptr().meta(); }

  /// # `box::layout()`,
  ///
  /// Returns the layout of the pointed-to value.
  constexpr best::layout layout() const { return as_ptr().layout(); }

  /// # `box::try_copy()`
  ///
  /// Returns a copy of this box's contents, if it is copyable at runtime.
  ///
  /// Some types, such as `best::dyn`s, *can* be copied at runtime, but not
  /// necessarily. This function exposes this as a fallible operation.
  constexpr best::option<box> try_copy() const requires best::copyable<alloc>;

  /// # `box::operator*, box::operator->`
  ///
  /// Dereferences the box. Note that boxes cannot be null!
  ///
  /// Unlike `std::unique_ptr<T>`, dereferencing preserves the constness and
  /// value category of the box.
  constexpr decltype(auto) operator*() const& { return *ptr_.as_const(); }
  constexpr decltype(auto) operator*() & { return *ptr_; }
  constexpr decltype(auto) operator*() const&& {
    return BEST_MOVE(*ptr_.as_const());
  }
  constexpr decltype(auto) operator*() && { return BEST_MOVE(*ptr_); }
  constexpr auto operator->() const { return ptr_.as_const().operator->(); }
  constexpr auto operator->() { return ptr_.operator->(); }

  /// # `box[idx]`, `box(idx)`
  ///
  /// If the pointee of this `box` is indexable or callable, these functions
  /// will forward to it.
  // clang-format off
  constexpr decltype(auto) operator[](size_t i) const requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator[](size_t i) requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator[](bounds i) const requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator[](bounds i) requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator[](auto&& i) const requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator[](auto&& i) requires requires { as_ptr()[i]; } { return as_ptr()[i]; }
  constexpr decltype(auto) operator()(auto&&... args) const requires requires { as_ptr()(BEST_FWD(args)...); } {
    return as_ptr()(BEST_FWD(args)...);
  }
  constexpr decltype(auto) operator()(auto&&... args) requires requires { as_ptr()(BEST_FWD(args)...); } {
    return as_ptr()(BEST_FWD(args)...);
  }
  // clang-format on

  /// # `box::into_raw()`
  ///
  /// Explodes this box into its raw parts, and inhibits the destructor. The
  /// resulting pointer must be freed using the returned allocator at the end of
  /// the object's lifetime.
  constexpr best::row<best::ptr<T>, alloc> into_raw() && {
    auto ptr = std::exchange(ptr_, best::ptr<T>::dangling());
    return {ptr, *BEST_MOVE(alloc_)};
  }

  /// # `box::leak()`
  ///
  /// Disables this `box`'s destructor and returns the pointer. This function
  /// explicitly leaks memory by withholding the call to `dealloc()`.
  constexpr best::ptr<T> leak() && {
    return BEST_MOVE(*this).into_raw().first();
  }

  template <typename U>
  constexpr operator best::ptr<U>() const requires requires {
    { as_ptr().as_const() } -> best::converts_to<best::ptr<U>>;
  }
  {
    return as_ptr().as_const();
  }
  template <typename U>
  constexpr operator best::ptr<U>() requires requires {
    { as_ptr() } -> best::converts_to<best::ptr<U>>;
  }
  {
    return as_ptr();
  }

  friend void BestFmt(auto& fmt, const box& box) {
    if constexpr (requires { fmt.format(*box); }) {
      if (fmt.current_spec().method != 'p') { fmt.format(*box); }
      return;
    }
    fmt.format(box.as_ptr());
  }
  constexpr friend void BestFmtQuery(auto& query, box*) {
    query = query.template of<T>;
    query.uses_method = [](auto r) {
      if (r == 'p') { return true; }
      auto that = best::as_auto<decltype(query)>::template of<T>.uses_method;
      return that && that(r);
    };
  }

  template <typename U>
  constexpr bool operator==(const best::box<U>& that) const
    requires best::equatable<best::view<T>, best::view<U>>
  {
    return **this == *that;
  }
  template <best::equatable<best::view<T>> U>
  constexpr bool operator==(const U& u) const {
    return **this == u;
  }

  template <typename U>
  constexpr best::order_type<best::view<T>, best::view<U>> operator<=>(
    const best::box<U>& that) const
    requires best::comparable<best::view<T>, best::view<U>>
  {
    return **this <=> *that;
  }
  template <best::comparable<best::view<T>> U>
  constexpr best::order_type<best::view<T>, U> operator<=>(const U& u) const {
    return **this <=> u;
  }

  constexpr explicit box(niche) : ptr_(nullptr) {}
  constexpr bool operator==(niche) const { return ptr_ == nullptr; }

 private:
  best::ptr<T> ptr_;
  [[no_unique_address]] best::object<alloc> alloc_;
};

template <best::is_thin T>
box(T&&) -> box<best::as_auto<T>>;
template <best::is_object T>
box(std::initializer_list<T>) -> box<T[]>;

/// # `best::dynbox<I>`
///
/// A shorthand for a box containing a `best::dyn`.
template <best::interface I, typename A = best::malloc>
using dynbox = best::box<best::dyn<I>, A>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T, typename A>
constexpr box<T, A>::box(const box& that) requires requires(ptr p) {
  requires best::copyable<alloc>;
  p.copy(p);
}
  : alloc_(best::in_place, that.allocator()) {
  ptr_ = {
    allocator().alloc(that.as_ptr().layout()).cast(best::types<pointee>),
    that.as_ptr().meta(),
  };
  ptr_.copy(that.as_ptr());
}

template <typename T, typename A>
constexpr box<T, A>& box<T, A>::operator=(const box& that)
  requires requires(ptr p) { p.copy_assign(p); }
{
  if (layout() != that.layout()) {
    this->~box();
    return *new (this) box(that);
  } else {
    as_ptr().copy_assign(that.as_ptr());
    return *this;
  }
}
template <typename T, typename A>
constexpr box<T, A>::box(box&& that) requires best::moveable<alloc>
  : ptr_(std::exchange(that.ptr_, best::ptr<T>::dangling())),
    alloc_(BEST_MOVE(that.alloc_)) {}

template <typename T, typename A>
constexpr box<T, A>& box<T, A>::operator=(box&& that)
  requires best::moveable<alloc>
{
  if (best::equal(this, &that)) { return *this; }
  this->~box();
  return *new (this) box(BEST_MOVE(that));
}

template <typename T, typename A>
constexpr best::option<box<T, A>> box<T, A>::try_copy() const
  requires best::copyable<A>
{
  if (!as_ptr().can_copy()) { return best::none; }

  best::ptr<T> copy{
    allocator().alloc(as_ptr().layout()).cast(best::types<pointee>),
    {},
  };
  return box(best::unsafe("raw has been appropriately allocated"), allocator(),
             copy.try_copy(as_ptr()));
}

}  // namespace best

#endif  // BEST_CONTAINER_BOX_H_
