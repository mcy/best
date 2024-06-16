#include "best/container/bag.h"

#include <type_traits>

#include "best/test/test.h"

namespace best::bag_test {
static_assert(std::is_empty_v<best::bag<>>);

best::test Nums = [](auto& t) {
  best::bag<int, float, bool> x0(42, 1.5, true);
  t.expect_eq(x0[best::index<0>], 42);
  t.expect_eq(x0[best::index<1>], 1.5);
  t.expect_eq(x0[best::index<2>], true);

  auto [a, b, c] = x0;
  t.expect_eq(x0, best::bag(a, b, c));
  t.expect_ne(x0, best::bag(0, b, c));
  t.expect_ne(x0, best::bag(a, 0, c));
  t.expect_ne(x0, best::bag(a, b, 0));

  t.expect_eq(x0.apply([](auto... x) { return (0 + ... + x); }), 44.5);
};

}  // namespace best::bag_test