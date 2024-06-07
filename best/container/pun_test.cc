#include "best/container/pun.h"

#include <string.h>

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
static constexpr best::pun<int, TrivialCopy> VerifyLiteral2(best::uninit);

void mark_as_used() {
  (void)VerifyLiteral;
  (void)VerifyLiteral2;
}

best::test Default = [](auto& t) {
  /// No elements are accessible, so no assertions to make really.
  best::pun<int32_t, int64_t> x;
  auto y = x;
  (void)y;
};

best::test Uninit = [](auto& t) {
  best::pun<int32_t, int64_t> x1(best::uninit);
  std::memset(&x1, 42, sizeof(x1));
  for (size_t i = 0; i < sizeof(x1); ++i) {
    t.expect_eq(x1.uninit()[i], 42);
  }

  best::pun<int32_t, int64_t> x2(best::index<0>, 0xaaaaaaaa);
  t.expect_eq(x2.get<0>(best::unsafe), 0xaaaaaaaa);
  for (size_t i = 0; i < 4; ++i) {
    t.expect_eq(x2.uninit()[i], 0xaa);
  }

  best::pun<int32_t, int64_t> x3(best::index<1>, 0xaaaaaaaa'55555555);
  t.expect_eq(x3.get<1>(best::unsafe), 0xaaaaaaaa'55555555);
  for (size_t i = 0; i < 8; ++i) {
    t.expect_eq(x3.uninit()[i], i < 4 ? 0x55 : 0xaa);
  }
  for (size_t i = 0; i < 4; ++i) {
    t.expect_eq(x3.padding<0>(best::unsafe)[i], 0xaa);
  }
};

best::test Copies = [](auto& t) {
  best::pun<int32_t, int64_t> x1(best::index<0>, 0xaaaaaaaa);
  auto x2 = x1;
  t.expect_eq(x2.get<0>(best::unsafe), 0xaaaaaaaa);
  x2 = best::pun<int32_t, int64_t>(best::index<1>, 0xaaaaaaaa'55555555);
  t.expect_eq(x2.get<1>(best::unsafe), 0xaaaaaaaa'55555555);
};

best::test NoDtor = [](auto& t) {
  int target = 0;
  {
    best::pun<bool, NonTrivialDtor> x1(best::index<1>, &target, 42);
    std::destroy_at(&x1.get<1>(best::unsafe));
    t.expect_eq(target, 42);
    target = 0;
  }
  t.expect_eq(target, 0, "destructor of best::pun variant ran unexpectedly");
};

best::test NonTrivial = [](auto& t) {
  best::pun<bool, NonTrivialPod> x2(best::index<1>, 5, -2);
  t.expect_eq(x2.get<1>(best::unsafe).x(), 5);
  t.expect_eq(x2.get<1>(best::unsafe).y(), -2);
  auto x3 = x2.get<1>(best::unsafe);
  t.expect_eq(x3.x(), 5);
  t.expect_eq(x3.y(), -2);
};

best::test String = [](auto& t) {
  best::pun<best::str, int> s(best::index<0>, "hello...");
  t.expect_eq(s.get<0>(best::unsafe), "hello...");
};

constexpr best::pun<int, bool> check_constexpr() {
  return best::pun<int, bool>(best::index<1>, true);
}
static_assert(check_constexpr().get<1>(best::unsafe));
}  // namespace best::pun_test