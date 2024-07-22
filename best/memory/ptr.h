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

#ifndef BEST_MEMORY_PTR_H_
#define BEST_MEMORY_PTR_H_

#include <cstddef>
#include <type_traits>

#include "best/base/hint.h"
#include "best/base/niche.h"
#include "best/base/ord.h"
#include "best/base/port.h"
#include "best/log/internal/crash.h"
#include "best/memory/internal/ptr.h"
#include "best/memory/layout.h"
#include "best/meta/init.h"
#include "best/meta/taxonomy.h"
#include "best/meta/tlist.h"
#include "best/meta/traits.h"

//! Pointers.
//!
//! This header provides an enhanced raw pointer type `best::ptr<T>`.

namespace best {
/// # `best::pointee_for<T>`
///
/// A suitable pointee type for representing values of the given type. In
/// particular, this converts references into pointers, and function types
/// into function pointers.
template <typename T>
using pointee_for = best::select<best::is_object<T> || best::is_void<T>, T,
                                 const best::as_ptr<T>>;

/// # `best::ptr_convertible_to`
///
/// Whether a `best::ptr<U>` is implicitly convertible into a `best::ptr<T>`.
template <typename T, typename U>
concept ptr_convertible_to =
  best::convertible<best::pointee_for<U>*, best::pointee_for<T>*>;

/// # `best::ptr<T>`
///
/// A pointer to a possibly non-object `T`.
///
/// This allows creating a handle to a possibly non-object type that can be
/// manipulated in a consistent manner.
///
/// The mapping is as follows:
///
/// ```text
/// best::is_object<T> -> T*
/// best::is_ref<T>    -> const best::as_ptr<T>*
/// best::is_func<T>   -> const best::as_ptr<T>*
/// best::is_void<T>   -> void*
/// ```
///
/// Note the `const` for reference types: this reflects the fact that e.g.
/// `T& const` is just `T&`.
///
/// Note that a `best::ptr<void>` is *not* a `void*` insofar that it does not
/// represent a type-erased pointer.
template <typename T>
class ptr final {
 public:
  /// # `ptr::type`.
  ///
  /// The wrapped type; `ptr<T>` is nominally a `T*`.
  using type = T;

  /// # `ptr::pointee`.
  ///
  /// The "true" pointee type. Internally, an `ptr` stores a `pointee*`.
  using pointee = best::pointee_for<T>;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;

  /// # `ptr::ptr()`
  ///
  /// Constructs a null pointer.
  constexpr ptr() = default;
  constexpr ptr(std::nullptr_t) {}

  /// # `ptr::ptr(ptr)`
  ///
  /// Trivially copyable.
  constexpr ptr(const ptr&) = default;
  constexpr ptr& operator=(const ptr&) = default;
  constexpr ptr(ptr&&) = default;
  constexpr ptr& operator=(ptr&&) = default;

  /// # `ptr::ptr(ptr)`
  ///
  /// Wraps a C++ pointer, potentially casting it if necessary.
  template <ptr_convertible_to<T> U>
  constexpr ptr(U* ptr) : BEST_PTR_(ptr) {}

  /// # `ptr::ptr(ptr<U>)`
  ///
  /// Wraps an `ptr` of a different type, performing implicit conversion if
  /// necessary.
  template <ptr_convertible_to<T> U>
  constexpr ptr(ptr<U> ptr)
    : BEST_PTR_(
        // NOTE: This const cast won't cast away `const` if this constructor
        // is called via implicit conversion, unless T is a void type.
        const_cast<best::unqual<typename best::ptr<U>::pointee>*>(ptr.raw())) {}

  /// # `ptr::is_niche()`
  ///
  /// Whether this value is a niche representation.
  constexpr bool is_niche() const;

  /// # `ptr::dangling()`
  ///
  /// Returns a non-null but dangling pointer, which is unique for the choice of
  /// `T`.
  static ptr dangling() { return from_addr(best::align_of<T>); }

  /// # `ptr::to_object()`
  ///
  /// Converts this `best::ptr` to a `best::ptr` pointing to an object type (or
  /// void).
  constexpr best::ptr<pointee> to_object() const { return *this; }

  /// # `ptr::cast()`
  ///
  /// Performs an arbitrary pointer cast.
  template <typename U>
  constexpr best::ptr<U> cast(best::tlist<U> = {}) const {
    return (pointee_for<U>*)raw();
  }

