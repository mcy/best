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

#ifndef BEST_META_TAXONOMY_H_
#define BEST_META_TAXONOMY_H_

#include <concepts>
#include <type_traits>

//! Concepts for identifying and destructuring basic C++ types.
//!
//! Other headers may define `is_*<T>` concepts for identifying `best`'s generic
//! container types.

namespace best {

/*
/// # `best::is_deref<T>`
///
/// Whether a type `T` is "properly dereferenceable", that is, it has an
/// `operator*` that returns some type, and an `operator->` that returns a
/// pointer. If `Target` is `void`, the types of these operators is not checked.
/// Otherwise, both operators must yield reference/pointer to `Target`,
/// respectively.
template <typename T, typename Target = void>
concept is_deref =
  (is_ptr<T> &&
   (is_void<Target> || std::is_same_v<Target, best::un_qual<best::unptr<T>>>))
|| (is_void<T> && requires(T& r) { operator*(r); r.operator->();
  }) || requires(const T& cr) {
    { operator*(cr) } -> std::convertible_to<const Target&>;
    { cr.operator->() } -> std::convertible_to<const Target*>;
  };*/
}  // namespace best

#endif  // BEST_META_TAXONOMY_H_
