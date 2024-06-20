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
}  // namespace best::row_test