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
#include "best/base/tags.h"
#include "best/memory/allocator.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/memory/span.h"
#include "best/meta/init.h"

//! Value boxing.
//!
//! Boxes are pointers to heap-allocated objects, essentially fulfilling the
//! purpose of `std::unique_ptr` but with semantics closer to Rust's `Box<T>`
//! type. In particular, unlike `std::unique_ptr`, they are copyable.
//!
//! There are three flavors: `best::box<T>`, a single value; `best::box<T[]>`,
//! a heap-allocated array; `best::vbox<T>`, an upcast value.

namespace best {
/// # `best::box<T>`
///
/// A non-null pointer to a value on the heap.
template <typename T, typename A = best::malloc>
class BEST_RELOCATABLE box final {
 public:
  /// # `box::type`.
  ///
  /// The wrapped type; `box<T>` is nominally a `T*`.
  using type = T;

  /// # `box::pointee`.
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
  box(const box& that) requires best::copyable<T> && best::copyable<alloc>;
  box& operator=(const box& that) requires best::copyable<T>;
  box(box&& that) requires best::moveable<alloc>;
  box& operator=(box&& that) requires best::moveable<alloc>;

  /// # `box::box(value)`
  ///
  /// Wraps a value by copying/moving out from it.
  explicit box(const T& from) : box(best::in_place, from) {}
  explicit box(T&& from) : box(best::in_place, BEST_MOVE(from)) {}
  explicit box(alloc alloc, const T& from)
    : box(BEST_FWD(alloc), best::in_place, from) {}
  explicit box(alloc alloc, T&& from)
    : box(BEST_FWD(alloc), best::in_place, BEST_MOVE(from)) {}

  /// # `box::box(...)`
  ///
  /// Constructs a box by calling a constructor in-place.
  explicit box(best::in_place_t, auto&&... args)
    : box(alloc{}, best::in_place, BEST_FWD(args)...) {}
  explicit box(alloc alloc, best::in_place_t, auto&&... args)
    : alloc_(best::in_place, BEST_FWD(alloc)) {
    ptr_ =
      best::ptr(allocator().alloc(best::layout::of<T>())).cast(best::types<T>);
    ptr_.construct(BEST_FWD(args)...);
  }

  /// # `box::box(unsafe, ptr)`
  ///
  /// Wraps a pointer in a box. This pointer MUST have been allocated using
  /// the given allocated, with the layout of `T`.
  explicit box(unsafe u, best::ptr<T> ptr) : box(u, alloc{}, ptr) {}
  explicit box(unsafe u, alloc alloc, best::ptr<T> ptr)
    : ptr_(ptr), alloc_(best::in_place, BEST_FWD(alloc)) {}

  /// # `box::~box()`
  ///
  /// Boxes automatically destroy their contents.
  ~box() {
    if (ptr_ == best::ptr<T>::dangling()) { return; }

    ptr_.destroy();
    allocator().dealloc(ptr_.raw(), best::layout::of<T>());
  }

  /// # `box::as_ptr()`
  ///
  /// Returns the underlying `best::ptr`.
  best::ptr<T> as_ptr() const { return ptr_; }

  /// # `box::allocator()`
  ///
  /// Returns a reference to the box's allocator.
  const alloc& allocator() const { return *alloc_; }
  alloc& allocator() { return *alloc_; }

  /// # `box::into_raw()`
  ///
  /// Explodes this box into its raw parts, and inhibits the destructor. The
  /// resulting pointer must be freed using the returned allocator at the end of
  /// the object's lifetime.
  best::row<best::ptr<T>, alloc> into_raw() && {
    auto ptr = std::exchange(ptr_, best::ptr<T>::dangling());
    return {ptr, *BEST_MOVE(alloc_)};
  }

  /// # `box::leak()`
  ///
  /// Disables this `box`'s destructor and returns the pointer. This function
  /// explicitly leaks memory by withholding the call to `dealloc()`.
  best::ptr<T> leak() && { return BEST_MOVE(*this).into_raw().first(); }

