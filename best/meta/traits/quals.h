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

#ifndef BEST_META_TRAITS_QUALS_H_
#define BEST_META_TRAITS_QUALS_H_

#include <type_traits>

#include "best/meta/traits/internal/quals.h"

//! Qualifier type traits.
//!
//! This header provides traits related to the type qualifiers `const` and
//! `volatile`.

namespace best {
/// # `best::is_const`
///
/// Whether this is a `const`-qualified type. Function and reference types are
/// always implicitly `const`.
template <typename T>
concept is_const =
  std::is_same_v<T, typename best::traits_internal::const_<T>::add>;

/// # `best::as_const<T>`
///
/// Adds `const` to `T`. Note that this is not the same as `std::add_const_t`,
/// because it works properly on array types: `T[]` to `const T[]`.
///
/// This function leaves reference and function types unchanged.
template <typename T>
using as_const = best::traits_internal::const_<T>::add;

/// # `best::un_const<T>`
///
/// This is the inverse of `best::as_const<T>`.
template <typename T>
using un_const = best::traits_internal::const_<T>::remove;

/// # `best::is_volatile`
///
/// Whether this is a `volatile`-qualified type. Function and reference types
/// are always implicitly non-`volatile`.
template <typename T>
concept is_volatile =
  !std::is_reference_v<T> && !std::is_function_v<T> &&
  std::is_same_v<T, typename best::traits_internal::volatile_<T>::add>;

/// # `best::as_volatile<T>`
///
/// Adds `volatile` to `T`. Note that this is not the same as
/// `std::add_volatile_t`, because it works properly on array types: `T[]` to
/// `volatile T[]`.
///
/// This function leaves reference and function types unchanged.
template <typename T>
using as_volatile = best::traits_internal::volatile_<T>::add;

/// # `best::un_volatile<T>`
///
/// This is the inverse of `best::as_volatile<T>`.
template <typename T>
using un_volatile = best::traits_internal::volatile_<T>::remove;

/// # `best::un_qual<T>`
///
/// Removes all qualifiers from `T`.
template <typename T>
using un_qual = best::un_const<best::un_volatile<T>>;

/// # `best::qualcopy<Dst, Src>
///
/// Copies any top-level cv-qualification from `Src` onto `Dst`, as if by
/// the above traits.
template <typename Dst, typename Src>
using copy_quals = best::traits_internal::quals<Dst, Src>::copied;

/// # `best::qualifies_to<From, To>`
///
/// Returns whether you can build the type `To` by adding qualifiers to `From`.
template <typename From, typename To>
concept qualifies_to = std::is_same_v<From, To> ||               //
                       std::is_same_v<as_const<From>, To> ||     //
                       std::is_same_v<as_volatile<From>, To> ||  //
                       std::is_same_v<as_const<as_volatile<From>>, To>;
}  // namespace best

#endif  // BEST_META_TRAITS_QUALS_H_