  /// # `ptr::to_addr()`, `ptr::from_addr()`
  ///
  /// Converts this pointer to/from a raw address.
  uintptr_t to_addr() const { return reinterpret_cast<uintptr_t>(raw()); }
  static ptr from_addr(uintptr_t addr) {
    return reinterpret_cast<pointee*>(addr);
  }

  /// # `ptr::raw()`.
  ///
  /// Returns the raw underlying pointer.
  constexpr pointee* raw() const { return BEST_PTR_; }
  constexpr operator pointee*() const { return raw(); }

  /// # `ptr::operator==`, `ptr::operator<=>`
  ///
  /// All pointers may be compared by address, regardless of type.
  template <typename U>
  constexpr bool operator==(ptr<U> that) const;
  template <typename U>
  constexpr best::ord operator<=>(ptr<U> that) const;
  template <typename U>
  constexpr bool operator==(U* that) const;
  template <typename U>
  constexpr best::ord operator<=>(U* that) const;
  constexpr bool operator==(std::nullptr_t) const { return raw() == nullptr; }
  constexpr best::ord operator<=>(std::nullptr_t) const {
    return raw() <=> nullptr;
  }

  /// # `ptr::operator*`, `ptr::operator->`
  ///
  /// Dereferences this pointer.
  ///
  /// For example, this will dereference the wrapping pointer of a `T&`, so
  /// if `T = U&`, then `raw = U**` and this dereferences that twice.
  BEST_INLINE_SYNTHETIC constexpr ref operator*() const;
  BEST_INLINE_SYNTHETIC constexpr best::as_ptr<ref> operator->() const;

  /// # `ptr[idx]`, `ptr::operator+`, `ptr::operator-`
  ///
  /// Performs unchecked pointer arithmetic in the way you'd expect.
  constexpr ref operator[](ptrdiff_t idx) const { return *(*this + idx); }
  constexpr ptr operator+(ptrdiff_t idx) const;
  constexpr ptr operator+=(ptrdiff_t idx);
  constexpr ptr operator++();
  constexpr ptr operator++(int);
  constexpr ptr operator-(ptrdiff_t idx) const;
  constexpr ptr operator-=(ptrdiff_t idx);
  constexpr ptr operator--();
  constexpr ptr operator--(int);
  template <typename U>
  constexpr ptrdiff_t operator-(ptr<U> that) const
    requires best::same<const volatile T, const volatile U>
  {
    return raw() - that.raw();
  }

  /// # `ptr::byte_offset()`
  ///
  /// Performs explicitly scaled pointer arithmetic: this function behaves as
  /// if we cast to a `size`-sized type, offset by `idx`, and cast back to `T`.
  ///
  /// Beware: this may create unaligned pointers, which is Undefined Behavior.
  ptr scaled_offset(ptrdiff_t idx, size_t size = 1) const {
    return (cast(best::types<const char>) + (idx * size)).cast(best::types<T>);
  }

  /// # `ptr::construct()`
  ///
  /// Constructs in place according to the constructors valid per
  /// `best::constructible`.
  ///
  /// Note that this function implicitly casts away an untracked const if called
  /// on an `ptr<T&>` or `ptr<T()>`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct(Args&&... args) const
    requires best::constructible<T, Args&&...>;
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct(best::args<Args...> args) const
    requires best::constructible<T, Args...>;
  BEST_INLINE_SYNTHETIC constexpr void construct(niche) const
    requires best::has_niche<T>;

  /// # `ptr::assign()`
  ///
  /// Assigns in place according to the assignments valid per
  /// `best::assignable`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(Args&&... args) const
    requires best::assignable<T, Args&&...>;
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(best::args<Args...> args) const
    requires best::assignable<T, Args...>;

  /// # `ptr::destroy()`
  ///
  /// Destroys the pointed to object in place.
  BEST_INLINE_SYNTHETIC constexpr void destroy() const
    requires best::destructible<T>;

  /// # `ptr::fill()`
  ///
  /// Fills `count` `T`s with the given byte (i.e., `memset()`).
  BEST_INLINE_ALWAYS void fill(uint8_t byte, size_t count = 1) const {
    best::ptr_internal::memset(raw(), byte, count * sizeof(pointee));
  }

