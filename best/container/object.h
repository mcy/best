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

#ifndef BEST_CONTAINER_OBJECT_H_
#define BEST_CONTAINER_OBJECT_H_

#include <stddef.h>

#include <memory>
#include <type_traits>

#include "best/base/ord.h"
#include "best/base/port.h"
#include "best/meta/empty.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"
#include "best/meta/traits.h"

//! Objectification: turn any C++ type into a something you can use as a class
//! member.
//!
//! Not all C++ types are created equal. Most are "object types", which is
//! anything that is not a reference type, a function type (like `int(int)`)
//! or `void`.
//!
//! `best::object` is a workaround for this. For example, `best::object<T&>` is
//! just a weird pointer.

namespace best {
/// # `best::niche`
///
/// A tag for constructing niche representations.
///
/// A niche representation of a type `T` is a `best::object<T>` that contains an
/// otherwise invalid value of `T`. No operations need to be valid for a niche
/// representation, not even the destructor.
///
/// If `T` has a niche representation, it must satisfy `best::constructible<T,
/// niche>` and `best::equatable<T, niche>`. Only the niche representation
/// (obtained only by constructing from a `best::niche`) must compare as equal
/// to `best::niche`.
///
/// Niche representations are used for compressing the layout of some types,
/// such as `best::choice`.
struct niche final {};

/// # `best::has_niche`
///
/// Whether T is a type with a niche.
template <typename T>
concept has_niche =
    best::is_ref<T> || (best::is_object<T> && best::constructible<T, niche> &&
                        best::equatable<T, best::niche>);

/// # `best::object_ptr<T>`
///
/// A pointer to a possibly non-object `T`.
///
/// This allows creating a handle to a possibly non-object type that can be
/// manipulated in a consistent manner.
///
/// The mapping is as follows:
///
/// ```text
/// best::is_object T          -> T*
/// best::is_ref T             -> const base::as_ptr<T>*
/// best::abominable_func_type T -> const base::as_ptr<T>*
/// best::is_void T            -> void*
/// ```
/// Note the `const` for reference types: this reflects the fact that e.g.
/// `T& const` is just `T&`.
///
/// Note that a `best::object_ptr<void>` is *not* a `void*` in so far that it
/// does not represent a type-erased pointer.
template <typename T>
class object_ptr final {
 public:
  /// # `object_ptr::type`.
  ///
  /// The wrapped type; `object_ptr<T>` is nominally a `T*`.
  using type = T;

  /// # `object_ptr::pointee`.
  ///
  /// The "true" pointee type. Internally, an `object_ptr` stores a `pointee*`.
  using pointee = best::select<best::is_object<T> || best::is_void<T>, T,
                               const best::as_ptr<T>>;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;
  using cptr = best::as_ptr<const T>;
  using ptr = best::as_ptr<T>;

  /// # `object_ptr::object_ptr()`
  ///
  /// Constructs a null pointer.
  constexpr object_ptr() = default;
  constexpr object_ptr(std::nullptr_t){};

  /// # `object_ptr::object_ptr(object_ptr)`
  ///
  /// Trivially copyable.
  constexpr object_ptr(const object_ptr&) = default;
  constexpr object_ptr& operator=(const object_ptr&) = default;
  constexpr object_ptr(object_ptr&&) = default;
  constexpr object_ptr& operator=(object_ptr&&) = default;

  /// # `object_ptr::object_ptr(ptr)`
  ///
  /// Wraps a C++ pointer, potentially casting it if necessary.
  template <typename U>
  constexpr explicit(!best::qualifies_to<U, pointee> && !best::is_void<T>)
      object_ptr(U* ptr)
      : BEST_OBJECT_PTR_(ptr) {}

  /// # `object_ptr::object_ptr(object_ptr<U>)`
  ///
  /// Wraps an `object_ptr` of a different type, potentially casting it if
  /// necessary.
  template <typename U>
  constexpr explicit(!best::qualifies_to<U, pointee> && !best::is_void<T>)
      object_ptr(object_ptr<U> ptr)
      : BEST_OBJECT_PTR_(
            const_cast<best::unqual<typename object_ptr<U>::pointee>*>(
                ptr.raw())) {}

