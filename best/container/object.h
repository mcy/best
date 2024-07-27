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

#include <type_traits>

#include "best/base/ord.h"
#include "best/base/tags.h"
#include "best/memory/ptr.h"
#include "best/meta/empty.h"
#include "best/meta/init.h"

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
/// # `best::object<T>`
///
/// An "equivalent" object type for any type, intended primarily for generic
/// code.
///
/// This type wraps any `T` and reproduces its properties. The wrapped `T` can
/// be accessed via `operator*` and `operator->`.
template <typename T>
class object final {
 public:
  static_assert(best::is_thin<T>,
                "best::object<T> only makes sense for thin-pointer types");

  /// # `object::wrapped_type`
  ///
  /// The representation for the value we're wrapping.
  using wrapped_type = best::devoid<best::unqual<best::pointee<T>>>;

  using type = T;
  using value_type = best::as_auto<T>;

 private:
  static constexpr bool view_is_ref = best::is_ref<best::view<T>>;

 public:
  using cref =
    best::select<view_is_ref, best::as_ref<const type>,
                 decltype(best::lie<best::ptr<T>>.as_const().deref())>;
  using ref = best::select<view_is_ref, best::as_ref<type>, best::view<type>>;
  using crref =
    best::select<view_is_ref, best::as_rref<const type>,
                 decltype(best::lie<best::ptr<T>>.as_const().deref())>;
  using rref = best::select<view_is_ref, best::as_rref<type>, best::view<type>>;

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
    as_ptr().construct(BEST_FWD(args)...);
  }
  template <typename... Args>
  constexpr explicit object(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...> &&
             (best::is_object<T> && !std::is_array_v<T>)
    : BEST_OBJECT_VALUE_(BEST_FWD(args)...) {}
  constexpr explicit object(best::in_place_t, best::niche)
    requires best::is_ref<T>
    : BEST_OBJECT_VALUE_(nullptr) {}

  /// # `object::operator=()`
  ///
  /// Assigns to the wrapped value via `ptr::assign()`'s semantics.
  template <typename Arg>
  constexpr object& operator=(Arg&& arg) requires best::assignable<T, Arg>
  {
    as_ptr().assign(BEST_FWD(arg));
    return *this;
  }

  /// # `object::as_ptr()`.
  ///
  /// Extracts an `ptr<T>` pointing to this object.
  constexpr best::ptr<const T> as_ptr() const {
    return best::ptr(best::addr(BEST_OBJECT_VALUE_)).cast(best::types<const T>);
  }
  constexpr best::ptr<T> as_ptr() {
    return best::ptr(best::addr(BEST_OBJECT_VALUE_)).cast(best::types<T>);
  }

  /// # `ptr::operator*`, `ptr::operator->`
  ///
  /// Retrieves the wrapped value.
  constexpr cref operator*() const& { return *as_ptr().as_const(); }
  constexpr ref operator*() & { return *as_ptr(); }
  constexpr crref operator*() const&& { return static_cast<crref>(**this); }
  constexpr rref operator*() && { return static_cast<rref>(**this); }
  constexpr auto operator->() const { return as_ptr().operator->(); }
  constexpr auto operator->() { return as_ptr().operator->(); }

  /// # `object::or_empty()`
  ///
  /// Returns the result of `operator*`, or a `best::empty&` if `T` is of void
  /// type.
  constexpr decltype(auto) or_empty() const& {
    if constexpr (best::is_void<T>) {
      return BEST_OBJECT_VALUE_;
    } else {
      return **this;
    }
  }
  constexpr decltype(auto) or_empty() & {
    if constexpr (best::is_void<T>) {
      return BEST_OBJECT_VALUE_;
    } else {
      return **this;
    }
  }
  constexpr decltype(auto) or_empty() const&& {
    if constexpr (best::is_void<T>) {
      return BEST_MOVE(BEST_OBJECT_VALUE_);
    } else {
      return *BEST_MOVE(*this);
    }
  }
  constexpr decltype(auto) or_empty() && {
    if constexpr (best::is_void<T>) {
      return BEST_MOVE(BEST_OBJECT_VALUE_);
    } else {
      return *BEST_MOVE(*this);
    }
  }

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
  constexpr friend void BestFmtQuery(auto& query, object*) {
    query = query.template of<T>;
  }

 public:
  [[no_unique_address]] wrapped_type BEST_OBJECT_VALUE_;
};
#define BEST_OBJECT_VALUE_ _private
}  // namespace best

#endif  // BEST_CONTAINER_OBJECT_H_