  /// # `ptr::copy()`, `ptr::move()`, ptr::relo()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions must not overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  ///
  /// `ptr::relo()` will leave the copied-from region uninitialized.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy(ptr<U> that, size_t count = 1) const
    requires best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move(ptr<U> that, size_t count = 1) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void relo(ptr<U> that, size_t count = 1) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Relo}>(that, count);
  }

  /// # `ptr::copy_assign()`, `ptr::move_assign()`, ptr::relo_assign()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may not overlap, and
  /// the destination must be initialized.
  ///
  /// `ptr::relo_assign()` will leave the copied-from region uninitialized.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_assign(ptr<U> that,
                                                size_t count = 1) const
    requires best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .assign = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_assign(ptr<U> that,
                                                size_t count = 1) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .assign = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void relo_assign(ptr<U> that,
                                                size_t count = 1) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Relo, .assign = true}>(that, count);
  }

  /// # `ptr::copy_overlapping()`, `ptr::move_overlapping()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_overlapping(ptr<U> that,
                                                     size_t count) const
    requires best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .overlapping = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_overlapping(ptr<U> that,
                                                     size_t count) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .overlapping = true}>(that, count);
  }
  BEST_INLINE_ALWAYS constexpr void relo_overlapping(ptr<T> that,
                                                     size_t count) const;

  /// # `ptr::copy_assign_overlapping()`, `ptr::move_assign_overlapping()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_assign_overlapping(ptr<U> that,
                                                            size_t count) const
    requires best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .overlapping = true, .assign = true}>(
      that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_assign_overlapping(ptr<U> that,
                                                            size_t count) const
    requires best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .overlapping = true, .assign = true}>(
      that, count);
  }

  friend void BestFmt(auto& fmt, ptr value) {
    fmt.format("{:#x}", value.to_addr());
  }
  constexpr friend void BestFmtQuery(auto& query, ptr*) {
    query.requires_debug = false;
    query.uses_method = [](auto r) { return r == 'p'; };
  }

 private:
  template <typename>
  friend class ptr;
  template <typename>
  friend class vptr;

  BEST_INLINE_ALWAYS constexpr void check() const {
    if (!best::is_debug() || std::is_constant_evaluated()) { return; }
    if (*this == nullptr) {
      best::crash_internal::crash("dereferenced a null `best::ptr`");
    }
    if (*this < from_addr(0x1000)) {
      best::crash_internal::crash("dereferenced a dangling `best::ptr`");
    };
  }

  struct how final {
    enum { Copy, Move, Relo } kind;
    bool overlapping = false;
    bool assign = false;

    template <typename U>
    constexpr bool can_memcpy() const {
      return best::same<const pointee_for<T>, const pointee_for<U>> &&
             (best::copyable<T, trivially> ||
              (best::relocatable<T, trivially> && kind == how::Relo));
    }

    bool should_memcpy() const {
      return !std::is_constant_evaluated() ||
             // Constexpr support for memcpy() is only valid for trivially
             // copyable types, *not* trivially relocatable types.
             (BEST_CONSTEXPR_MEMCPY_ && best::copyable<T, trivially>);
    }
  };
  template <how how, typename U>
  BEST_INLINE_SYNTHETIC constexpr void copy_impl(ptr<U> that,
                                                 size_t count) const;

  static constexpr uint8_t Magic = 0xcd;

 public:
  // Public for structural-ness.
  pointee* BEST_PTR_ = nullptr;
};
template <typename T>
ptr(ptr<T>) -> ptr<T>;
template <typename T>
ptr(T*) -> ptr<T>;

/// # `best::vtable`
///
/// A vtable for a `best::vptr`. It does nothing on its own, needs to be
/// combined with a `best::vptr` first.
class vtable final {
 public:
  /// # `vtable::of()`
  ///
  /// A constant referring to the vtable for a given type.
  template <typename T>
  static constexpr const vtable* of() {
    return &vt<best::unqual<T>>;
  }

  /// # `vtable::layout()`
  ///
  /// Returns the layout for this vtable's type.
  constexpr best::layout layout() const { return layout_; }

  /// # `vtable::operator==`
  ///
  /// Two vtables compare as equal if they are either the default value, or
  /// they were constructed by the same specialization of `vtable::of()`.
  constexpr bool operator==(const vtable& that) const {
    return id_ == that.id_;
  }

 private:
  template <typename>
  friend class vptr;

  template <typename T>
  static const vtable vt;