  /// # `box::operator*, box::operator->`
  ///
  /// Dereferences the box. Note that boxes cannot be null!
  ///
  /// Unlike `std::unique_ptr<T>`, dereferencing preserves the constness and
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
  constexpr friend void BestFmtQuery(auto& query, box*) {
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

  explicit box(niche) : ptr_(nullptr) {}
  bool operator==(niche) const { return ptr_ == nullptr; }

 private:
  template <typename, typename>
  friend class vbox;

  best::ptr<T> ptr_;
  [[no_unique_address]] best::object<alloc> alloc_;
};

template <typename T>
box(T&&) -> box<best::as_auto<T>>;

/// # `best::box<T[]>`
///
/// A non-null pointer to an array on the heap.
template <typename T, typename A>
class BEST_RELOCATABLE box<T[], A> final {
 public:
  /// # `box::type`.
  ///
  /// The wrapped type; `ptr<T>` is nominally a `T*`.
  using type = T;

  /// # `box::pointee`.
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

  /// # `box::box()`
  ///
  /// Constructs an empty array box.
  box() : box(best::span<T>{}) {}

  /// # `box::box(box)`
  ///
  /// Trivially relocatable. Copies perform memory allocations.
  box(const box& that) requires best::copyable<T> && best::copyable<alloc>;
  box& operator=(const box& that) requires best::copyable<T>;
  box(box&& that) requires best::moveable<alloc>;
  box& operator=(box&& that) requires best::moveable<alloc>;

  /// # `box::box(...)`
  ///
  /// Constructs a box by copying out of a span.
  template <typename U = const T>
  explicit box(best::span<U> args) : box(alloc{}, args) {}
  template <typename U = const T>
  explicit box(alloc alloc, best::span<U> args)
    : size_(args.size()), alloc_(best::in_place, BEST_FWD(alloc)) {
    if (size_ == 0) { return; }
    ptr_ = best::ptr(allocator().alloc(best::layout::array<T>(args.size())))
             .cast(best::types<T>);
    as_span().emplace_from(args);
  }

  /// # `box::box(unsafe, ...)`
  ///
  /// Constructs a box by taking ownership of a span.
  explicit box(unsafe, best::span<T> args) : box(alloc{}, args) {}
  explicit box(unsafe, alloc alloc, best::span<T> args)
    : ptr_(args.data()),
      size_(args.size()),
      alloc_(best::in_place, BEST_FWD(alloc)) {}

  /// # `box::~box()`
  ///
  /// Boxes automatically destroy their contents.
  ~box() {
    if (data() == best::ptr<T>::dangling()) { return; }

    as_span().destroy();
    alloc_->dealloc(data(), best::layout::array<T>(size()));
  }

  /// # `box::as_ptr()`
  ///
  /// Returns the underlying `best::ptr`.
  best::ptr<T> as_ptr() const { return ptr_; }

  /// # `box::allocator()`
  ///
  /// Returns a reference to the box's allocator.
  const alloc& allocator() const { return *alloc_; }
  alloc& allocator() { return *alloc_; }

  /// # `vec::data()`.
  ///
  /// Returns a pointer to the start of the array this box manages.
  best::ptr<const T> data() const { return ptr_; }
  best::ptr<T> data() { return ptr_; }

  /// # `vec::size()`.
  ///
  /// Returns the number of elements in this vector.
  size_t size() const { return size_; }

  /// # `vec::is_empty()`.
  ///
  /// Returns whether this is an empty vector.
  bool is_empty() const { return size() == 0; }

  /// # `box::as_span()`, `box::operator->()`
  ///
  /// Returns a span over the array this box manages.
  ///
  /// All of the span methods, including those not explicitly delegated, are
  /// accessible through `->`. For example, `my_box->sort()` works.
  best::span<const T> as_span() const { return {ptr_, size_}; }
  best::span<T> as_span() { return {ptr_, size_}; }
  best::arrow<best::span<const T>> operator->() const { return as_span(); }
  best::arrow<best::span<T>> operator->() { return as_span(); }

  /// # `box::into_raw()`
  ///
  /// Explodes this box into its raw parts, and inhibits the destructor. The
  /// resulting pointer must be freed using the returned allocator at the end of
  /// the object's lifetime.
  best::row<best::span<T>, alloc> into_raw() && {
    auto ptr = std::exchange(ptr_, best::ptr<T>::dangling());
    return {{ptr, size_}, *BEST_MOVE(alloc_)};
  }

  /// # `box::leak()`
  ///
  /// Disables this `box`'s destructor and returns the span. This function
  /// explicitly leaks memory by withholding the call to `dealloc()`.
  best::span<T> leak() && { return BEST_MOVE(*this).into_raw().first(); }

