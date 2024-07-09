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

#include "best/container/span.h"

#include "best/container/span_sort.h"
#include "best/container/vec.h"
#include "best/test/test.h"

namespace best::span_test {
static_assert(best::is_span<best::span<int>>);
static_assert(best::is_span<best::span<int, 4>>);
static_assert(best::contiguous<best::span<int>>);
static_assert(best::contiguous<best::span<int, 4>>);

static_assert(best::static_size<best::span<int>>.is_empty());
static_assert(best::static_size<best::span<int, 4>>.has_value());

best::test Empty = [](auto& t) {
  best::span<int> empty;
  best::span<int, 0> zero;

  t.expect(empty.is_empty());
  t.expect(zero.is_empty());
  t.expect_eq(empty.size(), 0);
  t.expect_eq(zero.size(), 0);

  t.expect_eq(empty[{.start = 0}], best::span<int>{});
  t.expect_eq(empty[{.end = 0}], best::span<int>{});
  t.expect_eq(empty[{.count = 0}], best::span<int>{});
};

best::test Convert = [](auto& t) {
  int arr[] = {1, 2, 3};
  auto dfixed = best::from_static(arr);
  static_assert(best::same<decltype(dfixed), best::span<int, 3>>);
  t.expect(!dfixed.is_empty());
  t.expect_eq(dfixed.size(), 3);
  t.expect_eq(dfixed[0], 1);
  t.expect_eq(dfixed[1], 2);
  t.expect_eq(dfixed[2], 3);

  best::span<const int, 3> bfixed = dfixed;
  t.expect_eq(bfixed[0], 1);
  t.expect_eq(bfixed[1], 2);
  t.expect_eq(bfixed[2], 3);

  best::span<int> ddyn = dfixed;
  t.expect_eq(ddyn[0], 1);
  t.expect_eq(ddyn[1], 2);
  t.expect_eq(ddyn[2], 3);

  best::span<const int> bdyn = ddyn;
  t.expect_eq(bdyn[0], 1);
  t.expect_eq(bdyn[1], 2);
  t.expect_eq(bdyn[2], 3);

  best::span<const int, 3> bfixed2{ddyn};
  t.expect_eq(bfixed[0], 1);
  t.expect_eq(bfixed[1], 2);
  t.expect_eq(bfixed[2], 3);
};

best::test InitList = [](auto& t) {
  auto cb = [&](best::span<const int> x) { t.expect_eq(x[2], 3); };
  cb({1, 2, 3});
};

best::test FromNul = [](auto& t) {
  int ints[] = {1, 2, 3, 0, 4, 5, 6, 0};
  auto span = best::span<int>::from_nul(ints);
  t.expect_eq(span, best::span{1, 2, 3});
  t.expect_eq(best::span<int>::from_nul(nullptr), best::span<int>{});
};

best::test ToString = [](auto& t) {
  t.expect_eq(best::format("{:?}", best::span<int>()), "[]");
  t.expect_eq(best::format("{:?}", best::span{1}), "[1]");
  t.expect_eq(best::format("{:?}", best::span{1, 2}), "[1, 2]");
};

best::test Ordering = [](auto& t) {
  int32_t ints[] = {1, 2, 3};
  int64_t longs[] = {1, 2, 3};
  float floats[] = {1, 2, 3};

  best::span i = ints;
  best::span l = longs;
  best::span f = floats;

  t.expect_eq(i, l);
  t.expect_eq(i, ints);
  t.expect_eq(i, best::span{1, 2, 3});
  t.expect_eq(i, floats);
  t.expect_eq(i, f);

  t.expect(!(i < i));
  t.expect(!(i > i));
  t.expect_le(i, i);
  t.expect_ge(i, i);

  t.expect(!(i < l));
  t.expect(!(i > l));
  t.expect_le(i, l);
  t.expect_ge(i, l);

  int32_t less[] = {1, 0, 0, 0};
  int32_t prefix[] = {1, 2};
  int32_t one_more[] = {1, 2, 3, 4};
  int32_t greater[] = {1, 3, 1};

  t.expect_gt(i, less);
  t.expect_gt(i, prefix);
  t.expect_lt(i, one_more);
  t.expect_lt(i, greater);

  t.expect_gt(l, less);
  t.expect_gt(l, prefix);
  t.expect_lt(l, one_more);
  t.expect_lt(l, greater);

  t.expect_gt(f, less);
  t.expect_gt(f, prefix);
  t.expect_lt(f, one_more);
  t.expect_lt(f, greater);
};

best::test Indexing = [](auto& t) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  best::span sp = best::from_static(arr);