  best::layout layout_;
  void (*dtor_)(void*) = nullptr;
  void (*copy_)(void*, const void*) = nullptr;
  const void* id_ = nullptr;
};

template <typename T>
inline constexpr const vtable vtable::vt = [] {
  vtable vt;
  vt.layout_ = layout::of<T>();
  vt.id_ = of<T>();

  if constexpr (!std::is_class_v<T> || !std::is_abstract_v<T>) {
    vt.dtor_ = +[](void* p) { best::ptr(p).template cast<T>().destroy(); };
  }

  if constexpr (best::copyable<T>) {
    vt.copy_ = +[](void* to, const void* from) {
      auto dst = best::ptr(to).template cast<T>();
      auto src = best::ptr(from).template cast<const T>();
      dst.copy(src, 1);
    };
  }

  return vt;
}();

/// # `best::vptr`
///
/// A polymorphic fat pointer.
///
/// This type is *similar* to a virtual base pointer in ordinary C++, but
/// carries some additional vtable information on the side.
///
/// `best::vptr<void>` represents a fully type-erased pointer.
template <typename T>
class vptr final {
 public:
  /// # `ptr::type`.
  ///
  /// The wrapped type; `vptr<T>` is nominally a `T*`.
  using type = T;

  /// # `ptr::pointee`.
  ///
  /// The "true" pointee type. Internally, an `vptr` stores a `pointee*`.
  using pointee = best::pointee_for<T>;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;

  /// # `vptr::vptr()`
  ///
  /// Constructs a null pointer. Such a pointer will not contain a vtable.
  constexpr vptr() = default;
  constexpr vptr(std::nullptr_t){};

  /// # `ptr::ptr(ptr)`
  ///
  /// Trivially copyable.
  constexpr vptr(const vptr&) = default;
  constexpr vptr& operator=(const vptr&) = default;
  constexpr vptr(vptr&&) = default;
  constexpr vptr& operator=(vptr&&) = default;

  /// # `ptr::ptr(ptr)`
  ///
  /// Wraps a thin pointer, potentially upcasting it if necessary.
  template <ptr_convertible_to<T> U>
  constexpr vptr(U* ptr) : vptr(ptr), vt_(vtable::of<U>()) {}
  template <ptr_convertible_to<T> U>
  constexpr vptr(ptr<U> ptr) : ptr_(ptr), vt_(vtable::of<U>()) {}

  /// # `vptr::vptr(vptr<U>)`
  ///
  /// Upcasts a virtual pointer.
  template <ptr_convertible_to<T> U>
  constexpr vptr(vptr<U> ptr) : ptr_(ptr.ptr_), vt_(ptr.vt_) {}

  /// # `vptr::vptr(ptr, vt)`
  ///
  /// Constructs a virtual pointer from a pointer and vtable. The caller is
  /// responsible for ensuring this is an appropriate vtable for that pointer.
  template <ptr_convertible_to<T> U>
  constexpr vptr(unsafe, U* ptr, const best::vtable* vt) : ptr_(ptr), vt_(vt) {}
  template <ptr_convertible_to<T> U>
  constexpr vptr(unsafe, ptr<U> ptr, const best::vtable* vt)
    : ptr_(ptr), vt_(vt) {}

  /// # `vptr::raw()`, `vptr::thin()`
  ///
  /// Returns the raw underlying pointer.
  constexpr T* raw() const { return ptr_.raw(); }
  constexpr best::ptr<T> thin() const { return ptr_; }

  /// # `vptr::vtable()`
  ///
  /// Returns a pointer to this type's vtable.
  constexpr const vtable* vtable() const { return vt_; }

  /// # `vptr::layout()`
  ///
  /// Returns the layout for this `vptr`'s type.
  constexpr best::layout layout() const { return vtable()->layout(); }

  /// # `vptr::is()`
  ///
  /// Returns whether the complete type is a specific type. This does not use
  /// RTTI: it may return `false` when a `dynamic_cast` would succeed.
  template <typename U>
  constexpr bool is() const {
    return vtable() && *vtable() == *vtable::of<U>();
  }

  /// # `vptr::operator*`, `vptr::operator->`
  ///
  /// Dereferences this pointer.
  ///
  /// For example, this will dereference the wrapping pointer of a `T&`, so
  /// if `T = U&`, then `raw = U**` and this dereferences that twice.
  BEST_INLINE_SYNTHETIC constexpr ref operator*() const { return *ptr_; }
  BEST_INLINE_SYNTHETIC constexpr best::as_ptr<ref> operator->() const {
    return ptr_.operator->();
  }

