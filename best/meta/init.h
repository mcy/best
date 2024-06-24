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

#ifndef BEST_META_INIT_H_
#define BEST_META_INIT_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/internal/init.h"

//! Concepts for determining when a type can be initialized in a particular
//! way.
//!
//! This reflects the behavior of the various construction operations in
//! `best::object_ptr`.
//!
//! (Almost) all concepts in this header take variadic arguments, even if it
//! doesn't make sense (such as for assignment). This is because otherwise it's
//! not possible to pass an arbitrary pack in, which makes some generic code
//! awkward.

namespace best {
/// # `best::trivially`
///
/// Pass this to the first argument of any of the initialization concepts to
/// require that the initialization be trivial.
class trivially;

/// # `best::constructible`
///
/// Whether `T` can be initialized by direct construction from the given
/// arguments.
///
/// For object types, this is simply `std::is_constructible` and
/// `std::is_trivially_constructible`. Note that these imply
/// the respective versions of `std::is_destructible`. If the argument list is
/// a single `void` type, it is treated as-if it was empty.
///
/// For reference types, this is whether `Arg&` can convert to `T&` without
/// copies, i.e., if `Arg*` can convert to `T*`.
///
/// For void types, this is either no arguments, or a single argument of
/// any type, including void.
template <typename T, typename... Args>
concept constructible = requires {
  init_internal::ctor(init_internal::tag<T>{}, init_internal::tag<Args>{}...);
};

/// # `best::convertible`
///
/// Same as constructible, but moreover requires for object types that
/// initialization be possible by implicit conversion.
///
/// `T&&` cannot be converted to from any type, even `T&&` itself. This is to
/// avoid potential issues with binding to temporaries.
template <typename T, typename... Args>
concept convertible = requires {
  init_internal::conv(init_internal::tag<T>{}, init_internal::tag<Args>{}...);
};

/// # `best::assignable`
///
/// Whether `T` can be initialized by direct assignment from the given
/// arguments.
///
/// For object types, this is simply `std::is_assignable` and
/// `std::is_trivially_assignable`.
///
/// For all other types, this is `best::constructible` on single types. For
/// compatibility reasons, void types are assignable from nothing.
template <typename T, typename... Args>
concept assignable = requires {
  init_internal::assign(init_internal::tag<T>{}, init_internal::tag<Args>{}...);
};

/// # `best::converts_to`
///
/// A convenient version of `best::convertible` for using in `requires` clauses.
template <typename From, typename To>
concept converts_to = convertible<To, From>;

/// # `best::move_constructible`
///
/// Whether `T` is move-constructible.
template <typename T, typename... Args>
concept move_constructible =
    init_internal::only_trivial<Args...> &&  //
    best::convertible<T, Args..., T> &&
    best::convertible<T, Args..., std::add_rvalue_reference_t<T>>;

/// # `best::copy_constructible`
///
/// Whether T is copy-constructible.
template <typename T, typename... Args>
concept copy_constructible =
    init_internal::only_trivial<Args...> &&  //
    best::move_constructible<T, Args...> &&
    best::convertible<T, Args..., std::add_const_t<T>> &&
    best::convertible<T, Args...,
                      std::add_lvalue_reference_t<std::add_const_t<T>>>;

/// # `best::move_assignable`
///
/// Whether T is move-assignable.
template <typename T, typename... Args>
concept move_assignable =
    init_internal::only_trivial<Args...> &&  //
    best::assignable<T, Args..., T> &&
    best::assignable<T, Args..., std::add_rvalue_reference_t<T>>;

/// # `best::copy_assignable`
///
/// Whether T is copy-assignable.
template <typename T, typename... Args>
concept copy_assignable =
    init_internal::only_trivial<Args...> &&  //
    best::move_assignable<T, Args...> &&
    best::assignable<T, Args..., std::add_const_t<T>> &&
    best::assignable<T, Args...,
                     std::add_lvalue_reference_t<std::add_const_t<T>>>;

/// # `best::moveable`
///
/// Whether T is moveable.
template <typename T, typename... Args>
concept moveable =
    init_internal::only_trivial<Args...> &&  //
    best::move_constructible<T, Args...> && best::move_assignable<T, Args...>;

/// # `best::copyable`
///
/// Whether T is copyable.
template <typename T, typename... Args>
concept copyable =
    init_internal::only_trivial<Args...> &&  //
    best::copy_constructible<T, Args...> && best::copy_assignable<T, Args...>;

/// # `best::relocatable`
///
/// Whether T can be relocated.
///
/// Equivalent to `best::movable`; however, asking whether a type is trivially
/// relocatable, it is possible that some types that are not a fortiori
/// trivially moveable are trivially relocatable, assuming the move-and-destroy
/// operation is "fused".
///
/// To make a non-trivially-moveable type trivially relocatable, use the
/// `BEST_RELOCATABLE` macro.
template <typename T, typename... Args>
concept relocatable = init_internal::only_trivial<Args...> &&  //
                      (!std::is_object_v<T> ||                 //
                       (init_internal::is_trivial<Args...>
                            ? init_internal::trivially_relocatable<T>
                            : moveable<T>));
/// # `best::destructible`
///
/// Whether T can be destroyed.
///
/// For object types, this is simply `std::is_destructible` and
/// `std::is_trivially_destructible`
///
/// For all other types, this is vacuously true.
template <typename T, typename... Args>
concept destructible =
    init_internal::only_trivial<Args...> &&  //
    (!std::is_object_v<T> ||
     (init_internal::is_trivial<Args...> ? std::is_trivially_destructible_v<T>
                                         : std::is_destructible_v<T>));

/// A callback that constructs a `T`.
///
/// Currently only available for object types.
template <best::is_object T>
inline constexpr auto ctor = [](auto&&... args) -> T
  requires best::constructible<T, decltype(args)...>
{ return T{BEST_FWD(args)...}; };

}  // namespace best

#endif  // BEST_META_INIT_H_
