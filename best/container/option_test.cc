#include "best/container/option.h"

#include "best/container/result.h"
#include "best/container/vec.h"
#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::option_test {
using ::best_fodder::LeakTest;

static_assert(sizeof(best::option<int>) == 2 * sizeof(int));
static_assert(sizeof(best::option<int*>) == 2 * sizeof(int*));
static_assert(sizeof(best::option<int&>) == sizeof(int*));

best::test Empty = [](auto& t) {
  best::option<void> x1;
  best::option<int> x2;
  best::option<int&> x3;

  t.expect(x1.is_empty());
  t.expect(x2.is_empty());
  t.expect(x3.is_empty());
  t.expect(!x1.has_value());
  t.expect(!x2.has_value());
  t.expect(!x3.has_value());

  best::option<void> y1 = best::none;
  best::option<int> y2 = best::none;
  best::option<int&> y3 = best::none;

  t.expect(y1.is_empty());
  t.expect(y2.is_empty());
  t.expect(y3.is_empty());
  t.expect(!y1.has_value());
  t.expect(!y2.has_value());
  t.expect(!y3.has_value());

  y1 = best::none;
  y2 = best::none;
  y3 = best::none;

  t.expect(y1.is_empty());
  t.expect(y2.is_empty());
  t.expect(y3.is_empty());
  t.expect(!y1.has_value());
  t.expect(!y2.has_value());
  t.expect(!y3.has_value());

  y1 = x1;
  y2 = x2;
  y3 = x3;

  t.expect(y1.is_empty());
  t.expect(y2.is_empty());
  t.expect(y3.is_empty());
  t.expect(!y1.has_value());
  t.expect(!y2.has_value());
  t.expect(!y3.has_value());
};

best::test Nonempty = [](auto& t) {
  best::option<void> x1 = best::empty{};
  best::option<int> x2 = 42;
  int a = 5;
  best::option<int&> x3 = a;

  t.expect(!x1.is_empty());
  t.expect(!x2.is_empty());
  t.expect(!x3.is_empty());
  t.expect(x1.has_value());
  t.expect(x2.has_value());
  t.expect(x3.has_value());

  t.expect_eq(x2, 42);
  t.expect_eq(x3, 5);

  auto z1 = x1;
  auto z2 = x2;
  auto z3 = x3;

  t.expect(!z1.is_empty());
  t.expect(!z2.is_empty());
  t.expect(!z3.is_empty());
  t.expect(z1.has_value());
  t.expect(z2.has_value());
  t.expect(z3.has_value());
  t.expect_eq(z2, 42);
  t.expect_eq(z3, 5);

  best::option<void> y1 = best::none;
  best::option<int> y2 = best::none;
  best::option<int&> y3 = best::none;

  t.expect(y1.is_empty());
  t.expect(y2.is_empty());
  t.expect(y3.is_empty());
  t.expect(!y1.has_value());
  t.expect(!y2.has_value());
  t.expect(!y3.has_value());

  y1 = z1;
  y2 = z2;
  y3 = z3;

  t.expect(!y1.is_empty());
  t.expect(!y2.is_empty());
  t.expect(!y3.is_empty());
  t.expect(y1.has_value());
  t.expect(y2.has_value());
  t.expect(y3.has_value());
  t.expect_eq(y2, 42);
  t.expect_eq(y3, 5);

  y1 = best::none;
  y2 = best::none;
  y3 = best::none;

  t.expect(y1.is_empty());
  t.expect(y2.is_empty());
  t.expect(y3.is_empty());
  t.expect(!y1.has_value());
  t.expect(!y2.has_value());
  t.expect(!y3.has_value());
};

best::test HasValue = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 0;
  best::option<int> x2 = 42;

  t.expect(!x0.has_value([](int x) { return x != 0; }));
  t.expect(!x1.has_value([](int x) { return x != 0; }));
  t.expect(x2.has_value([](int x) { return x != 0; }));

  best::option<void> x3;
  best::option<void> x4 = best::VoidOption;

  t.expect(!x3.has_value([] { return true; }));
  t.expect(x4.has_value([] { return true; }));
  t.expect(!x3.has_value([] { return false; }));
  t.expect(!x4.has_value([] { return false; }));
};

