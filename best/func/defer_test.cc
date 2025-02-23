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

#include "best/func/defer.h"

#include "best/test/test.h"

namespace best::tap_test {
best::test Defer = [](auto& t) {
  int x = 0;

  {
    best::defer d_ = [&] { x = 42; };
  }
  t.expect_eq(x, 42);

  {
    best::defer d_ = [&] { x = 0; };
    d_.cancel();
  }
  t.expect_eq(x, 42);

  {
    best::defer d_ = [&] { x *= 2; };
    d_.run();
    t.expect_eq(x, 84);
    d_.run();
    t.expect_eq(x, 84);
  }
  t.expect_eq(x, 84);
};
}  // namespace best::tap_test