  /// # `box[idx]`, `box[{.start = ...}]`
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

  /// # `box::at(idx)`, `box::at({.start = ...})`
  ///
  /// Extracts a single element or a subspan. If the requested index is
  /// out-of-bounds, returns `best::none`.
  best::option<const T&> at(size_t idx) const { return as_span().at(idx); }
  best::option<T&> at(size_t idx) { return as_span().at(idx); }
  best::option<const T&> at(best::bounds b) const { return as_span().at(b); }
  best::option<T&> at(best::bounds b) { return as_span().at(b); }

  /// # `box::citer`, `box::iter`, `box::begin()`, `box::end()`.
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

  explicit box(niche) : ptr_(nullptr) {}
  bool operator==(niche) const { return ptr_ == nullptr; }

 private:
  best::ptr<T> ptr_ = best::ptr<T>::dangling();
  size_t size_;
  [[no_unique_address]] best::object<alloc> alloc_;
};

/// # `best::vbox<T>`
///
/// A non-null pointer to a type-erased value on the heap.
template <typename T, typename A = best::malloc>
class BEST_RELOCATABLE vbox final {
 public:
  /// # `vbox::type`.
  ///
  /// The wrapped type; `vbox<T>` is nominally a `T*`.
  using type = T;

  /// # `vbox::pointee`.
  ///
  /// The "true" pointee type. Internally, an `vbox` stores a `pointee*`.
  using pointee = best::ptr<T>::pointee;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  using alloc = A;

  vbox() = delete;

  /// # `vbox::vbox(box)`
  ///
  /// Trivially relocatable. Copies perform memory allocations; copy will crash
  /// if `T` is copyable but the complete type is not.
  vbox(const vbox& that) = delete;
  vbox& operator=(const vbox& that) = delete;
  vbox(vbox&& that) requires best::moveable<alloc>;
  vbox& operator=(vbox&& that) requires best::moveable<alloc>;

  /// # `vbox::vbox(box)`, `vbox::vbox(vbox)`
  ///
  /// Wraps a box, virtual or otherwise.
  template <best::ptr_convertible_to<T> U>
  explicit vbox(box<U> that)
    : ptr_(BEST_MOVE(that).leak()), alloc_(that.alloc_) {}
  template <best::ptr_convertible_to<T> U>
  explicit vbox(vbox<U> that)
    : ptr_(BEST_MOVE(that).leak()), alloc_(that.alloc_) {}

  /// # `vbox::vbox(unsafe, ptr)`
  ///
  /// Wraps a pointer in a box. This pointer MUST have been allocated using
  /// the given allocator.
  explicit vbox(unsafe u, best::vptr<T> ptr) : vbox(u, alloc{}, ptr) {}
  explicit vbox(unsafe u, alloc alloc, best::vptr<T> ptr)
    : ptr_(ptr), alloc_(best::in_place, BEST_FWD(alloc)) {}

  /// # `vbox::~vbox()`
  ///
  /// Boxes automatically destroy their contents.
  ~vbox() {
    if (ptr_.thin() == best::ptr<T>::dangling()) { return; }

    ptr_.destroy();
    allocator().dealloc(ptr_.raw(), ptr_.layout());
  }

  /// # `vbox::as_ptr()`
  ///
  /// Returns the underlying `best::vptr`.
  best::vptr<T> as_ptr() const { return ptr_; }

  /// # `vbox::allocator()`
  ///
  /// Returns a reference to the box's allocator.
  const alloc& allocator() const { return *alloc_; }
  alloc& allocator() { return *alloc_; }

  /// # `vbox::into_raw()`
  ///
  /// Explodes this box into its raw parts, and inhibits the destructor. The
  /// resulting pointer must be freed using the returned allocator at the end of
  /// the object's lifetime.
  best::row<best::vptr<T>, alloc> into_raw() && {
    auto ptr = std::exchange(ptr_, best::ptr<T>::dangling());
    return {ptr, *BEST_MOVE(alloc_)};
  }

  /// # `box::leak()`
  ///
  /// Disables this `box`'s destructor and returns the pointer. This function
  /// explicitly leaks memory by withholding the call to `dealloc()`.
  best::vptr<T> leak() && { return BEST_MOVE(*this).into_raw().first(); }