  /// # `vptr[idx]`, `vptr::operator+`, `vptr::operator-`
  ///
  /// Performs unchecked pointer arithmetic in the way you'd expect.
  ///
  /// These functions assume that this is a pointer to a homogenous array of the
  /// same type, as represented by `vtable()`. Thus, the stride is that of the
  /// complete type, which prevents these functions from being constexpr.
  ref operator[](ptrdiff_t idx) const { return *(*this + idx); }
  vptr operator+(ptrdiff_t idx) const;
  vptr operator+=(ptrdiff_t idx);
  vptr operator++();
  vptr operator++(int);
  vptr operator-(ptrdiff_t idx) const;
  vptr operator-=(ptrdiff_t idx);
  vptr operator--();
  vptr operator--(int);

  /// # `ptr::destroy()`
  ///
  /// Destroys the pointed to object in place. This will call the complete
  /// object destructor.
  BEST_INLINE_SYNTHETIC void destroy() const requires best::destructible<T>;

  /// # `ptr::is_copyable()`
  ///
  /// Whether the complete type this pointer points to is copyable.
  constexpr bool is_copyable() const { return vtable()->copy_ != nullptr; }

  /// # `ptr::destroy()`
  ///
  /// Copies the pointed to object to the given destination. This will call the
  /// complete object destructor.
  ///
  /// If the complete type is not copyable, this function crashes.
  BEST_INLINE_SYNTHETIC void copy_to(void* to) const;

  friend void BestFmt(auto& fmt, vptr value) {
    fmt.format("{:#x}/{:#x}", value.to_addr(), value.vtable());
  }
  constexpr friend void BestFmtQuery(auto& query, vptr*) {
    query.requires_debug = false;
    query.uses_method = [](auto r) { return r == 'p'; };
  }

 private:
  template <typename>
  friend class ptr;
  template <typename>
  friend class vptr;

  best::ptr<T> ptr_;
  const best::vtable* vt_;
};

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T>
constexpr bool ptr<T>::is_niche() const {
  if constexpr (best::has_niche<T>) {
    if constexpr (best::is_ref<T>) {
      return *raw() == nullptr;
    } else {
      return **this == niche{};
    }
  }
  return false;
}

template <typename T>
template <typename U>
constexpr bool ptr<T>::operator==(ptr<U> that) const {
  return best::equal(raw(), that.raw());
};
template <typename T>
template <typename U>
constexpr best::ord ptr<T>::operator<=>(ptr<U> that) const {
  return best::compare(raw(), that.raw());
};
template <typename T>
template <typename U>
constexpr bool ptr<T>::operator==(U* that) const {
  return best::equal(raw(), that);
};
template <typename T>
template <typename U>
constexpr best::ord ptr<T>::operator<=>(U* that) const {
  return best::compare(raw(), that);
};

template <typename T>
BEST_INLINE_SYNTHETIC constexpr ptr<T>::ref ptr<T>::operator*() const {
  check();
  if constexpr (best::is_object<T>) {
    return *raw();
  } else if constexpr (best::is_void<T>) {
    return;
  } else {
    return **raw();
  }
}
template <typename T>
BEST_INLINE_SYNTHETIC constexpr auto ptr<T>::operator->() const
  -> best::as_ptr<ref> {
  check();
  if constexpr (best::is_object<T> || best::is_void<T>) {
    return raw();
  } else {
    return *raw();
  }
}

