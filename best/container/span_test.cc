#include "best/container/span.h"

#include "best/container/span_sort.h"
#include "best/container/vec.h"
#include "best/test/test.h"

namespace best::span_test {
static_assert(best::is_span<best::span<int>>);
static_assert(best::is_span<best::span<int, 4>>);
static_assert(best::contiguous<best::span<int>>);
static_assert(best::contiguous<best::span<int, 4>>);

static_assert(best::span<int>::extent.is_empty());
static_assert(best::span<int, 4>::extent.has_value());

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
  best::span dfixed = arr;
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
  best::span sp = arr;

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
  t.expect_eq(empty.split_first<0>(), std::pair{empty, empty});
  t.expect_eq(empty.split_last<0>(), std::pair{empty, empty});

  best::vec<int> ints = {1, 2, 3, 4, 5};
  best::span sp = ints;

  t.expect_eq(sp.first(), 1);
  t.expect_eq(sp.last(), 5);
  t.expect_eq(sp.first<2>(), best::span{1, 2});
  t.expect_eq(sp.last<2>(), best::span{4, 5});
  t.expect_eq(sp.split_first(), std::pair{1, sp[{.start = 1}]});
  t.expect_eq(sp.split_last(), std::pair{5, sp[{.end = 4}]});
  t.expect_eq(sp.split_first<2>(),
              std::pair{best::span{1, 2}, sp[{.start = 2}]});
  t.expect_eq(sp.split_last<2>(), std::pair{best::span{4, 5}, sp[{.end = 3}]});

  int ints2[] = {1, 2, 3, 4, 5};
  best::span sp2 = ints2;

  t.expect_eq(sp2.first(), 1);
  t.expect_eq(sp2.last(), 5);
  t.expect_eq(sp2.first<2>(), best::span{1, 2});
  t.expect_eq(sp2.last<2>(), best::span{4, 5});
  t.expect_eq(sp2.split_first(), std::pair{1, sp[{.start = 1}]});
  t.expect_eq(sp2.split_last(), std::pair{5, sp[{.end = 4}]});
  t.expect_eq(sp2.split_first<2>(),
              std::pair{best::span{1, 2}, sp[{.start = 2}]});
  t.expect_eq(sp2.split_last<2>(), std::pair{best::span{4, 5}, sp[{.end = 3}]});
};

best::test Swap = [](auto& t) {
  best::vec<int> ints = {1, 2, 3, 4, 5};
  ints.as_span().swap(1, 2);
  t.expect_eq(ints, best::span{1, 3, 2, 4, 5});

  ints.as_span().reverse();
  t.expect_eq(ints, best::span{5, 4, 2, 3, 1});
};

best::test Affixes = [](auto& t) {
  best::vec<int> ints = {1, 2, 3, 4, 5};
  best::span sp = ints;

  t.expect(sp.contains(4));
  t.expect(!sp.contains(-1));

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

best::test Shift = [](auto& t) {
  unsafe::in([&](auto u) {
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
    ints[5] = 2;
    ints[6] = 3;

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
    nps[5] = 2;
    nps[6] = 3;

    nps.shift_within(u, 3, 1, 4);
    t.expect_eq(best::black_box(nps), best::span{1, d, d, 4, 5, 2, 3, 8});
  });
};
}  // namespace best::span_test