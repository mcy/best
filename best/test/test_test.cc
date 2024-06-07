#include "best/test/test.h"

best::test Trivial = [](auto& t) { t.expect_eq(2 + 2, 4); };