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

#include "best/base/ord.h"

#include "best/test/test.h"

namespace best::ord_test {
best::test Eq = [](auto& t) {
  t.expect(best::equal(1, 1));
  t.expect(!best::equal(1, 2));

  int a = 0;
  float b = 0;
  int* x = &a;
  float* y = &b;
  float* z = reinterpret_cast<float*>(x);
  t.expect(best::equal(x, z));
  t.expect(!best::equal(x, y));

  t.expect(!best::equal(1, x));
};

best::test Chain = [](auto& t) {
  t.expect_eq(
      best::ord::equal->*best::or_cmp([] { return best::ord::greater; }),
      best::ord::greater);
  t.expect_eq(best::ord::less->*best::or_cmp([] { return best::ord::greater; }),
              best::ord::less);
};

static_assert(same<common_ord<>, ord>);
static_assert(same<common_ord<ord>, ord>);
static_assert(same<common_ord<ord, decltype(Less)>, ord>);
static_assert(same<common_ord<ord, decltype(Unordered)>, partial_ord>);
static_assert(same<common_ord<ord, ord>, ord>);
static_assert(same<common_ord<ord, ord, ord>, ord>);
static_assert(same<common_ord<partial_ord>, partial_ord>);
static_assert(same<common_ord<partial_ord, decltype(Less)>, partial_ord>);
static_assert(same<common_ord<ord, partial_ord>, partial_ord>);
static_assert(same<common_ord<ord, partial_ord, ord>, partial_ord>);
}  // namespace best::ord_test
