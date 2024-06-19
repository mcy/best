#include "best/math/conv.h"

#include "best/test/test.h"

namespace best::conv_test {
best::test Decimal = [](auto& t) {
  t.expect_eq(best::atoi<int>("0"), 0);
  t.expect_eq(best::atoi<int>("000"), 0);
  t.expect_eq(best::atoi<int>("-0"), 0);
  t.expect_eq(best::atoi<int>("+5"), 5);
  t.expect_eq(best::atoi<int>("123456789"), 123456789);
  t.expect_eq(best::atoi<int>("-123456789"), -123456789);
  t.expect_eq(best::atoi<int>("2147483647"), 2147483647);
  t.expect_eq(best::atoi<int>("-2147483648"), -2147483648);

  t.expect_eq(best::atoi<int>("1234567a"), best::err());
  t.expect_eq(best::atoi<int>("2147483648"), best::err());
  t.expect_eq(best::atoi<int>("-2147483649"), best::err());
  t.expect_eq(best::atoi<int>("cow"), best::err());
};

best::test Bin = [](auto& t) {
  t.expect_eq(best::atoi<int>("0", 2), 0);
  t.expect_eq(best::atoi<int>("000", 2), 0);
  t.expect_eq(best::atoi<int>("-0", 2), 0);
  t.expect_eq(best::atoi<int>("+1", 2), 1);
  t.expect_eq(best::atoi<int>("11110", 2), 30);
  t.expect_eq(best::atoi<int>("01111111111111111111111111111111", 2),
              2147483647);
  t.expect_eq(best::atoi<int>("-10000000000000000000000000000000", 2),
              -2147483648);

  t.expect_eq(best::atoi<int>("2", 2), best::err());
  t.expect_eq(best::atoi<int>("10000000000000000000000000000000", 2),
              best::err());
  t.expect_eq(best::atoi<int>("-10000000000000000000000000000001", 2),
              best::err());
};

best::test Hex = [](auto& t) {
  t.expect_eq(best::atoi<int>("0", 16), 0);
  t.expect_eq(best::atoi<int>("000", 16), 0);
  t.expect_eq(best::atoi<int>("-0", 16), 0);
  t.expect_eq(best::atoi<int>("+beef", 16), 0xbeef);
  t.expect_eq(best::atoi<unsigned>("12345678", 16), 0x12345678);
  t.expect_eq(best::atoi<unsigned>("9abcdefA", 16), 0x9abcdefa);
  t.expect_eq(best::atoi<unsigned>("BCDEF000", 16), 0xBCDEF000);

  t.expect_eq(best::atoi<int>("7fffffff", 16), 2147483647);
  t.expect_eq(best::atoi<int>("-80000000", 16), -2147483648);

  t.expect_eq(best::atoi<int>("80000000", 16), best::err());
  t.expect_eq(best::atoi<int>("-80000001", 16), best::err());
  t.expect_eq(best::atoi<int>("cow"), best::err());
};
}  // namespace best::conv_test