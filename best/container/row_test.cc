#include "best/container/row.h"

#include "best/test/test.h"

namespace best::row_test {
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

  static_assert(best::same<decltype(std::move(x0).forward()),
                           best::row_forward<int&&, const float&, bool&&>>);
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

best::test refs = [](auto& t) {
  int x = 0;
  const int y = 2;
  best::row x0(best::bind, x, y);

  static_assert(best::same<decltype(x0), best::row<int&, const int&>>);
};
}  // namespace best::row_test