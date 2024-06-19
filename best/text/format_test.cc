#include "best/text/format.h"

#include "best/test/test.h"

namespace best::format_test {
best::test Smoke = [](auto& t) {
  t.expect_eq(best::format("hello, {}!", "world"), "hello, world!");
  t.expect_eq(best::format("hello, {:?}!", "world"), "hello, \"world\"!");
};

best::test Ints = [](auto& t) {
  t.expect_eq(
      best::format(
          "{0} {0:?} {0:b} {0:o} {0:x} {0:X} {0:#b} {0:#o} {0:#x} {0:#X}", 42),
      "42 42 101010 52 2a 2A 0b101010 052 0x2a 0x2A");

  t.expect_eq(best::format("{0:x<5} {0:x^5} {0:x>5}", 42), "42xxx x42xx xxx42");
  t.expect_eq(best::format("{:#010x}", 55), "0x00000037");
};
}  // namespace best::format_test