#include "best/log/location.h"

#include "best/test/test.h"

best::test Smoke = [](auto& t) {
  best::location loc = best::here;
  t.expect_eq(loc.file(), "best/log/location_test.cc");
  t.expect_eq(loc.line(), 6);
  t.expect_eq(best::format("{:?}", loc), "best/log/location_test.cc:6");

  // Can't easily test col() and func().
};