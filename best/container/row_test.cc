#include "best/container/row.h"

#include <type_traits>

#include "best/test/test.h"

namespace best::row_test {
static_assert(std::is_empty_v<best::row<>>);

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
}  // namespace best::row_test