  auto tests = [&](auto sp) {
    t.expect_eq(sp.size(), 10);
    t.expect_eq(sp[3], 4);
    t.expect_eq(sp[best::index<3>], 4);

    t.expect_eq(sp.at(3), 4);
    t.expect_eq(sp.at(3), best::option(4));
    t.expect_eq(sp.at(11), best::none);

    t.expect_eq(sp[{.start = 5, .end = 7}], best::span{6, 7});
    t.expect_eq(sp[{.start = 5, .end = 5}], best::span<int>{});
    t.expect_eq(sp[{.start = 5, .count = 2}], best::span{6, 7});
    t.expect_eq(sp[{.start = 5, .count = 0}], best::span<int>{});
    t.expect_eq(sp[{.start = 5}], best::span{6, 7, 8, 9, 10});
    t.expect_eq(sp[{.end = 7}], best::span{1, 2, 3, 4, 5, 6, 7});
    t.expect_eq(sp[{.end = 0}], best::span<int>{});
    t.expect_eq(sp[{.start = 10}], best::span<int>{});
    t.expect_eq(sp[{.start = 5, .including_end = 7}], best::span{6, 7, 8});
    t.expect_eq(sp[{.start = 5, .including_end = 5}], best::span{6});
    t.expect_eq(sp[{.including_end = 7}], best::span{1, 2, 3, 4, 5, 6, 7, 8});

    t.expect_eq(sp.at({.start = 5, .end = 7}), best::span{6, 7});
    t.expect_eq(sp.at({.start = 5, .end = 5}), best::span<int>{});
    t.expect_eq(sp.at({.start = 5, .count = 2}), best::span{6, 7});
    t.expect_eq(sp.at({.start = 5, .count = 0}), best::span<int>{});
    t.expect_eq(sp.at({.start = 5}), best::span{6, 7, 8, 9, 10});
    t.expect_eq(sp.at({.end = 7}), best::span{1, 2, 3, 4, 5, 6, 7});
    t.expect_eq(sp.at({.end = 0}), best::span<int>{});
    t.expect_eq(sp.at({.start = 10}), best::span<int>{});
    t.expect_eq(sp.at({.start = 5, .including_end = 7}), best::span{6, 7, 8});
    t.expect_eq(sp.at({.start = 5, .including_end = 5}), best::span{6});
    t.expect_eq(sp.at({.including_end = 7}),
                best::span{1, 2, 3, 4, 5, 6, 7, 8});

    t.expect_eq(sp.at({.start = 11}), best::none);
    t.expect_eq(sp.at({.start = 9, .count = 2}), best::none);
    t.expect_eq(sp.at({.end = 11}), best::none);
    t.expect_eq(sp.at({.start = 3, .end = 1}), best::none);
  };

  tests(sp);
  tests(span<int>(sp));

  auto sp2 = sp[best::vals<bounds{.start = 8}>];
  static_assert(best::same<decltype(sp2), best::span<int, 2>>);
  t.expect_eq(sp2, best::span{9, 10});
};

best::test FrontAndBack = [](auto& t) {
  best::span<int> empty;

  t.expect_eq(empty.first(), best::none);
  t.expect_eq(empty.last(), best::none);
  t.expect_eq(empty.first<0>(), empty);
  t.expect_eq(empty.last<0>(), empty);
  t.expect_eq(empty.split_first(), best::none);
  t.expect_eq(empty.split_last(), best::none);
  t.expect_eq(empty.split_first<0>(), best::row{empty, empty});
  t.expect_eq(empty.split_last<0>(), best::row{empty, empty});

  best::vec<int> ints = {1, 2, 3, 4, 5};
  best::span sp = ints;

  t.expect_eq(sp.first(), 1);
  t.expect_eq(sp.last(), 5);
  t.expect_eq(sp.first<2>(), best::span{1, 2});
  t.expect_eq(sp.last<2>(), best::span{4, 5});
  t.expect_eq(sp.split_first(), best::row{1, sp[{.start = 1}]});
  t.expect_eq(sp.split_last(), best::row{5, sp[{.end = 4}]});
  t.expect_eq(sp.split_first<2>(),
              best::row{best::span{1, 2}, sp[{.start = 2}]});
  t.expect_eq(sp.split_last<2>(), best::row{best::span{4, 5}, sp[{.end = 3}]});

  int ints2[] = {1, 2, 3, 4, 5};
  best::span sp2 = ints2;

  t.expect_eq(sp2.first(), 1);
  t.expect_eq(sp2.last(), 5);
  t.expect_eq(sp2.first<2>(), best::span{1, 2});
  t.expect_eq(sp2.last<2>(), best::span{4, 5});
  t.expect_eq(sp2.split_first(), best::row{1, sp[{.start = 1}]});
  t.expect_eq(sp2.split_last(), best::row{5, sp[{.end = 4}]});
  t.expect_eq(sp2.split_first<2>(),
              best::row{best::span{1, 2}, sp[{.start = 2}]});
  t.expect_eq(sp2.split_last<2>(), best::row{best::span{4, 5}, sp[{.end = 3}]});
};

