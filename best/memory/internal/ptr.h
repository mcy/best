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

#ifndef BEST_MEMORY_INTERNAL_PTR_H_
#define BEST_MEMORY_INTERNAL_PTR_H_

#include <cstddef>

#include "best/base/hint.h"
#include "best/base/port.h"

#define BEST_CONSTEXPR_MEMCPY_ BEST_HAS_FEATURE(cxx_constexpr_string_builtins)

namespace best::ptr_internal {
#if BEST_CONSTEXPR_MEMCPY_
BEST_INLINE_SYNTHETIC constexpr void* memcpy(void* dst, const void* src,
                                             size_t len) {
  return __builtin_memcpy(dst, src, len);
}
BEST_INLINE_SYNTHETIC constexpr void* memmove(void* dst, const void* src,
                                              size_t len) {
  return __builtin_memmove(dst, src, len);
}
#else
extern "C" {
void* memcpy(void*, const void*, size_t) noexcept;
void* memmove(void*, const void*, size_t) noexcept;
}
#endif

extern "C" void* memset(void*, int, size_t) noexcept;
}  // namespace best::ptr_internal

#endif  // BEST_MEMORY_INTERNAL_PTR_H_
