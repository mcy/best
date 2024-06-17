#include "best/text/format.h"

#include "best/test/test.h"

namespace best::format_test {
best::test Smoke = [](auto& t) {
  t.expect_eq(best::format("hello, {}!", "world"), "hello, world!");
};
}  // namespace best::format_test