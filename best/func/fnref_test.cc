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

#include "best/func/fnref.h"

#include "best/test/test.h"

namespace best::fnref_internal {
int add(int x) { return x + 42; }

best::test FromFnptr = [](auto& t) {
  best::fnref<int(int) const> f = add;
  t.expect_eq(f(8), 50);

  f = nullptr;
  t.expect_eq(f, nullptr);
};

best::test FromLambda = [](auto& t) {
  int total = 0;
  best::fnref<int(int) const> f = [&](int x) { return total += x; };

  t.expect_eq(f(5), 5);
  t.expect_eq(total, 5);

  auto mut = [y = 0](int x) mutable { return y += x; };
  best::fnref<int(int)> g = mut;
  t.expect_eq(g(5), 5);
  t.expect_eq(g(5), 10);

  static_assert(!requires { best::fnref<int(int) const>(mut); });
};
}  // namespace best::fnref_internal
