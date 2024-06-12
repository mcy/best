#include "best/container/pun.h"

#include <memory>

#include "best/meta/init.h"
#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::pun_test {
using ::best_fodder::NonTrivialDtor;
using ::best_fodder::NonTrivialPod;
using ::best_fodder::Relocatable;
using ::best_fodder::TrivialCopy;

// static_assert(std::is_trivial_v<best::pun<int, int>>);
// static_assert(std::is_trivially_copyable_v<best::pun<int, TrivialCopy>>);
static_assert(best::relocatable<best::pun<int, Relocatable>, best::trivially>);

static constexpr best::pun<int, TrivialCopy> VerifyLiteral(best::index<1>);

void mark_as_used() { (void)VerifyLiteral; }

best::test Default = [](auto& t) {
  /// No elements are accessible, so no assertions to make really.
  best::pun<int32_t, int64_t> x;
  auto y = x;
  (void)y;
};

best::test Copies = [](auto& t) {
  unsafe::in([&](auto u) {
    best::pun<int32_t, int64_t> x1(best::index<0>, 0xaaaaaaaa);
    auto x2 = x1;
    t.expect_eq(x2.get<0>(u), 0xaaaaaaaa);
    x2 = best::pun<int32_t, int64_t>(best::index<1>, 0xaaaaaaaa'55555555);
    t.expect_eq(x2.get<1>(u), 0xaaaaaaaa'55555555);
  });
};

best::test NoDtor = [](auto& t) {
  unsafe::in([&](auto u) {
    int target = 0;
    {
      best::pun<bool, NonTrivialDtor> x1(best::index<1>, &target, 42);
      std::destroy_at(&x1.get<1>(u));
      t.expect_eq(target, 42);
      target = 0;
    }
    t.expect_eq(target, 0, "destructor of best::pun variant ran unexpectedly");
  });
};

best::test NonTrivial = [](auto& t) {
  unsafe::in([&](auto u) {
    best::pun<bool, NonTrivialPod> x2(best::index<1>, 5, -2);
    t.expect_eq(x2.get<1>(u).x(), 5);
    t.expect_eq(x2.get<1>(u).y(), -2);
    auto x3 = x2.get<1>(u);
    t.expect_eq(x3.x(), 5);
    t.expect_eq(x3.y(), -2);
  });
};

best::test String = [](auto& t) {
  unsafe::in([&](auto u) {
    best::pun<best::str, int> s(best::index<0>, "hello...");
    t.expect_eq(s.get<0>(u), "hello...");
  });
};

constexpr best::pun<int, bool> check_constexpr() {
  return best::pun<int, bool>(best::index<1>, true);
}
static_assert(unsafe::in([](auto u) { return check_constexpr().get<1>(u); }));
}  // namespace best::pun_test