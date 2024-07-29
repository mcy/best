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

#ifndef BEST_META_TRAITS_FUNCS_H_
#define BEST_META_TRAITS_FUNCS_H_

#include <type_traits>

#include "best/meta/traits/internal/funcs.h"

//! Function type traits.
//!
//! This header provides traits related to function types. C++ function types
//! are distinct from function pointers, and include so-called "abominable
//! function types", which cannot be made into function pointers.

namespace best {
/// # `best::is_func`
///
/// A function type. This includes "abominable function" types that cannot form
/// function pointers, such as `int (int) const`.
template <typename T>
concept is_func = std::is_function_v<T>;

/// # `best::is_tame_func`
///
/// A tame function type. These are function types `F` such that `F*` is
/// well-formed; they lack ref and cv qualifiers.
template <typename T>
concept is_tame_func = best::is_func<T> && requires { (T*){}; };

/// # `best::is_abominable`
///
/// An abominable function type, i.e., a function type with qualifiers.
template <typename T>
concept is_abominable = best::is_func<T> && !requires { (T*){}; };

/// # `best::is_const_func`
///
/// Whether this is a `const`-qualified abominable function.
template <typename T>
concept is_const_func = best::traits_internal::tame<T>::c;

/// # `best::is_lref_func`
///
/// Whether this is a `&`-qualified abominable function.
template <typename T>
concept is_lref_func = best::traits_internal::tame<T>::l;

/// # `best::is_rref_func`
///
/// Whether this is a `&&`-qualified abominable function.
template <typename T>
concept is_rref_func = best::traits_internal::tame<T>::r;

/// # `best::is_ref_func`
///
/// Whether this is a `&`- or `&&-qualified abominable function.
template <typename T>
concept is_ref_func = best::is_lref_func<T> || best::is_rref_func<T>;

/// # `best::tame<T>`
///
/// If `T` is an abominable function, returns its "tame" counterpart. Otherwise,
/// returns `T`.
template <typename T>
using tame = best::traits_internal::tame<T>::type;
}  // namespace best

#endif  // BEST_META_TRAITS_FUNCS_H_