best::test Swap = [](auto& t) {
  best::vec<int> ints = {1, 2, 3, 4, 5};
  ints.as_span().swap(1, 2);
  t.expect_eq(ints, best::span{1, 3, 2, 4, 5});

  ints.as_span().reverse();
  t.expect_eq(ints, best::span{5, 4, 2, 3, 1});
};

struct SlowCmp {
  int x;
  SlowCmp(int x) : x(x) {}
  bool operator==(const SlowCmp& that) const {
    return best::black_box(x) == that.x;
  }
};

best::test Find = [](auto& t) {
  best::span bytes = "1234444456789";

  t.expect_eq(bytes.find('1'), 0);
  t.expect_eq(bytes.find('4'), 3);
  t.expect_eq(bytes.find('9'), 12);
  t.expect_eq(bytes.find('\0'), 13);
  t.expect_eq(bytes.find('a'), best::none);
  t.expect_eq(bytes.find(best::span<char>{}), 0);
  t.expect_eq(bytes.find({'3', '4', '4'}), 2);

  best::vec<best::span<const char>> byte_split(bytes.split('4'));
  t.expect_eq(
      byte_split,
      {{'1', '2', '3'}, {}, {}, {}, {}, {'5', '6', '7', '8', '9', '\0'}});

  int a[] = {1, 2, 3, 4, 4, 4, 4, 4, 5, 6, 7, 8, 9, 0};
  best::span ints = a;

  t.expect_eq(ints.find(1), 0);
  t.expect_eq(ints.find(4), 3);
  t.expect_eq(ints.find(9), 12);
  t.expect_eq(ints.find(0), 13);  // This exercises the "unstrided false
                                  // positive" case in best::search_bytes.
  t.expect_eq(ints.find(50), best::none);
  t.expect_eq(ints.find(best::span<int>{}), 0);
  t.expect_eq(ints.find({3, 4, 4}), 2);

  best::vec<best::span<const int>> int_split(ints.split(4));
  t.expect_eq(int_split, {{1, 2, 3}, {}, {}, {}, {}, {5, 6, 7, 8, 9, 0}});

  SlowCmp b[] = {1, 2, 3, 4, 4, 4, 4, 4, 5, 6, 7, 8, 9, 0};
  best::span slow = b;

  t.expect_eq(slow.find(1), 0);
  t.expect_eq(slow.find(4), 3);
  t.expect_eq(slow.find(9), 12);
  t.expect_eq(slow.find(0), 13);  // This exercises the "unstrided false
                                  // positive" case in best::search_bytes.
  t.expect_eq(slow.find(50), best::none);
  t.expect_eq(slow.find(best::span<int>{}), 0);
  t.expect_eq(slow.find({3, 4, 4}), 2);

  best::vec<best::span<const SlowCmp>> slow_split(slow.split(4));
  t.expect_eq(slow_split, {{1, 2, 3}, {}, {}, {}, {}, {5, 6, 7, 8, 9, 0}});
};

best::test Affixes = [](auto& t) {
  best::vec<int> ints = {1, 2, 3, 4, 5};
  best::span sp = ints;

  t.expect(sp.starts_with({}));
  t.expect(sp.starts_with({1, 2, 3}));
  t.expect(!sp.starts_with({4, 5}));

  t.expect(sp.ends_with({}));
  t.expect(sp.ends_with({4, 5}));
  t.expect(!sp.ends_with({1, 2}));

  t.expect_eq(sp.strip_prefix({}), sp);
  t.expect_eq(sp.strip_prefix({1, 2, 3}), best::span{4, 5});
  t.expect_eq(sp.strip_prefix({2, 3}), best::none);

  t.expect_eq(sp.strip_suffix({}), sp);
  t.expect_eq(sp.strip_suffix({4, 5}), best::span{1, 2, 3});
  t.expect_eq(sp.strip_suffix({2, 3}), best::none);

  t.expect(sp.consume_prefix({1}));
  t.expect(!sp.consume_prefix({1}));
  t.expect_eq(sp, best::span{2, 3, 4, 5});

  t.expect(sp.consume_suffix({5}));
  t.expect(!sp.consume_suffix({5}));
  t.expect_eq(sp, best::span{2, 3, 4});
};

