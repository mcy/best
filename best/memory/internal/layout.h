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

#ifndef BEST_MEMORY_INTERNAL_LAYOUT_H_
#define BEST_MEMORY_INTERNAL_LAYOUT_H_

#include <cstddef>
#include <cstdlib>

#include "best/math/bit.h"
#include "best/meta/tlist.h"
#include "best/meta/traits/ptrs.h"

// This header contains implementations of the layout algorithms for structs and
// unions; they are implemented as variable templates to encourage the compiler
// to avoid recomputing them.

namespace best::layout_internal {
template <typename T>
using to_object = best::devoid<
  best::select<best::is_object<T> || best::is_void<T>, T, best::as_raw_ptr<T>>>;

/// Computes the alignment of a struct/union with the given member types.
///
/// In other words, this computes the maximum among the alignments of Types.
/// Zero types produces an alignment of 1.
///
/// This is guaranteed to be a power of 2.
template <typename... Types>
inline constexpr size_t align_of = best::max(size_t{1}, alignof(to_object<Types>)...);

/// Computes the size of a struct with the given member types.
///
/// This executes the C/C++ standard layout algorithm. Zero types produces
/// a size of 1.
template <typename... Types>
inline constexpr size_t size_of = [] {
  if (sizeof...(Types) == 0) { return size_t{1}; }

  size_t size = 0;
  best::types<to_object<Types>...>.each([&]<typename T> {
    size = best::round_up_to_pow2(size, alignof(T));
    size += sizeof(T);
  });

  return best::round_up_to_pow2(size, align_of<Types...>);
}();

/// Computes the size of a union with the given member types.
///
/// In other words, this computes the maximum size among the member types,
/// rounded to the alignment of the most-aligned type. Zero types produces
/// a size of 1.
template <typename... Types>
inline constexpr size_t size_of_union = [] {
  if constexpr (sizeof...(Types) == 0) { return size_t{1}; }

  return best::round_up_to_pow2(best::max(0, sizeof(to_object<Types>)...),
                                align_of<Types...>);
}();
}  // namespace best::layout_internal

#endif  // BEST_MEMORY_INTERNAL_LAYOUT_H_