template <typename T>
constexpr ptr<T> ptr<T>::operator+(ptrdiff_t idx) const {
  if (idx == 0) { return *this; }
  check();
  if constexpr (best::is_void<T>) {
    return *this;
  } else {
    return ptr(raw() + idx);
  }
}
template <typename T>
constexpr ptr<T> ptr<T>::operator+=(ptrdiff_t idx) {
  *this = *this + idx;
  return *this;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator++() {
  return *this += 1;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator++(int) {
  auto prev = *this;
  ++*this;
  return *this;
}

template <typename T>
constexpr ptr<T> ptr<T>::operator-(ptrdiff_t idx) const {
  return *this + -idx;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator-=(ptrdiff_t idx) {
  *this = *this - idx;
  return *this;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator--() {
  return *this -= 1;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator--(int) {
  auto prev = *this;
  --*this;
  return *this;
}

template <typename T>
vptr<T> vptr<T>::operator+(ptrdiff_t idx) const {
  return {
    thin().scaled_offset(idx, layout().size()),
    vtable(),
  };
}
template <typename T>
vptr<T> vptr<T>::operator+=(ptrdiff_t idx) {
  *this = *this + idx;
  return *this;
}
template <typename T>
vptr<T> vptr<T>::operator++() {
  return *this += 1;
}
template <typename T>
vptr<T> vptr<T>::operator++(int) {
  auto prev = *this;
  ++*this;
  return *this;
}

template <typename T>
vptr<T> vptr<T>::operator-(ptrdiff_t idx) const {
  return *this + -idx;
}
template <typename T>
vptr<T> vptr<T>::operator-=(ptrdiff_t idx) {
  *this = *this - idx;
  return *this;
}
template <typename T>
vptr<T> vptr<T>::operator--() {
  return *this -= 1;
}
template <typename T>
vptr<T> vptr<T>::operator--(int) {
  auto prev = *this;
  --*this;
  return *this;
}

template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(Args&&... args) const
  requires best::constructible<T, Args&&...>
{
  check();

  // TODO: Add array traits, support multi-dimensional arrays?
  if constexpr (std::is_bounded_array_v<T> && sizeof...(Args) == 1 &&
                ((std::extent_v<best::as_auto<Args>> ==
                  std::extent_v<T>)&&...)) {
    for (size_t i = 0; i < std::extent_v<T>; ++i) {
      new (best::addr((*raw())[i])) std::remove_extent_t<T>(args[i]...);
    }
  } else if constexpr (best::is_object<T>) {
    new (raw()) T(BEST_FWD(args)...);
  } else if constexpr (best::is_ref<T>) {
    *const_cast<best::unqual<pointee>*>(raw()) = best::addr(args...);
  } else if constexpr (best::is_func<T>) {
    *const_cast<best::unqual<pointee>*>(raw()) = (args, ...);
  }
}
template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(
  best::args<Args...> args) const requires best::constructible<T, Args...>
{
  args.row.apply([&](auto&&... args) { construct(BEST_FWD(args)...); });
}
template <typename T>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(niche) const
  requires best::has_niche<T>
{
  check();
  if constexpr (best::is_object<T>) {
    new (raw()) T(niche{});
  } else if constexpr (best::is_ref<T>) {
    *const_cast<best::unqual<pointee>*>(raw()) = nullptr;
  }
}

template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::assign(Args&&... args) const
  requires best::assignable<T, Args&&...>
{
  check();
  if constexpr (std::is_bounded_array_v<T> && sizeof...(Args) == 1 &&
                ((std::extent_v<best::as_auto<Args>> ==
                  std::extent_v<T>)&&...)) {
    for (size_t i = 0; i < std::extent_v<T>; ++i) {
      (*this)[i] = (args[i], ...);
    }
  } else if constexpr (best::is_object<T>) {
    **this = (BEST_FWD(args), ...);
  } else {
    construct(BEST_FWD(args)...);
  }
}
template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::assign(
  best::args<Args...> args) const requires best::assignable<T, Args...>
{
  args.row.apply([&](auto&&... args) { assign(BEST_FWD(args)...); });
}

template <typename T>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::destroy() const
  requires best::destructible<T>
{
  check();
  if constexpr (best::is_object<T>) { raw()->~T(); }
  if (!std::is_constant_evaluated() && best::is_debug()) {
    BEST_PUSH_GCC_DIAGNOSTIC()
    BEST_IGNORE_GCC_DIAGNOSTIC("-Wdynamic-class-memaccess")
    // If T is polymorphic, Clang will whine that we're clobbering the vtable.
    // However, we literally just called the destructor, so it's simply wrong.
    // Hence, we disable the warning.
    best::ptr_internal::memset(raw(), Magic, sizeof(pointee));
    BEST_POP_GCC_DIAGNOSTIC()
  }
}

template <typename T>
BEST_INLINE_SYNTHETIC void vptr<T>::destroy() const
  requires best::destructible<T>
{
  (void)**this;
  vt_->dtor_(this->raw());
}

template <typename T>
template <ptr<T>::how how, typename U>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::copy_impl(ptr<U> that,
                                                       size_t count) const {
  if (count == 0) { return; }
  check();
  that.check();
  if constexpr (how.template can_memcpy<U>()) {
    if (how.should_memcpy()) {
      auto len = count * sizeof(pointee);
      if constexpr (how.overlapping) {
        best::ptr_internal::memmove(raw(), that.raw(), len);
      } else {
        best::ptr_internal::memcpy(raw(), that.raw(), len);
      }

      // Even when relocating we shouldn't run destructors here. However, we
      // can still clobber the moved-from bytes.
      if constexpr (how.kind == how::Relo) {
        if (!std::is_constant_evaluated() && best::is_debug()) {
          best::ptr_internal::memset(that.raw(), Magic, len);
        }
      }

      return;
    }
  }

  for (size_t i = 0; i < count; ++i) {
    if constexpr (how.assign) {
      if constexpr (how.kind == how::Copy) {
        (*this + i).assign(that[i]);
      } else {
        (*this + i).assign(BEST_MOVE(that[i]));
      }
    } else if constexpr (how.kind == how::Copy) {
      (*this + i).construct(that[i]);
    } else {
      (*this + i).construct(BEST_MOVE(that[i]));
    }

    if constexpr (how.kind == how::Relo) { (that + i).destroy(); }
  }
}

template <typename T>
BEST_INLINE_ALWAYS constexpr void ptr<T>::relo_overlapping(ptr<T> that,
                                                           size_t count) const {
  auto dst = *this;
  auto src = that;

  if (dst == src) { return; }

  // We need to handle the following cases.
  //
  // Non-overlapping shift. Happens when src + count <= dst or dst + count <=
  // src, need to destroy {.start = src, .count = count }.
  // | xxxx | yyyyyyyyyyyy | xxxxxxxxxxxx | ------------ | xxxx |
  //        src            src + count    dst            dst + count
  //
  // Overlapping forward shift. Happens when src < dst < src + count, need to
  // destroy {.start = src, .end = dest }
  // | xxxx | yyyyyyyyyyyy | yyyyyy | ------------ | xxxx |
  //        src            dst      src + count    dst + count
  //
  // The moved part is subdivided as follows according to how it needs to be
  // moved:
  // | aaaa | bbbbbbbbbbbb | cccc |
  // src    src + overlap  dst    src + count
  //
  // Where overlap = src + count - dst. The c part is move-constructed
  // but not destroyed; then the b part is relocated, and the a part is
  // move-assigned and then destroyed.
  //
  // Overlapping backward shift. Happens when dst < src < dest + count, need to
  // destroy {.start = dst + count, .end = src + count }
  // | xxxx | ------------ | yyyyyy | yyyyyyyyyyyy | xxxx |
  //        dst            src      dst + count    src + count
  //
  // The moved part is divided in the analogous way, but the move/relocate/
  // assign regions are in the opposite order.

  // Non-overlapping case.
  if (src + count <= dst || dst + count <= src) {
    dst.relo(src, count);
    return;
  }

  constexpr how how{.kind = how::Relo};
  if constexpr (how.template can_memcpy<T>()) {
    if (how.should_memcpy()) {
      auto len = count * sizeof(pointee);
      best::ptr_internal::memmove(raw(), that.raw(), len);
      return;
    }
  }

  // Forward case.
  if (src < dst && dst < src + count) {
    size_t overlap0 = src + count - dst;
    size_t overlap1 = dst - src;
    // Need to make the copies in backward order to avoid trampling.
    (dst + overlap1).move(src + overlap1, count - overlap1);
    (dst + overlap0).relo(src + overlap0, overlap1 - overlap0);
    dst.relo_assign(src, overlap0);
    return;
  }

  // Backward case.
  if (dst < src && src < dst + count) {
    size_t overlap0 = dst + count - src;
    size_t overlap1 = src - dst;

    dst.move(src, overlap0);
    (dst + overlap0).relo(src + overlap0, overlap1 - overlap0);
    (dst + overlap1).relo_assign(src + overlap1, count - overlap1);

    return;
  }
}

template <typename T>
BEST_INLINE_SYNTHETIC void vptr<T>::copy_to(void* to) const {
  if (!is_copyable()) {
    best::crash_internal::crash(
      "attempted to copy non-copyable type through a vptr at %p", raw());
  }
  vtable()->copy_(to, raw());
}
}  // namespace best

#define BEST_PTR_ _private

#endif  // BEST_MEMORY_PTR_H_
