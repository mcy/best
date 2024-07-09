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

#include "best/iter/bounds.h"

#include "best/test/test.h"

namespace best::bounds_test {
best::test ComputeCount = [](auto& t) {
  auto count = [](bounds b, container_internal::option<size_t> max = {}) {
    return b.try_compute_count(max);
  };

  t.expect_eq(count({}), {});
  t.expect_eq(count({.start = 4}), {});
  t.expect_eq(count({.end = 4}), 4);
  t.expect_eq(count({.including_end = 4}), 5);
  t.expect_eq(count({.count = 4}), 4);
  t.expect_eq(count({.start = 4, .end = 4}), 0);
  t.expect_eq(count({.start = 4, .end = 5}), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}), 2);
  t.expect_eq(count({.start = 4, .count = 4}), 4);

  t.expect_eq(count({}, 10), 10);
  t.expect_eq(count({.start = 4}, 10), 6);
  t.expect_eq(count({.end = 4}, 10), 4);
  t.expect_eq(count({.including_end = 4}, 10), 5);
  t.expect_eq(count({.count = 4}, 10), 4);
  t.expect_eq(count({.start = 4, .end = 4}, 10), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 10), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}, 10), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}, 10), 2);
  t.expect_eq(count({.start = 2, .count = 2}, 10), 2);

  t.expect_eq(count({.start = 4}, 4), 0);
  t.expect_eq(count({.end = 4}, 4), 4);
  t.expect_eq(count({.including_end = 4}, 4), {});
  t.expect_eq(count({.count = 4}, 4), 4);
  t.expect_eq(count({.start = 4, .end = 4}, 5), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 5), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}, 5), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}, 5), {});
  t.expect_eq(count({.start = 2, .count = 2}, 4), 2);

  t.expect_eq(count({.start = 4}, 3), {});
  t.expect_eq(count({.end = 4}, 3), {});
  t.expect_eq(count({.including_end = 4}, 3), {});
  t.expect_eq(count({.count = 4}, 3), {});
  t.expect_eq(count({.start = 4, .end = 4}, 4), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 4), {});
  t.expect_eq(count({.start = 4, .including_end = 4}, 4), {});
  t.expect_eq(count({.start = 4, .including_end = 5}, 4), {});
  t.expect_eq(count({.start = 2, .count = 2}, 3), {});
};

best::test Debug = [](auto& t) {
  t.expect_eq(best::format("{:?}", bounds{}), "{}");
  t.expect_eq(best::format("{:?}", bounds{.start = 5}), "{.start = 5}");
  t.expect_eq(best::format("{:?}", bounds{.start = 5, .end = 6}),
              "{.start = 5, .end = 6}");
  t.expect_eq(best::format("{:?}", bounds{.start = 5, .count = 6}),
              "{.start = 5, .count = 6}");
  t.expect_eq(best::format("{:?}", bounds{.start = 5, .including_end = 6}),
              "{.start = 5, .including_end = 6}");
  t.expect_eq(best::format("{:?}", bounds{.end = 6}), "{.end = 6}");
  t.expect_eq(best::format("{:?}", bounds{.count = 6}), "{.count = 6}");
  t.expect_eq(best::format("{:?}", bounds{.including_end = 6}),
              "{.including_end = 6}");
};

best::test Iter = [](auto& t) {
  best::bounds b = {.start = 5, .end = 11};
  t.expect_eq(best::vec(b.iter()), {5, 6, 7, 8, 9, 10});
  t.expect_eq(b.iter().count(), 6);
  t.expect_eq(b.iter().last(), 10);

  b = {.start = 5, .including_end = 11};
  t.expect_eq(best::vec(b.iter()), {5, 6, 7, 8, 9, 10, 11});
  t.expect_eq(b.iter().count(), 7);
  t.expect_eq(b.iter().last(), 11);

  b = {
      .start = best::max_of<size_t> - 1,
      .including_end = best::max_of<size_t>,
  };
  t.expect_eq(best::vec(b.iter()),
              {best::max_of<size_t> - 1, best::max_of<size_t>});
  t.expect_eq(b.iter().count(), 2);
  t.expect_eq(b.iter().last(), best::max_of<size_t>);

  best::int_range i = {.start = uint8_t(0)};
  t.expect_eq(best::vec(i.iter()),
              best::vec(best::bounds{.count = 256}.iter()));
  t.expect_eq(i.iter().count(), 256);
  t.expect_eq(i.iter().last(), 255);
};

// TODO: Once we have death tests, test the check-fail messages.
}  // namespace best::bounds_test
