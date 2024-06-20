#include "best/base/ord.h"

#include "best/test/test.h"

namespace best::ord_test {
best::test Eq = [](auto& t) {
  t.expect(best::equal(1, 1));
  t.expect(!best::equal(1, 2));

  int a = 0;
  float b = 0;
  int* x = &a;
  float* y = &b;
  float* z = reinterpret_cast<float*>(x);
  t.expect(best::equal(x, z));
  t.expect(!best::equal(x, y));

  t.expect(!best::equal(1, x));
};

best::test Chain = [](auto& t) {
  t.expect_eq(
      best::ord::equal->*best::or_cmp([] { return best::ord::greater; }),
      best::ord::greater);
  t.expect_eq(best::ord::less->*best::or_cmp([] { return best::ord::greater; }),
              best::ord::less);
};

static_assert(same<common_ord<>, ord>);
static_assert(same<common_ord<ord>, ord>);
static_assert(same<common_ord<ord, decltype(Less)>, ord>);
static_assert(same<common_ord<ord, decltype(Unordered)>, partial_ord>);
static_assert(same<common_ord<ord, ord>, ord>);
static_assert(same<common_ord<ord, ord, ord>, ord>);
static_assert(same<common_ord<partial_ord>, partial_ord>);
static_assert(same<common_ord<partial_ord, decltype(Less)>, partial_ord>);
static_assert(same<common_ord<ord, partial_ord>, partial_ord>);
static_assert(same<common_ord<ord, partial_ord, ord>, partial_ord>);
}  // namespace best::ord_test