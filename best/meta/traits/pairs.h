
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

#ifndef BEST_META_TRAITS_PAIRS_H_
#define BEST_META_TRAITS_PAIRS_H_

#include "best/meta/traits/types.h"

//! Type traits for working with pair-like types.

namespace best {
/// # `best::is_pair`
///
/// Returns whether a type is pair-like, in that it can be destructured into
/// exactly two elements using structured bindings.
template <typename P>
concept is_pair = requires {
  {
    [](P pair) { auto&& [a, b] = BEST_FWD(pair); }
  };
};

/// # `best::first_in<P>`, `best::second_in<P>`
///
/// Returns the type of the first or second value in a pair, as defined by
/// `best::is_pair`.
template <best::is_pair P>
using first_in = decltype([](auto&& pair) -> decltype(auto) {
  auto&& [a, b] = BEST_FWD(pair);
  return BEST_FWD(a);
}(best::lie<P>));
template <best::is_pair P>
using second_in = decltype([](auto&& pair) -> decltype(auto) {
  auto&& [a, b] = BEST_FWD(pair);
  return BEST_FWD(b);
}(best::lie<P>));

}  // namespace best

#endif  // BEST_META_TRAITS_PAIRS_H_