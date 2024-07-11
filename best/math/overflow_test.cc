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

#include "best/math/overflow.h"

#include "best/math/int.h"

namespace best::overflow_test {
static_assert(overflow(max_of<int>) + 1 == overflow(min_of<int>, true));
static_assert(overflow(min_of<int>) + 1 == overflow(min_of<int> + 1));
static_assert(overflow(max_of<int>) - 1 == overflow(max_of<int> - 1));
static_assert(overflow(min_of<int>) - 1 == overflow(max_of<int>, true));

static_assert(overflow(0x1'0000) * 0x1'0000 == overflow(0, true));
static_assert(overflow(0x1'0000) * 2 == overflow(0x2'0000));

static_assert(overflow<short>(0x100) * overflow<short>(0x100) ==
              overflow(0, true));
static_assert(overflow(0x1'0000) * 2 == overflow(0x2'0000));

static_assert(overflow(min_of<int>) / -1 == overflow(min_of<int>, true));
static_assert(overflow(min_of<int>) % -1 == overflow(min_of<int>, true));

static_assert((overflow(42) << 33) == overflow(84, true));
static_assert((overflow(42) >> 33) == overflow(21, true));

static_assert((size_t(best::max_of<unsigned>) + overflow(1)).wrap() ==
              4294967296);

static_assert(best::saturating_add(max_of<int> - 5, 6) == best::max_of<int>);
static_assert(best::saturating_add(min_of<int> + 5, -6) == best::min_of<int>);
static_assert(best::saturating_add(max_of<unsigned> - 5, 6) ==
              best::max_of<unsigned>);

static_assert(best::saturating_sub(max_of<int> - 5, -6) == best::max_of<int>);
static_assert(best::saturating_sub(min_of<int> + 5, 6) == best::min_of<int>);
static_assert(best::saturating_sub(5u, 6u) == 0);

static_assert(best::saturating_mul(0x10000, 0x10000) == best::max_of<int>);
static_assert(best::saturating_mul(0x10000, -0x10000) == best::min_of<int>);
static_assert(best::saturating_mul(-0x10000, 0x10000) == best::min_of<int>);
static_assert(best::saturating_mul(-0x10000, -0x10000) == best::max_of<int>);

static_assert(best::saturating_mul(0x10000u, 0x10000u) ==
              best::max_of<unsigned>);

// TODO: Division by zero and strict() death tests.
}  // namespace best::overflow_test
