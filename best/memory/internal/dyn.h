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

#ifndef BEST_MEMORY_INTERNAL_PTR2_H_
#define BEST_MEMORY_INTERNAL_PTR2_H_

#include <cstddef>

#include "best/base/access.h"

namespace best::dyn_internal {
struct access {
  template <typename I>
  static constexpr bool can_wrap() {
    return requires(void* vp, const typename I::BestVtable* vt) {
      { best::access::constructor<I>(vp, vt) };
    };
  }

  template <typename I>
  static constexpr I wrap(const void* vp, const typename I::BestVtable* vt) {
    return best::access::constructor<I>(const_cast<void*>(vp), vt);
  }
};
}  // namespace best::dyn_internal

#endif