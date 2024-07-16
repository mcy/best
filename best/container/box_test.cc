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
  best::box<int[]> x0({1, 2, 3, 4, 5});
  t.expect_eq(x0.as_span(), best::span{1, 2, 3, 4, 5});

  best::option<best::box<int[]>> x1;
  t.expect_eq(x1, best::none);
  x1 = x0;
  t.expect_eq(x1->as_span(), best::span{1, 2, 3, 4, 5});
  t.expect_eq(x1->as_span(), x0.as_span());

  x0 = best::box<int[]>();
  t.expect_eq(x0.size(), 0);
};

struct Iface {
  Iface() = default;
  Iface(const Iface&) = default;
  Iface& operator=(const Iface&) = default;

  virtual best::str get() = 0;
};

class Impl final : public Iface {
 public:
  explicit Impl(best::strbuf buf) : value_(BEST_MOVE(buf)) {}
  Impl(const Impl&) = default;
  Impl& operator=(const Impl&) = default;

  best::str get() override { return value_; }

 private:
  best::strbuf value_;
};

best::test Virt = [](auto& t) {
  best::box<Impl> value(Impl{"hello hello hello hello hello"});
  t.expect_eq(value->get(), "hello hello hello hello hello");

  best::vbox<Iface> virt(BEST_MOVE(value));
  t.expect_eq(virt->get(), "hello hello hello hello hello");

  best::vbox<Iface> virt2 = *virt.copy();
  t.expect_eq(virt->get(), "hello hello hello hello hello");
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

  best::box<Bubble[]> x3({{}, {}, {}});
  auto x4 = x3;
  x4 = x3;
  auto x5 = std::move(x3);
  x4 = best::box<Bubble[]>({{}});
};
}  // namespace best::box_test
