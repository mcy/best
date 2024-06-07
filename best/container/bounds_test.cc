#include "best/container/bounds.h"

#include "best/test/test.h"

namespace best::bounds_test {
best::test ComputeCount = [](auto& t) {
  auto count = [](bounds b, container_internal::option<size_t> max = {}) {
    return b.try_compute_count(max);
  };

  t.expect_eq(count({}), {});
  t.expect_eq(count({.start = 4}), {});
  t.expect_eq(count({.end = 4}), 4);
  t.expect_eq(count({.including_end = 4}), 5);
  t.expect_eq(count({.count = 4}), 4);
  t.expect_eq(count({.start = 4, .end = 4}), 0);
  t.expect_eq(count({.start = 4, .end = 5}), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}), 2);
  t.expect_eq(count({.start = 4, .count = 4}), 4);

  t.expect_eq(count({}, 10), 10);
  t.expect_eq(count({.start = 4}, 10), 6);
  t.expect_eq(count({.end = 4}, 10), 4);
  t.expect_eq(count({.including_end = 4}, 10), 5);
  t.expect_eq(count({.count = 4}, 10), 4);
  t.expect_eq(count({.start = 4, .end = 4}, 10), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 10), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}, 10), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}, 10), 2);
  t.expect_eq(count({.start = 2, .count = 2}, 10), 2);

  t.expect_eq(count({.start = 4}, 4), 0);
  t.expect_eq(count({.end = 4}, 4), 4);
  t.expect_eq(count({.including_end = 4}, 4), {});
  t.expect_eq(count({.count = 4}, 4), 4);
  t.expect_eq(count({.start = 4, .end = 4}, 5), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 5), 1);
  t.expect_eq(count({.start = 4, .including_end = 4}, 5), 1);
  t.expect_eq(count({.start = 4, .including_end = 5}, 5), {});
  t.expect_eq(count({.start = 2, .count = 2}, 4), 2);

  t.expect_eq(count({.start = 4}, 3), {});
  t.expect_eq(count({.end = 4}, 3), {});
  t.expect_eq(count({.including_end = 4}, 3), {});
  t.expect_eq(count({.count = 4}, 3), {});
  t.expect_eq(count({.start = 4, .end = 4}, 4), 0);
  t.expect_eq(count({.start = 4, .end = 5}, 4), {});
  t.expect_eq(count({.start = 4, .including_end = 4}, 4), {});
  t.expect_eq(count({.start = 4, .including_end = 5}, 4), {});
  t.expect_eq(count({.start = 2, .count = 2}, 3), {});
};

// TODO: Once we have death tests, test the check-fail messages.
}  // namespace best::bounds_test