  /// # `object_ptr::dangling()`
  ///
  /// Returns a non-null but dangling pointer, which is unique for the choice of
  /// `T`.
  static constexpr object_ptr dangling() { return best::addr(dangling_); }

  /// # `object_ptr::raw()`.
  ///
  /// Returns the raw underlying pointer.
  constexpr pointee* raw() const { return BEST_OBJECT_PTR_; }

  /// Pointers may be compared by address.
  template <typename U>
  constexpr bool operator==(object_ptr<U> that) const {
    return best::equal(raw(), that.raw());
  };
  template <typename U>
  constexpr best::ord operator<=>(object_ptr<U> that) const {
    return best::compare(raw(), that.raw());
  };
  template <typename U>
  constexpr bool operator==(U* that) const {
    return best::equal(raw(), that);
  };
  template <typename U>
  constexpr best::ord operator<=>(U* that) const {
    return best::compare(raw(), that);
  };
  constexpr operator pointee*() const { return raw(); }

  /// # `object_ptr::operator*`, `object_ptr::operator->`
  ///
  /// Dereferences this pointer.
  ///
  /// For example, this will dereference the wrapping pointer of a `T&`, so
  /// if `T = U&`, then `raw = U**` and this dereferences that twice.
  constexpr ref operator*() const {
    if constexpr (best::is_object<T>) {
      return *raw();
    } else if constexpr (best::is_void<T>) {
      return;
    } else {
      return **raw();
    }
  }
  BEST_INLINE_SYNTHETIC constexpr ptr operator->() const {
    if constexpr (best::is_object<T> || best::is_void<T>) {
      return raw();
    } else {
      return *raw();
    }
  }

  /// # `object_ptr[idx]`, `object_ptr::operator+`, `object_ptr::operator-`
  ///
  /// Performs unchecked pointer arithmetic in the way you'd expect.
  constexpr ref operator[](size_t idx) const { return *(*this + idx); }

  constexpr object_ptr operator+(ptrdiff_t idx) const {
    if constexpr (best::is_void<T>) {
      return *this;
    } else {
      return object_ptr(raw() + idx);
    }
  }
  constexpr object_ptr operator+=(ptrdiff_t idx) {
    *this = *this + idx;
    return *this;
  }
  constexpr object_ptr operator++() { return *this += 1; }
  constexpr object_ptr operator++(int) {
    auto prev = *this;
    ++*this;
    return *this;
  }

  constexpr object_ptr operator-(ptrdiff_t idx) const {
    if constexpr (best::is_void<T>) {
      return *this;
    } else {
      return object_ptr(raw() - idx);
    }
  }
  constexpr object_ptr operator-=(ptrdiff_t idx) {
    *this = *this - idx;
    return *this;
  }
  constexpr object_ptr operator--() { return *this -= 1; }
  constexpr object_ptr operator--(int) {
    auto prev = *this;
    --*this;
    return *this;
  }

  /// # `object_ptr::construct_in_place()`
  ///
  /// Constructs in place according to the constructors valid per
  /// `best::constructible`.
  ///
  /// Note that this function implicitly casts away an untracked const if called
  /// on an `object_ptr<T&>` or `object_ptr<T()>`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct_in_place(Args&&... args) const
    requires best::constructible<T, Args&&...>
  {
    if constexpr (std::is_bounded_array_v<T> && sizeof...(Args) == 1 &&
                  ((std::extent_v<best::as_auto<Args>> ==
                    std::extent_v<T>)&&...)) {
      for (size_t i = 0; i < std::extent_v<T>; ++i) {
        std::construct_at(best::addr((*raw())[i]), args[i]...);
      }
    } else if constexpr (best::is_object<T>) {
      std::construct_at(raw(), BEST_FWD(args)...);
    } else if constexpr (best::is_ref<T>) {
      *const_cast<best::unqual<pointee>*>(raw()) = best::addr(args...);
    } else if constexpr (best::is_func<T>) {
      *const_cast<best::unqual<pointee>*>(raw()) = (args, ...);
    }
  }
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct_in_place(
      best::args<Args...> args) const
    requires best::constructible<T, Args...>
  {
    args.row.apply(
        [&](auto&&... args) { construct_in_place(BEST_FWD(args)...); });
  }