best::test ToString = [](auto& t) {
  best::option<int> x0;
  best::option<void> x1;
  best::option<int> x2 = 42;
  best::option<void> x3 = VoidOption;

  t.expect_eq(best::format("{:?}", x0), "none");
  t.expect_eq(best::format("{:?}", x1), "none");
  t.expect_eq(best::format("{:?}", x2), "option(42)");
  t.expect_eq(best::format("{:?}", x3), "option(void)");
};

best::test Converting = [](auto& t) {
  best::option<int32_t> x1 = 42;
  best::option<int64_t> x2 = x1;
  t.expect_eq(x2, 42);
  t.expect_eq(x2, x1);

  best::option<int32_t&> x3 = x1;
  t.expect_eq(x3, 42);
  t.expect_eq(x3, x2);
  t.expect_eq(x3, x1);
  t.expect_eq(x3.as_ptr(), x1.as_ptr());
};

best::test FromPointer = [](auto& t) {
  int a = 42;

  best::option<int32_t&> x0 = &a;
  t.expect_eq(x0, 42);
  t.expect_eq(x0.as_ptr(), &a);

  x0 = nullptr;
  t.expect(x0.is_empty());
  t.expect_eq(x0.as_ptr(), nullptr);
};

best::test Leaky = [](auto& t) {
  LeakTest l_(t);

  using Bubble = LeakTest::Bubble;

  best::option<Bubble> x0;
  x0 = best::option(Bubble());
  x0.reset();
  x0.emplace();

  auto x1 = x0;
  auto x2 = std::move(x0);
  auto x3 = x0;

  x2 = x1;
  x2 = std::move(x1);

  x0.reset();
  x0 = x2;
  x0.reset();
  x2 = x0;

  x0 = best::option<Bubble>(best::in_place);
  x2 = *x0;
  x0.emplace();
  *x2 = *x0;
};

best::test ValueOr = [](auto& t) {
  best::option<int> x0 = 42;
  x0.reset();
  t.expect_eq(x0.value_or(), 0);
  t.expect_eq(x0.value_or(42), 42);
};

best::test Refs = [](auto& t) {
  best::option<int> x0 = 42;
  best::option<int&> x1 = x0;
  x1 = x0.as_ref();
  best::option<int&&> x2(std::move(x0));

  best::option<int> x3 = x1;
  t.expect_eq(x3, 42);

  best::option<std::unique_ptr<int>> x4 = new int(42);
  best::option<std::unique_ptr<int>&&> x5 = std::move(x4).as_ref();
  best::option<std::unique_ptr<int>> x6 = std::move(x5);
  t.expect_eq(**x6, 42);
  t.expect_eq(*x4, nullptr);

  auto x7 = std::move(x6).copy();
  t.expect_eq(**x7, 42);
  t.expect_eq(*x6, nullptr);
};

best::test Cmp = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 0;
  best::option<int> x2 = 42;

  t.expect_lt(x0, x1);
  t.expect_lt(x1, x2);
  t.expect_lt(x0, x2);

  t.expect_lt(x0, 0);
  t.expect_lt(x1, 42);
  t.expect_lt(best::none, x2);
};