  /// # `vbox::operator*, vbox::operator->`
  ///
  /// Dereferences the box. Note that boxes cannot be null!
  ///
  /// Unlike `std::unique_ptr<T>`, dereferencing preserves the constness and
  /// value category of the box.
  cref operator*() const& { return *ptr_; }
  ref operator*() & { return *ptr_; }
  crref operator*() const&& { return BEST_MOVE(*ptr_); }
  rref operator*() && { return BEST_MOVE(*ptr_); }
  cptr operator->() const { return ptr_.operator->(); }
  ptr operator->() { return ptr_.operator->(); }

  /// # `vbox::copy()`
  ///
  /// Makes a copy of the contents of this `box`, if the complete type is
  /// copyable.
  best::option<vbox> copy() const requires best::copyable<alloc>
  {
    if (!ptr_.is_copyable()) { return best::none; }

    best::vptr<T> ptr(
      unsafe("the vtable is correct because we're making a copy"),
      best::ptr(allocator().alloc(ptr_.layout())).cast(best::types<T>),
      ptr_.vtable());
    ptr_.copy_to(ptr.raw());

    return vbox(alloc_, ptr);
  }

  explicit vbox(niche) : ptr_(nullptr) {}
  bool operator==(niche) const { return ptr_.raw() == nullptr; }

 private:
  best::vptr<T> ptr_;
  [[no_unique_address]] best::object<alloc> alloc_;
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T, typename A>
box<T, A>::box(const box& that)
  requires best::copyable<T> && best::copyable<alloc>
  : box(that.allocator(), best::in_place, *that) {}

template <typename T, typename A>
box<T, A>& box<T, A>::operator=(const box& that) requires best::copyable<T>
{
  **this = *that;
  return *this;
}
template <typename T, typename A>
box<T, A>::box(box&& that) requires best::moveable<alloc>
  : ptr_(std::exchange(that.ptr_, best::ptr<T>::dangling())),
    alloc_(BEST_MOVE(that.alloc_)) {}

template <typename T, typename A>
box<T, A>& box<T, A>::operator=(box&& that) requires best::moveable<alloc>
{
  if (best::equal(this, &that)) { return *this; }
  this->~box();
  new (this) box(BEST_MOVE(that));
  return *this;
}

template <typename T, typename A>
box<T[], A>::box(const box& that)
  requires best::copyable<T> && best::copyable<alloc>
  : box(that.allocator(), that.as_span()) {}
template <typename T, typename A>
box<T[], A>& box<T[], A>::operator=(const box& that) requires best::copyable<T>
{
  if (size() == that.size()) {
    as_span().copy_from(that.as_span());
  } else {
    // TODO: It would be nice to try to re-use this allocation when possible.
    if (data() != best::ptr<T>::dangling()) {
      as_span().destroy();
      alloc_->dealloc(data(), best::layout::array<T>(size()));
    }
    ptr_ = best::ptr(alloc_->alloc(best::layout::array<T>(that.size())))
             .cast(best::types<T>);
    size_ = that.size();
    as_span().emplace_from(that.as_span());
  }
  return *this;
}
template <typename T, typename A>
box<T[], A>::box(box&& that) requires best::moveable<alloc>
  : ptr_(std::exchange(that.ptr_, best::ptr<T>::dangling())),
    size_(that.size()),
    alloc_(BEST_MOVE(that.alloc_)) {}
template <typename T, typename A>
box<T[], A>& box<T[], A>::operator=(box&& that) requires best::moveable<alloc>
{
  if (best::equal(this, &that)) { return *this; }
  this->~box();
  new (this) box(BEST_MOVE(that));
  return *this;
}

template <typename T, typename A>
vbox<T, A>::vbox(vbox&& that) requires best::moveable<alloc>
  : ptr_(std::exchange(that.ptr_, best::ptr<T>::dangling())),
    alloc_(BEST_MOVE(that.alloc_)) {}
template <typename T, typename A>
vbox<T, A>& vbox<T, A>::operator=(vbox&& that) requires best::moveable<alloc>
{
  if (best::equal(this, &that)) { return *this; }
  this->~vbox();
  new (this) vbox(BEST_MOVE(that));
  return *this;
}
}  // namespace best

#endif  // BEST_CONTAINER_BOX_H_