best::test Sort = [](auto& t) {
  best::mark_sort_header_used();

  best::vec<int> ints = {5, 4, 3, 2, 1};
  ints.as_span().sort();
  t.expect_eq(ints, best::span{1, 2, 3, 4, 5});

  ints.as_span().stable_sort([](int x) { return best::count_ones(x); });
  t.expect_eq(ints, best::span{1, 2, 4, 3, 5});

  ints.as_span().sort([](int x, int y) { return y <=> x; });
  t.expect_eq(ints, best::span{5, 4, 3, 2, 1});
};

best::test Bisect = [](auto& t) {
  int ints[] = {1, 2, 3, 4, 100, 200};
  t.expect_eq(best::span(ints).bisect(3), best::ok(2));
  t.expect_eq(best::span(ints).bisect(100), best::ok(4));
  t.expect_eq(best::span(ints).bisect(55), best::err(4));
  t.expect_eq(best::span(ints).bisect(0), best::err(0));
  t.expect_eq(best::span(ints).bisect(1000), best::err(6));

  struct entry {
    best::str k;
    size_t v;
  };
  entry strs[] = {{"x", 1}, {"y", 2}, {"z", 3}, {"s1", 4}, {"s2", 5}};
  best::span(strs).sort(&entry::k);
  t.expect_eq(strs[*best::span(strs).bisect("x", &entry::k)].v, 1);
};

struct NonPod {
  int x;
  NonPod(int x) : x(x) {}
  NonPod(const NonPod&) = default;
  NonPod& operator=(const NonPod&) = default;
  NonPod(NonPod&&) = default;
  NonPod& operator=(NonPod&&) = default;
  ~NonPod() {}

  bool operator==(int y) const { return x == y; }
  friend auto& operator<<(auto& os, NonPod np) { return os << np.x; }
};
static_assert(!best::relocatable<NonPod, best::trivially>);

best::test Shift = [](auto& t) {
  unsafe u("test");

  int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
  best::span ints = a;

  int d = 0xcdcdcdcd;

  ints.shift_within(u, 5, 1, 2);
  t.expect_eq(best::black_box(ints), best::span{1, d, d, 4, 5, 2, 3, 8});
  ints[1] = 2;
  ints[2] = 3;

  ints.shift_within(u, 1, 6, 2);
  t.expect_eq(best::black_box(ints), best::span{1, 3, 8, 4, 5, 2, d, d});
  ints[6] = 3;
  ints[7] = 8;

  ints.shift_within(u, 1, 3, 4);
  t.expect_eq(best::black_box(ints), best::span{1, 4, 5, 2, 3, d, d, 8});

  ints.shift_within(u, 3, 1, 4);
  t.expect_eq(best::black_box(ints), best::span{1, d, d, 4, 5, 2, 3, 8});

  NonPod a2[] = {1, 2, 3, 4, 5, 6, 7, 8};
  best::span nps = a2;

  nps.shift_within(u, 5, 1, 2);
  t.expect_eq(best::black_box(nps), best::span{1, d, d, 4, 5, 2, 3, 8});
  nps[1] = 2;
  nps[2] = 3;

  nps.shift_within(u, 1, 6, 2);
  t.expect_eq(best::black_box(nps), best::span{1, 3, 8, 4, 5, 2, d, d});
  nps[6] = 3;
  nps[7] = 8;

  nps.shift_within(u, 1, 3, 4);
  t.expect_eq(best::black_box(nps), best::span{1, 4, 5, 2, 3, d, d, 8});

  nps.shift_within(u, 3, 1, 4);
  t.expect_eq(best::black_box(nps), best::span{1, d, d, 4, 5, 2, 3, 8});
};

best::test IsSubarray = [](auto& t) {
  int x[] = {1, 2, 3};
  int y[] = {1, 2};

  t.expect(best::span(x).has_subarray(x));
  t.expect(best::span(x).has_subarray(best::span(x)[{.start = 1}]));
  t.expect(best::span(x).has_subarray(best::span(x)[{.end = 1}]));
  t.expect(best::span(x).has_subarray(best::span(x)[{.start = 1, .end = 2}]));

  t.expect(!best::span(x).has_subarray(y));
};
}  // namespace best::span_test
