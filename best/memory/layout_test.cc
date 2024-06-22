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

#include "best/memory/layout.h"

namespace best::layout_test {
static_assert(best::align_of<int32_t> == 4);
static_assert(best::align_of<int&> == alignof(void*));
static_assert(best::align_of<int()> == alignof(void*));
static_assert(best::align_of<void> == 1);
static_assert(best::layout::of_struct<int16_t, int32_t>().align() == 4);

static_assert(best::size_of<int32_t> == 4);
static_assert(best::size_of<int&> == sizeof(void*));
static_assert(best::size_of<int()> == sizeof(void*));
static_assert(best::size_of<void> == 1);
static_assert(best::layout::of_struct<int16_t, int32_t>().size() == 8);
static_assert(best::layout::of_struct<int16_t, int32_t, uint8_t>().size() ==
              12);

static_assert(best::layout::of_union<int32_t>().size() == 4);
static_assert(best::layout::of_union<int&>().size() == sizeof(void*));
static_assert(best::layout::of_union<int()>().size() == sizeof(void*));
static_assert(best::layout::of_union<void>().size() == 1);
static_assert(best::layout::of_union<int16_t, int32_t>().size() == 4);
static_assert(best::layout::of_union<int16_t, int32_t, uint8_t>().size() == 4);

// TODO: Death tests for overflow.
}  // namespace best::layout_test
