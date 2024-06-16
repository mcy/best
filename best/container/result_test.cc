
#include "best/container/result.h"

#include "best/test/fodder.h"
#include "best/test/test.h"
#include "best/text/strbuf.h"

namespace best::result_test {
using ::best_fodder::NonTrivialPod;
using ::best_fodder::Stuck;

best::test Eq = [](auto& t) {
  best::result<int, best::str> x0 = best::ok(42);
  best::result<int, best::str> x1 = best::err(best::str("oops!"));

  t.expect_eq(x0.ok(), 42);
  t.expect_eq(x0.err(), best::none);
  t.expect_eq(x1.ok(), best::none);
  t.expect_eq(x1.err(), "oops!");

  t.expect_eq(x0, best::ok(42));
  t.expect_ne(x0, best::err("oops!"));
  t.expect_ne(x1, best::ok(42));
  t.expect_eq(x1, best::err("oops!"));

  t.expect_eq(x0, best::ok());
  t.expect_ne(x0, best::err());
  t.expect_ne(x1, best::ok());
  t.expect_eq(x1, best::err());

  best::result<unsigned, best::str> x2 = best::ok(42);
  best::result<int, best::strbuf> x3 = best::err(best::strbuf("oops!"));

  t.expect_eq(x0, x2);
  t.expect_eq(x1, x3);
  t.expect_ne(x0, x3);
  t.expect_ne(x1, x2);

  best::result<void, best::str> x4 = best::ok();
  best::result<void, best::str> x5 = x1;

  t.expect_eq(x4.ok(), best::VoidOption);
  t.expect_eq(x4.err(), best::none);
  t.expect_eq(x5.ok(), best::none);
  t.expect_eq(x5.err(), "oops!");

  t.expect_eq(x4, best::ok());
  t.expect_ne(x4, best::err("oops!"));
  t.expect_ne(x5, best::ok());
  t.expect_eq(x5, best::err("oops!"));

  t.expect_eq(x4, best::ok());
  t.expect_ne(x4, best::err());
  t.expect_ne(x5, best::ok());
  t.expect_eq(x5, best::err());
};

best::test Cmp = [](auto& t) {
  best::result<int, best::str> x0 = best::ok(1);
  best::result<int, best::str> x1 = best::ok(42);
  best::result<int, best::str> x2 = best::err(best::str("oops!"));
  best::result<int, best::str> x3 = best::err(best::str("oops! 2"));

  t.expect_lt(x0, x1);
  t.expect_lt(x1, x2);
  t.expect_lt(x2, x3);
};

best::test Map = [](auto& t) {
  best::result<int, best::str> x0 = best::ok(42);
  best::result<int, best::str> x1 = best::err(best::str("oops!"));

  t.expect_eq(x0.map([](int x) { return x + x; }), best::ok(84));
  t.expect_eq(x1.map([](int x) { return x + x; }), best::err("oops!"));
  t.expect_eq(x0.map_err([](auto x) { return x.size(); }), best::ok(42));
  t.expect_eq(x1.map_err([](auto x) { return x.size(); }), best::err(5));

  best::result<void, best::str> x2 = best::ok();
  best::result<void, best::str> x3 = x1;

  int c = 0;
  t.expect_eq(x2.map([&] { return ++c; }), best::ok(1));
  t.expect_eq(x3.map([&] { return ++c; }), best::err("oops!"));
  t.expect_eq(x2.map_err([](auto x) { return x.size(); }), best::ok());
  t.expect_eq(x3.map_err([](auto x) { return x.size(); }), best::err(5));
  t.expect_eq(c, 1);

  best::result<std::unique_ptr<int>, best::str> x4(
      x0.map([](int x) { return new int(x); }));
  best::result<std::unique_ptr<int>, best::str> x5(
      x1.map([](int x) { return new int(x); }));

  t.expect_eq(**std::move(x4).map([](auto&& x) { return std::move(x); }), 42);
  t.expect_eq(std::move(x5).map([](auto&& x) { return std::move(x); }),
              best::err("oops!"));
  t.expect_eq(*x4, nullptr);
};

best::test Then = [](auto& t) {
  best::result<int, best::str> x0 = best::ok(42);
  best::result<int, best::str> x1 = best::err(best::str("oops!"));

  t.expect_eq(x0.then([](int x) -> best::result<int, best::str> {
    return best::ok(x + x);
  }),
              best::ok(84));
  t.expect_eq(x1.then([](int x) -> best::result<int, best::str> {
    return best::ok(x + x);
  }),
              best::err("oops!"));

  t.expect_eq(x0.then([](int x) -> best::result<int, best::str> {
    return best::err(best::str("oops?"));
  }),
              best::err("oops?"));
};

best::test Irregular = [](auto& t) {
  best::result<Stuck, int> r = best::ok();
  best::result<NonTrivialPod, int> r2 = best::ok(1, 2);
  (void)r;
  (void)r2;
};

best::test Guard = [](auto& t) {
  auto cb =
      [](best::result<int, best::str> x) -> best::result<bool, best::str> {
    BEST_GUARD(x);
    return *x == 0;
  };

  t.expect_eq(cb(0), best::ok(true));
  t.expect_eq(cb(1), best::ok(false));
  t.expect_eq(cb(best::str("no")), best::err("no"));
};
}  // namespace best::result_test