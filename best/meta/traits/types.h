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

#ifndef BEST_META_TRAITS_TYPES_H_
#define BEST_META_TRAITS_TYPES_H_

#include "best/meta/traits/internal/types.h"

//! Type traits.
//!
//! This header provides traits for operating on arbitrary types.
//!
//! Some of these traits are implemented by `<type_traits>` but have horrendous
//! names.

namespace best {
/// # `best::id<T>`
///
/// The identity type trait. Useful for turning any single type into a tag, when
/// you don't need the full power of `best::tlist`.
template <typename T>
struct id final {
  using type = T;
};

/// # `best::val<T>`
///
/// The identity value trait. Like `best::id`, but for values.
template <auto x>
struct val final {
  using type = decltype(x);
  static constexpr auto value = x;
};

/// # `best::dependent<...>`, `best::dependent_value<...>`
///
/// Makes a type dependent on template parameters.
///
/// There are sometimes cases where we want to force two-phase lookup[1] because
/// we are using e.g. an incomplete type in complete position in a template, and
/// the use is not dependent on the template parameters.
///
/// This always has the value of `T`, regardless of what `Deps` actually are.
///
/// [1]: https://en.cppreference.com/w/cpp/language/two-phase_lookup
template <typename T, typename... Deps>
using dependent = traits_internal::dependent<T, Deps...>::type;
template <auto value, typename... Deps>
using dependent_value =
  traits_internal::dependent<best::val<value>, Deps...>::type::value;

/// # `best::lie<T>`
///
/// Lies to the compiler that we can materialize a `T`. This is just a shorter
/// `std::declval()`.
using ::best::traits_internal::lie;

/// # `best::type_trait`
///
/// Whether `T` is a type trait, i.e., a type with an alias member named `type`.
template <typename T>
concept type_trait = requires { typename T::type; };

/// # `best::extract_trait<T>
///
/// A helper for extracting a type trait.
///
/// For example, passing this to `map` on a type list will evaluate all type
/// traits therein.
template <best::type_trait T>
using extract_trait = T::type;

/// # `best::value_trait`
///
/// Whether `T` is a value trait, i.e., a type with a static data member named
/// `value`.
template <typename T>
concept value_trait = requires { T::value; };

/// # `best::select<...>`
///
/// Selects one of two types depending on a boolean condition.
template <bool cond, typename A, typename B>
using select = traits_internal::select<cond, A, B>::type;

/// # `best::select_trait<...>`
///
/// Selects one of two type traits depending on a boolean condition, and
/// extracts the result. This is useful for delaying materialization of an
/// illegal construction if it would not be selected.
template <bool cond, best::type_trait A, best::type_trait B>
using select_trait = traits_internal::select<cond, A, B>::type::type;

/// # `best::abridge<T>`, `best::unabridge<T>`
///
/// Abridges a type name.
///
/// This produces a new type with a very small opaque name that can be
/// `best::unabridge`ed to produce the original type.
template <typename T>
using abridge = decltype(best::traits_internal::seal<best::id<T>>);
template <typename T>
using unabridge = best::traits_internal::unseal<T>::type;

/// # `best::abridged<T>`
///
/// A type generated with `best::abridge<T>`.
template <typename T>
concept abridged = best::traits_internal::sealed<T>;
}  // namespace best

#endif  // BEST_META_TRAITS_TYPES_H_
