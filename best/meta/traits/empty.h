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

#ifndef BEST_META_TRAITS_EMPTY_H_
#define BEST_META_TRAITS_EMPTY_H_

#include <compare>
#include <type_traits>

#include "best/meta/traits/types.h"

//! Empty type traits.
//!
//! This header provides traits related to empty types, as well as a canonical
//! empty object type, `best::empty`.

namespace best {
/// # `best::is_void`
///
/// A void type, i.e. cv-qualified void.
template <typename T>
concept is_void = std::is_void_v<T>;

/// # `best::is_empty`
///
/// Whether this type counts as "empty", i.e., if it will trigger the empty
/// base class optimization. (Ish. Some compilers (MSVC) will lie about when
/// they will do the optimization.)
template <typename T>
concept is_empty = best::is_void<T> || std::is_empty_v<T>;

/// # `best::empty`
///
/// An empty type with minimal dependencies.
struct empty final {
  constexpr bool operator==(const empty& that) const = default;
  constexpr std::strong_ordering operator<=>(const empty& that) const = default;
};

/// # `best::devoid<T>`
///
/// If `T` is void, produces `best::empty`. Otherwise produces `T`.
template <typename T>
using devoid = best::select<best::is_void<T>, best::empty, T>;
}  // namespace best

#endif  // BEST_META_TRAITS_EMPTY_H_
