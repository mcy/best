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

#include "best/text/rune.h"

#include "best/test/test.h"

namespace best::rune_test {
best::test FromInt = [](auto& t) {
  t.expect_eq(best::rune::from_int(0), 0);
  t.expect_eq(best::rune::from_int('a'), 'a');
  t.expect_eq(best::rune::from_int('\x7f'), '\x7f');
  t.expect_eq(best::rune::from_int(u'µ'), u'µ');
  t.expect_eq(best::rune::from_int(u'猫'), u'猫');
  t.expect_eq(best::rune::from_int(U'🧶'), U'🧶');
  t.expect_eq(best::rune::from_int(0x10ffff), 0x10ffff);

  t.expect_eq(best::rune::from_int(0xd800), best::none);
  t.expect_eq(best::rune::from_int(0xdbff), best::none);
  t.expect_eq(best::rune::from_int(0xdc00), best::none);
  t.expect_eq(best::rune::from_int(0xdfff), best::none);
  t.expect_eq(best::rune::from_int(0x110000), best::none);
  t.expect_eq(best::rune::from_int(-1), best::none);
};

best::test FromIntAllowSurrogates = [](auto& t) {
  t.expect_eq(best::rune::from_int_allow_surrogates(0), 0);
  t.expect_eq(best::rune::from_int_allow_surrogates('a'), 'a');
  t.expect_eq(best::rune::from_int_allow_surrogates('\x7f'), '\x7f');
  t.expect_eq(best::rune::from_int_allow_surrogates(u'µ'), u'µ');
  t.expect_eq(best::rune::from_int_allow_surrogates(u'猫'), u'猫');
  t.expect_eq(best::rune::from_int_allow_surrogates(U'🧶'), U'🧶');
  t.expect_eq(best::rune::from_int_allow_surrogates(0x10ffff), 0x10ffff);

  t.expect_eq(best::rune::from_int_allow_surrogates(0xd800), 0xd800);
  t.expect_eq(best::rune::from_int_allow_surrogates(0xdbff), 0xdbff);
  t.expect_eq(best::rune::from_int_allow_surrogates(0xdc00), 0xdc00);
  t.expect_eq(best::rune::from_int_allow_surrogates(0xdfff), 0xdfff);
  t.expect_eq(best::rune::from_int_allow_surrogates(0x110000), best::none);
  t.expect_eq(best::rune::from_int_allow_surrogates(-1), best::none);
};
}  // namespace best::rune_test
