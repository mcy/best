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

#ifndef BEST_META_TRAITS_INTERNAL_TYPES_H_
#define BEST_META_TRAITS_INTERNAL_TYPES_H_

#include <type_traits>

namespace best::traits_internal {
template <typename T, typename...>
struct dependent {
  using type = T;
};

template <bool cond, typename A, typename B>
struct select {
  using type = A;
};
template <typename A, typename B>
struct select<false, A, B> {
  using type = B;
};

template <typename T>
concept non_void = !std::is_void_v<T>;

template <traits_internal::non_void T>
T&& lie = [] {
  static_assert(
    sizeof(T) == 0,
    "attempted to tell a best::lie: this value cannot be materialized");
}();

struct wax {};
template <typename Sealed>
concept sealed = requires(Sealed sealed) {
  sealed(wax{});
  +sealed;  // Ensure that the user isn't passing a generic lambda.
};

template <typename T, sealed auto sealed = [](wax) { return T{}; }>
inline constexpr auto seal = sealed;

template <sealed S>
using unseal = decltype(S{}(wax{}));
}  // namespace best::traits_internal

#endif  // BEST_META_TRAITS_INTERNAL_TYPES_H_
