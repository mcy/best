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

#include "best/meta/tlist.h"

// This header contains implementations of the layout algorithms for structs and
// unions; they are implemented as variable templates to encourage the compiler
// to avoid recomputing them.

namespace best::layout_internal {
template <typename T>
using to_object = best::devoid<
    best::select<best::is_object<T> || best::is_void<T>, T, best::as_ptr<T>>>;

/// Computes the alignment of a struct/union with the given member types.
///
/// In other words, this computes the maximum among the alignments of Types.
/// Zero types produces an alignment of 1.
///
/// This is guaranteed to be a power of 2.
template <typename... Types>
inline constexpr size_t align_of = [] {
  size_t align = 1;
  best::types<to_object<Types>...>.each(
      [&]<typename T> { align = (align > alignof(T) ? align : alignof(T)); });
  return align;
}();

/// Computes the size of a struct with the given member types.
///
/// This executes the C/C++ standard layout algorithm. Zero types produces
/// a size of 1.
template <typename... Types>
inline constexpr size_t size_of = [] {
  if (sizeof...(Types) == 0) return size_t{1};

  size_t size = 0, align = 1;

  auto align_to = [&size](size_t align) {
    auto remainder = size % align;
    if (remainder != 0) {
      size += align - remainder;
    }
  };

  best::types<to_object<Types>...>.each([&]<typename T> {
    align_to(alignof(T));
    align = (align > alignof(T) ? align : alignof(T));
    size += sizeof(T);
  });

  align_to(align);
  return size;
}();

/// Computes the size of a union with the given member types.
///
/// In other words, this computes the maximum size among the member types,
/// rounded to the alignment of the most-aligned type. Zero types produces
/// a size of 1.
template <typename... Types>
inline constexpr size_t size_of_union = [] {
  if (sizeof...(Types) == 0) return size_t{1};

  size_t size = 0, align = 1;
  auto align_to = [&](size_t align) {
    auto remainder = size % align;
    if (remainder != 0) {
      size += align - remainder;
    }
  };

  best::types<to_object<Types>...>.each([&]<typename T> {
    align = (align > alignof(T) ? align : alignof(T));
    size = (size > sizeof(T) ? size : sizeof(T));
  });

  align_to(align);
  return size;
}();
}  // namespace best::layout_internal

#endif  // BEST_MEMORY_INTERNAL_LAYOUT_H_
