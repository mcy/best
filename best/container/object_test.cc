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

#include "best/container/object.h"

#include "best/test/test.h"

namespace best::object_test {
static_assert(best::is_empty<best::object<void>>);
static_assert(best::is_empty<best::object<best::empty>>);

best::test Smoke = [](auto& t) {
  best::object<int> x0(best::in_place, 42);
  t.expect_eq(*x0, 42);
  x0 = 43;
  t.expect_eq(*x0, 43);

  best::object<int&> x1(best::in_place, *x0);
  t.expect_eq(&*x1, &*x0);
  t.expect_eq(*x1, 43);
  int y = 57;
  x1 = y;
  t.expect_eq(&*x1, &y);
  t.expect_eq(*x1, 57);

  best::object<void> x3(best::in_place, 42);
  best::object<void> x4(best::in_place);
  x3 = x4;
  x3 = nullptr;  // Anything can be assigned to a void object, it's
                 // like std::ignore!
};

best::test ToString = [](auto& t) {
  best::object<int> x0(best::in_place, 42);
  best::object<bool> x1(best::in_place, true);
  best::object<void> x2(best::in_place);

  t.expect_eq(best::format("{:?}", x0), "42");
  t.expect_eq(best::format("{:?}", x1), "true");
  t.expect_eq(best::format("{:?}", x2), "void");
};
}  // namespace best::object_test
