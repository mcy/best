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

#include "best/meta/traits.h"

namespace best::int_internal {
constexpr size_t widest(auto ints) {
  size_t idx = 0;
  size_t cur = 0;
  size_t size = 0;
  ints.each([&]<typename T> {
    if (sizeof(T) > size) {
      size = sizeof(T);
      cur = idx;
    }
    ++idx;
  });
  return cur;
}

constexpr bool any_unsigned(auto ints) {
  return ints.template map<std::is_unsigned>().any();
}

template <auto ints, bool u = any_unsigned(ints),
          typename W = decltype(ints)::template type<widest(ints)>>
best::select<u, std::make_unsigned_t<W>, W> common();
}  // namespace best::int_internal

#endif  // BEST_MATH_INTERNAL_COMMON_INT_H_
