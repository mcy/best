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

#ifndef BEST_META_TRAITS_H_
#define BEST_META_TRAITS_H_

#include "best/meta/internal/traits.h"

//! Traits for performing miscellaneous metaprogramming tasks. Many of these
//! traits are implemented by `<type_traits>` but have horrendous names.

namespace best {
/// # `best::id<T>`
///
/// The identity type trait. Useful for turning any single type into a tag, when
/// you don't need the full power of `best::tlist`.
template <typename T>
struct id final {
  using type = T;
};

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

/// # `best::dependent<...>
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

/// # `best::lie<T>`
///
/// Lies to the compiler that we can materialize a `T`. This is just a shorter
/// `std::declval()`.
template <traits_internal::nonvoid T>
T&& lie = [] {
  static_assert(
      sizeof(T) == 0,
      "attempted to tell a best::lie: this value cannot be materialized");
}();

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
}  // namespace best

#endif  // BEST_META_TRAITS_H_
