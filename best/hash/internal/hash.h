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

#ifndef BEST_HASH_INTERNAL_HASH_H_
#define BEST_HASH_INTERNAL_HASH_H_

#include "best/base/fwd.h"

namespace best::hash_internal {
// Dummy hash state for best::hashable.
template <typename C>
struct dummy final {
  using output = uint64_t;

  void write(best::span<C>);
  output finish();
};
}  // namespace best::hash_internal

#endif  // BEST_HASH_INTERNAL_HASH_H_