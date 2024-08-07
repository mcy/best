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

#include "best/iter/iter.h"

#include "best/iter/bounds.h"
#include "best/test/test.h"

namespace best::iter_test {
best::test Map = [](auto& t) {
  best::vec<size_t> ints = best::bounds{.start = 5, .count = 7}
                             .iter()
                             .map([](int x) { return x * x; })
                             .collect();
  t.expect_eq(ints, {25, 36, 49, 64, 81, 100, 121});
};

best::test Count = [](auto& t) {
  t.expect_eq(best::bounds{.start = 5, .count = 7}.iter().count(), 7);

  size_t calls = 0;
  t.expect_eq(best::bounds{.start = 5, .count = 7}
                .iter()
                .inspect([&] { ++calls; })
                .count(),
              7);
  t.expect_eq(calls, 7);
};

best::test Enumerate = [](auto& t) {
  best::vec pairs(best::bounds{.start = 5, .count = 7}.iter().enumerate());
  t.expect_eq(pairs,
              {{0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9}, {5, 10}, {6, 11}});
};

best::test Take = [](auto& t) {
  best::vec x0(best::bounds{.start = 5, .count = 7}.iter().take(3));
  t.expect_eq(x0, {5, 6, 7});

  best::vec x1(best::bounds{.start = 5, .count = 7}.iter().take(20));
  t.expect_eq(x1, {5, 6, 7, 8, 9, 10, 11});
};
};  // namespace best::iter_test
