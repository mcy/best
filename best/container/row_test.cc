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

#include "best/container/row.h"

#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::row_test {
using ::best_fodder::MoveOnly;

static_assert(best::is_empty<best::row<>>);

best::test Nums = [](auto& t) {
  best::row<int, float, bool> x0(42, 1.5, true);
  t.expect_eq(x0[best::index<0>], 42);
  t.expect_eq(x0[best::index<1>], 1.5);
  t.expect_eq(x0[best::index<2>], true);

  auto [a, b, c] = x0;
  t.expect_eq(x0, best::row(a, b, c));
  t.expect_ne(x0, best::row(0, b, c));
  t.expect_ne(x0, best::row(a, 0, c));
  t.expect_ne(x0, best::row(a, b, 0));

  t.expect_eq(x0.apply([](auto... x) { return (0 + ... + x); }), 44.5);
};

best::test Fwd = [](auto& t) {
  float x;
  best::row<int, const float&, bool> x0{42, x, true};

  static_assert(best::same<decltype(std::move(x0).as_args()),
                           best::args<int&&, const float&, bool&&>>);
};

best::test ToString = [](auto& t) {
  best::row<> x0{};
  best::row<int> x1{1};
  best::row<int, int> x2{1, 2};
  best::row<int, void, int> x3{1, 2, 3};

  t.expect_eq(best::format("{:?}", x0), "()");
  t.expect_eq(best::format("{:?}", x1), "(1)");
  t.expect_eq(best::format("{:?}", x2), "(1, 2)");
  t.expect_eq(best::format("{:?}", x3), "(1, void, 3)");
};

best::test Each = [](auto& t) {
  best::row x0 = {1, 2};

  int sum = 0;
  x0.each([&](int x) { sum += x; });
  t.expect_eq(sum, 3);
};

struct Tag {
  using BestRowKey = Tag;
  bool operator==(const Tag&) const = default;
};
struct Tagged1 {
  using BestRowKey = Tag;
  bool operator==(const Tagged1&) const = default;
};

best::test Select = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};

  t.expect_eq(x0.select<int>(), best::row(1, 4));
  t.expect_eq(x0.select<int*>(), best::row(nullptr));
  t.expect_eq(x0.select<void*>(), best::row());

  best::row<int, Tagged1, Tag> x1{42, {}, {}};
  t.expect_eq(x1.select<Tagged1>(), best::row(Tagged1()));
  t.expect_eq(x1.select<Tag>(), best::row(Tagged1(), Tag()));
};

best::test Refs = [](auto& t) {
  int x = 0;
  const int y = 2;
  best::row x0(best::bind, x, y);

  static_assert(best::same<decltype(x0), best::row<int&, const int&>>);
};

best::test Join = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};
  t.expect_eq(x0 + x0, best::row(1, 2, nullptr, 4, 1, 2, nullptr, 4));

  best::row x1{MoveOnly()};
  best::row<MoveOnly> x2 = BEST_MOVE(x1) + best::row();
  x2 = best::row() + BEST_MOVE(x1);
};

best::test Slice = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};
  t.expect_eq(x0.at<bounds{.count = 2}>(), best::row{1, 2});
  t.expect_eq(x0.at<bounds{.start = 2}>(), best::row{nullptr, 4});
  t.expect_eq(x0.at<bounds{.start = 2, .count = 0}>(), best::row{});

  best::row x1{MoveOnly(), 42};
  BEST_MOVE(x1).at<bounds{.start = 0, .count = 1}>();
};

best::test Splice = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};
  best::row<int, long, int*, int, int*> x1 = x0.push(x0[best::index<2>]);
  t.expect_eq(x1, best::row{1, 2, nullptr, 4, nullptr});

  best::row<int, long, int*, int, int*&> x11 =
      x0.push(best::bind, x0[best::index<2>]);
  t.expect_eq(x11, best::row{1, 2, nullptr, 4, nullptr});

  best::row<int, long, int*, int, long*> x12 =
      x0.push(best::types<long*>, nullptr);
  t.expect_eq(x12, best::row{1, 2, nullptr, 4, nullptr});

  best::row<int, long, int*, int*, int> x2 = x0.insert<3>(x0[best::index<2>]);
  t.expect_eq(x2, best::row{1, 2, nullptr, nullptr, 4});

  best::row<int, long, int*, int*&, int> x21 =
      x0.insert<3>(best::bind, x0[best::index<2>]);
  t.expect_eq(x21, best::row{1, 2, nullptr, nullptr, 4});

  best::row<int, long, int*, long*, int> x22 =
      x0.insert<3>(best::types<long*>, nullptr);
  t.expect_eq(x22, best::row{1, 2, nullptr, nullptr, 4});

  best::row<int, long, int*, int*> x2o = x0.update<3>(x0[best::index<2>]);
  t.expect_eq(x2o, best::row{1, 2, nullptr, nullptr});

  best::row<int, long, int*, int*&> x21o =
      x0.update<3>(best::bind, x0[best::index<2>]);
  t.expect_eq(x21o, best::row{1, 2, nullptr, nullptr});

  best::row<int, long, int*, long*> x22o =
      x0.update<3>(best::types<long*>, nullptr);
  t.expect_eq(x22o, best::row{1, 2, nullptr, nullptr});

  best::row<int, int, long, int*, int, int> x3o =
      x0.splice<best::bounds{.start = 1, .end = 3}>(x0);
  t.expect_eq(x3o, best::row{1, 1, 2, nullptr, 4, 4});

  best::row<int, long, long, const int*, unsigned, int> x31 =
      x0.splice<best::bounds{.start = 1, .end = 3}>(
          best::types<long, long, const int*, unsigned>, x0);
  t.expect_eq(x31, best::row{1, 1, 2, nullptr, 4, 4});

  best::row x4{MoveOnly(), 42};
  BEST_MOVE(x4).push(5);
  BEST_MOVE(x4).insert<0>(5);
  BEST_MOVE(x4).splice<bounds{.start = 1}>(best::row{1, 2, 3});
  x4.update<0>(42);  // No need for move.
};

best::test Erase = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};

  best::row<int, long, int> x1 = x0.remove<2>();
  t.expect_eq(x1, best::row(1, 2, 4));

  best::row<int, int> x2 = x0.erase<best::bounds{.start = 1, .end = 3}>();
  t.expect_eq(x2, best::row(1, 4));

  best::row<> x3 = x0.erase<best::bounds{}>();
  t.expect_eq(x3, best::row());

  best::row x4{MoveOnly(), 42};
  BEST_MOVE(x4).remove<1>();
  BEST_MOVE(x4).erase<bounds{.start = 1}>();
  x4.remove<0>();  // No need for move.
};

best::test ScatterGather = [](auto& t) {
  best::row<int, long, int*, int> x0{1, 2, nullptr, 4};

  best::row<int, long> x1 = x0.gather<3, 1>();
  t.expect_eq(x1, best::row(4, 2));

  best::row<int, char, char, int> x2 = x0.scatter<2, 1>(best::row{'a', 'b'});
  t.expect_eq(x2, best::row(1, 'b', 'a', 4));
  best::row<int, long, char, int> x3 = x0.scatter<2>(best::row{'a', 'b'});
  t.expect_eq(x3, best::row(1, 2, 'a', 4));

  best::row x4{MoveOnly(), 42};
  BEST_MOVE(x4).gather<0>();
  BEST_MOVE(x4).scatter<1>(best::row{false});
  x4.scatter<0>(best::row{false});
};
}  // namespace best::row_test
