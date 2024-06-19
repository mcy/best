#include "best/container/object.h"

#include "best/test/test.h"

namespace best::object_test {
best::test Smoke = [](auto& t) {
  best::object<int> x0(best::in_place, 42);
  t.expect_eq(*x0, 42);
  x0 = 43;
  t.expect_eq(*x0, 43);

  best::object<int&> x1(best::in_place, *x0);
  t.expect_eq(&*x1, &*x0);
  t.expect_eq(*x1, 43);
  int y = 57;
  x1 = y;
  t.expect_eq(&*x1, &y);
  t.expect_eq(*x1, 57);

  best::object<void> x3(best::in_place, 42);
  best::object<void> x4(best::in_place);
  x3 = x4;
  x3 = nullptr;  // Anything can be assigned to a void object, it's
                 // like std::ignore!
};

best::test ToString = [](auto& t) {
  best::object<int> x0(best::in_place, 42);
  best::object<bool> x1(best::in_place, true);
  best::object<void> x2(best::in_place);

  t.expect_eq(best::format("{:?}", x0), "42");
  t.expect_eq(best::format("{:?}", x1), "true");
  t.expect_eq(best::format("{:?}", x2), "void");
};
}  // namespace best::object_test