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

#include "best/log/location.h"

#include "best/test/test.h"

best::test Smoke = [](auto& t) {
// Explicitly set line info to make this test somewhat more stable.
#line 42 "best/log/location_test.cc"
  best::location loc = best::here;
  t.expect_eq(loc.file(), "best/log/location_test.cc");
  t.expect_eq(loc.line(), 42);
  t.expect_eq(best::format("{:?}", loc), "best/log/location_test.cc:42");

  // Can't easily test col() and func().
};
