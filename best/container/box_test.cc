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

#include "best/container/box.h"

#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::box_test {
using ::best_fodder::LeakTest;

best::test Thin = [](auto& t) {
  best::box x0(42);
  t.expect_eq(*x0, 42);

  best::option<best::box<int>> x1;
  t.expect_eq(x1, best::none);
  x1 = x0;
  t.expect_eq(**x1, 42);
  t.expect_eq(**x1, *x0);
};

best::test Span = [](auto& t) {
  best::box x0({1, 2, 3, 4, 5});
  t.expect_eq(*x0, best::span{1, 2, 3, 4, 5});

  best::option<best::box<int[]>> x1;
  t.expect_eq(x1, best::none);
  x1 = x0;
  t.expect_eq(*x1, best::span{1, 2, 3, 4, 5});
  t.expect_eq(*x1, *x0);
  t.expect_eq(x1, x0);

  x0 = best::box<int[]>();
  t.expect_eq(x0->size(), 0);
};

best::test Leaky = [](auto& t) {
  LeakTest l_(t);

  using Bubble = LeakTest::Bubble;

  auto x0 = best::box(Bubble());
  x0 = best::box(Bubble());

  auto x1 = x0;
  auto x2 = std::move(x0);

  x2 = x1;
  x2 = std::move(x1);

  x0 = best::box(Bubble());
  x0 = x2;
  x0 = best::box(Bubble());
  x2 = x0;

  best::box x3({Bubble{}, {}, {}});
  auto x4 = x3;
  x4 = x3;
  auto x5 = std::move(x3);
  x4 = best::box({Bubble{}});
};
}  // namespace best::box_test