  /// # `object_ptr::construct_in_place(niche)`
  ///
  /// Constructs a niche representation in-place. Note that references have a
  /// niche: the null pointer.
  BEST_INLINE_SYNTHETIC constexpr void construct_in_place(niche) const
    requires best::has_niche<T>
  {
    if constexpr (best::is_object<T>) {
      std::construct_at(raw(), niche{});
    } else if constexpr (best::is_ref<T>) {
      *const_cast<best::unqual<pointee>*>(raw()) = nullptr;
    }
  }

  /// # `object_ptr::is_niche()`
  ///
  /// Whether this value is a niche representation.
  constexpr bool is_niche() const {
    if constexpr (best::has_niche<T>) {
      if constexpr (best::is_ref<T>) {
        return *raw() == nullptr;
      } else {
        return **this == niche{};
      }
    }
    return false;
  }

  /// # `object_ptr::destroy_in_place()`
  ///
  /// Destroys the pointed to object in place.
  BEST_INLINE_SYNTHETIC constexpr void destroy_in_place() const
    requires best::destructible<T>
  {
    if constexpr (best::is_object<T>) {
      std::destroy_at(raw());
      if (!std::is_constant_evaluated() && best::is_debug()) {
        std::memset(raw(), 0xcd, sizeof(pointee));
      }
    }
  }

  /// # `object_ptr::assign()`
  ///
  /// Assigns in place according to the assignments valid per
  /// `best::assignable`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(Args&&... args) const
    requires best::assignable<T, Args&&...>
  {
    if constexpr (std::is_bounded_array_v<T> && sizeof...(Args) == 1 &&
                  ((std::extent_v<best::as_auto<Args>> ==
                    std::extent_v<T>)&&...)) {
      for (size_t i = 0; i < std::extent_v<T>; ++i) {
        (*this)[i] = (args[i], ...);
      }
    } else if constexpr (best::is_object<T>) {
      **this = (BEST_FWD(args), ...);
    } else {
      construct_in_place(BEST_FWD(args)...);
    }
  }
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(best::args<Args...> args) const
    requires best::constructible<T, Args...>
  {
    args.row.apply([&](auto&&... args) { assign(BEST_FWD(args)...); });
  }

  /// # `object_ptr::copy_from()`
  ///
  /// Constructs a copy of `that`'s pointee. `is_init` specifies whether to
  /// construct or assign in-place.
  BEST_INLINE_SYNTHETIC constexpr void copy_from(object_ptr<const T> that,
                                                 bool is_init) const
    requires best::copyable<T>
  {
    if constexpr (!best::is_void<T>) {
      if (is_init) {
        assign(*that);
      } else {
        construct_in_place(*that);
      }
    }
  }
  /// # `object_ptr::move_from()`
  ///
  /// Constructs a move of `that`'s pointee. `is_init` specifies whether to
  /// construct or assign in-place.
  BEST_INLINE_SYNTHETIC constexpr void move_from(object_ptr that,
                                                 bool is_init) const
    requires best::moveable<T>
  {
    if constexpr (!best::is_void<T>) {
      if (is_init) {
        assign(BEST_MOVE(*that));
      } else {
        construct_in_place(BEST_MOVE(*that));
      }
    }
  }
  /// # `object_ptr::relocate_from()`
  ///
  /// Relocates `that`'s pointee. In other words, this moves from and then
  /// destroys `*that`. `is_init` specifies whether to construct or assign
  /// in-place.
  BEST_INLINE_SYNTHETIC constexpr void relocate_from(object_ptr that,
                                                     bool is_init) const
    requires best::relocatable<T>
  {
    move_from(that, is_init);
    that.destroy_in_place();
  }

 private:
  inline static best::select<best::is_void<T>, best::empty, pointee> dangling_;