best::test Map = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 42;

  int c = 0;

  t.expect_eq(x0.map([](int x) { return x + x; }), best::none);
  t.expect_eq(x1.map([](int x) { return x + x; }), 84);

  t.expect_eq(x0.inspect([&](int x) { c += x; }), best::none);
  t.expect_eq(x1.inspect([&](int x) { c += x; }), 42);
  t.expect_eq(c, 42);

  t.expect_eq(x0.map(5, [](int x) { return x + x; }), 5);
  t.expect_eq(x1.map(5, [](int x) { return x + x; }), 84);
  t.expect_eq(x0.map([] { return 5; }, [](int x) { return x + x; }), 5);
  t.expect_eq(x1.map([] { return 5; }, [](int x) { return x + x; }), 84);

  best::option<void> x2;
  best::option<void> x3 = best::VoidOption;

  t.expect_eq(x2.map([] { return 42; }), best::none);
  t.expect_eq(x3.map([] { return 42; }), 42);

  best::option<std::unique_ptr<int>> x4;
  best::option<std::unique_ptr<int>> x5 = new int(42);

  t.expect_eq(std::move(x4).map([](auto&& x) { return std::move(x); }),
              best::none);
  t.expect_eq(**std::move(x5).map([](auto&& x) { return std::move(x); }), 42);
  t.expect_eq(*x5, nullptr);
};

best::test AndThen = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 0;
  best::option<int> x2 = 42;

  auto f = [](int x) -> best::option<double> {
    if (x == 0) return best::none;
    return 1.0 / x;
  };

  t.expect_eq(x0.then(f), best::none);
  t.expect_eq(x1.then(f), best::none);
  t.expect_eq(x2.then(f), 1.0 / 42);
};

best::test Filter = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 0;
  best::option<int> x2 = 42;

  auto f = [](int x) { return x != 0; };

  t.expect_eq(x0.filter(f), best::none);
  t.expect_eq(x1.filter(f), best::none);
  t.expect_eq(x2.filter(f), 42);
};

best::test BitOps = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 0;
  best::option<int> x2 = 42;

  t.expect_eq(x0 & x1, best::none);
  t.expect_eq(x2 & x1, 0);
  t.expect_eq(x1 & x0, best::none);
  t.expect_eq(x0 | x1, 0);
  t.expect_eq(x2 | x1, 42);
  t.expect_eq(x1 | x0, 0);
  t.expect_eq(x0 ^ x1, 0);
  t.expect_eq(x2 ^ x1, best::none);
  t.expect_eq(x1 ^ x0, 0);
};

best::test OkOr = [](auto& t) {
  best::option<int> x0;
  best::option<int> x1 = 42;
  best::option<int&> x2 = x1;

  t.expect_eq(x0.ok_or(5), best::err(5));
  t.expect_eq(x1.ok_or(5), best::ok(42));
  t.expect_eq(x2.ok_or(5), best::ok(42));
  t.expect_eq(&*x2.ok_or(5).ok(), &*x1);

  t.expect_eq(x0.ok_or<best::vec<int>>(best::span{1, 2, 3}),
              best::err(best::span{1, 2, 3}));
  t.expect_eq(x1.ok_or<best::vec<int>>(best::span{1, 2, 3}), best::ok(42));
  t.expect_eq(x2.ok_or<best::vec<int>>(best::span{1, 2, 3}), best::ok(42));

  t.expect_eq(x0.ok_or([] { return 5; }), best::err(5));
  t.expect_eq(x1.ok_or([] { return 5; }), best::ok(42));
  t.expect_eq(x2.ok_or([] { return 5; }), best::ok(42));

  t.expect_eq(x0.err_or(5), best::ok(5));
  t.expect_eq(x1.err_or(5), best::err(42));
  t.expect_eq(x2.err_or(5), best::err(42));
  t.expect_eq(&*x2.err_or(5).err(), &*x1);

  t.expect_eq(x0.err_or<best::vec<int>>(best::span{1, 2, 3}),
              best::ok(best::span{1, 2, 3}));
  t.expect_eq(x1.err_or<best::vec<int>>(best::span{1, 2, 3}), best::err(42));
  t.expect_eq(x2.err_or<best::vec<int>>(best::span{1, 2, 3}), best::err(42));
};

best::test Guard = [](auto& t) {
  auto cb = [](best::option<int> x) -> best::option<int> {
    BEST_GUARD(x);
    return *x * 2;
  };

  t.expect_eq(cb(best::none), best::none);
  t.expect_eq(cb(3), 6);
};
}  // namespace best::option_test