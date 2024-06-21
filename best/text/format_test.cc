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

#include "best/text/format.h"

#include "best/test/test.h"

namespace best::format_test {
best::test Smoke = [](auto& t) {
  t.expect_eq(best::format("hello, {}!", "world"), "hello, world!");
  t.expect_eq(best::format("hello, {:?}!", "world"), "hello, \"world\"!");
};

best::test Ints = [](auto& t) {
  t.expect_eq(
      best::format(
          "{0} {0:?} {0:b} {0:o} {0:x} {0:X} {0:#b} {0:#o} {0:#x} {0:#X}", 42),
      "42 42 101010 52 2a 2A 0b101010 052 0x2a 0x2A");

  t.expect_eq(best::format("{0:x<5} {0:x^5} {0:x>5}", 42), "42xxx x42xx xxx42");
  t.expect_eq(best::format("{:#010x}", 55), "0x00000037");
};
}  // namespace best::format_test