 public:
  // Public for structural-ness.
  pointee* BEST_OBJECT_PTR_ = nullptr;
#define BEST_OBJECT_PTR_ _private
};

namespace object_internal {
template <best::is_void T>
best::empty* wrap_impl();
template <best::is_object T>
T* wrap_impl();
template <typename T>
best::as_ptr<T>* wrap_impl();

template <typename T>
using wrap = best::unptr<decltype(wrap_impl<T>())>;
}  // namespace object_internal

/// # `best::object<T>`
///
/// An "equivalent" object type for any type, intended primarily for generic
/// code.
///
/// This type wraps any `T` and reproduces its properties. The wrapped `T` can
/// be accessed via `operator*` and `operator->`.
template <typename T>
class object final :
    // Public so we can be structural.
    public best::ebo<object_internal::wrap<T>, T> {
 private:
  using base = best::ebo<object_internal::wrap<T>, T>;

 public:
  /// # `object::wrapped_type`
  ///
  /// The representation for the value we're wrapping.
  using wrapped_type = object_internal::wrap<T>;

  using type = T;
  using value_type = best::as_auto<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  /// # `object::object()`
  ///
  /// Constructs a default value. For some types, such as integers, pointers,
  /// and references, the resulting value will be uninitialized.
  constexpr object() = default;

  /// # `object::object(object)`
  ///
  /// Inherits the copy operations of the wrapped
  constexpr object(const object&) = default;
  constexpr object& operator=(const object&) = default;
  constexpr object(object&&) = default;
  constexpr object& operator=(object&&) = default;

  /// # `object::object(in_place, ...)`
  ///
  /// Constructs an object by deferring to the appropriate downstream
  /// constructor.
  template <typename... Args>
  constexpr explicit object(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...> &&
             (!best::is_object<T> || std::is_array_v<T>)
  {
    as_ptr().construct_in_place(BEST_FWD(args)...);
  }
  template <typename... Args>
  constexpr explicit object(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...> &&
             (best::is_object<T> && !std::is_array_v<T>)
      : base(best::in_place, BEST_FWD(args)...) {}
  constexpr explicit object(best::in_place_t, best::niche)
    requires best::is_ref<T>
      : base(best::in_place, nullptr) {}

  /// # `object::operator=()`
  ///
  /// Assigns to the wrapped value via `object_ptr::assign()`'s semantics.
  template <typename Arg>
  constexpr object& operator=(Arg&& arg)
    requires best::assignable<T, Arg>
  {
    as_ptr().assign(BEST_FWD(arg));
    return *this;
  }

  /// # `object::as_ptr()`.
  ///
  /// Extracts an `object_ptr<T>` pointing to this object.
  constexpr best::object_ptr<const T> as_ptr() const {
    return best::object_ptr<const T>(best::addr(base::get()));
  }
  constexpr best::object_ptr<T> as_ptr() {
    return best::object_ptr<T>(best::addr(base::get()));
  }

  /// # `object_ptr::operator*`, `object_ptr::operator->`
  ///
  /// Retrieves the wrapped value.
  constexpr cref operator*() const& { return *as_ptr(); }
  constexpr ref operator*() & { return *as_ptr(); }
  constexpr crref operator*() const&& { return BEST_MOVE(**this); }
  constexpr rref operator*() && { return BEST_MOVE(**this); }
  constexpr cptr operator->() const { return as_ptr().operator->(); }
  constexpr ptr operator->() { return as_ptr().operator->(); }

  // Comparisons are the obvious thing.
  constexpr bool operator==(const object& that) const
    requires best::equatable<T, T>
  {
    if constexpr (best::is_void<T>) {
      return true;
    } else {
      return **this == *that;
    }
  }
  constexpr auto operator<=>(const object& that) const
    requires best::comparable<T, T>
  {
    if constexpr (best::is_void<T>) {
      return best::ord::equal;
    } else {
      return **this <=> *that;
    }
  }

  friend void BestFmt(auto& fmt, const object& obj)
    requires best::is_void<T> || requires { fmt.format(*obj); }
  {
    if constexpr (best::is_void<T>) {
      fmt.write("void");
    } else {
      fmt.format(*obj);
    }
  }
  friend constexpr void BestFmtQuery(auto& query, object*) {
    query = query.template of<T>;
  }
};
}  // namespace best

#endif  // BEST_CONTAINER_OBJECT_H_
