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

#ifndef BEST_META_TRAITS_ENUMS_H_
#define BEST_META_TRAITS_ENUMS_H_

#include <type_traits>

//! Type traits for user-defined enum types.
//!
//! This header provides traits for enums.

namespace best {
/// # `best::is_enum`
///
/// Identifies an enumeration type.
template <typename T>
concept is_enum = std::is_enum_v<T>;

/// # `best::as_underlying<E>`
///
/// The underlying integer type of `E`.
template <best::is_enum E>
using as_underlying = std::underlying_type_t<E>;

/// # `best::to_underlying()`
///
/// Casts any enum value into the corresponding integer type.
template <best::is_enum E>
constexpr best::as_underlying<E> to_underlying(E value) {
  return static_cast<best::as_underlying<E>>(value);
}
}  // namespace best

#endif  // BEST_META_TRAITS_ENUMS_H_
