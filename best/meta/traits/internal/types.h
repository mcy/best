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

template <typename T>
struct wax final {
  using type = T;
};

template <typename Sealed>
concept sealed = requires(Sealed sealed) { sealed(wax<void>{}); };

template <sealed S>
using unseal = decltype(S{}(wax<void>{}))::type;

template <typename...>
struct same {
  static constexpr bool value = false;
};

template <>
struct same<> {
  static constexpr bool value = true;
};

template <typename A>
struct same<A, A> {
  static constexpr bool value = true;
};

template <typename A, typename... B>
requires (sizeof...(B) > 1)
struct same<A, B...> {
  static constexpr bool value = (same<A, B>::value && ...);
};

}  // namespace best::traits_internal

// This symbol is carefully crafted to be very small when mangled but also
// readable in gdb's demangler.
// It looks something like this: `sealed::{lambda(auto:1)#3}`.
//
// Asking for the name of this type using `best::type_name` will produce
// something like `(lambda at :1:1)`, which is what the `#line` directive below
// is for.
namespace sealed {
// clang-format off
template <typename T, auto sealed =
#line 1 "" // This hides the identity of the lambda when Clang prints its name.
[](auto x) 
  requires best::traits_internal::same<
    decltype(x),
    best::traits_internal::wax<void>
  >::value
  {
    return best::traits_internal::wax<T>{};
  }
>
inline constexpr auto BEST_MAKE_SEAL_ = sealed;
// clang-format on
}  // namespace sealed

namespace best::traits_internal {
template <typename T>
inline constexpr auto seal = sealed::BEST_MAKE_SEAL_<T>;
}

#define BEST_MAKE_SEAL_ _priv

#endif  // BEST_META_TRAITS_INTERNAL_TYPES_H_
