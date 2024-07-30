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

#ifndef BEST_MATH_INTERNAL_COMMON_INT_H_
#define BEST_MATH_INTERNAL_COMMON_INT_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/traits/types.h"

namespace best::int_internal {
template <typename T>
concept is_int =                          //
  std::is_same_v<T, char> ||              //
  std::is_same_v<T, signed char> ||       //
  std::is_same_v<T, signed short> ||      //
  std::is_same_v<T, signed int> ||        //
  std::is_same_v<T, signed long> ||       //
  std::is_same_v<T, signed long long> ||  //
  std::is_same_v<T, unsigned char> ||     //
  std::is_same_v<T, unsigned short> ||    //
  std::is_same_v<T, unsigned int> ||      //
  std::is_same_v<T, unsigned long> ||     //
  std::is_same_v<T, unsigned long long>;

template <typename Int, typename... Ints>
constexpr auto widest() {
  if constexpr (sizeof...(Ints) == 0) {
    return Int{};
  } else if constexpr (sizeof(widest<Ints...>()) < sizeof(Int)) {
    return Int{};
  } else {
    return widest<Ints...>();
  }
}

template <typename... Ints>
constexpr bool any_unsigned() {
  return (std::is_unsigned_v<Ints> || ...);
}

template <typename... Ints, bool u = any_unsigned<Ints...>(),
          typename W = decltype(widest<Ints...>())>
best::select<u, std::make_unsigned_t<W>, W> common();
}  // namespace best::int_internal

#endif  // BEST_MATH_INTERNAL_COMMON_INT_H_
