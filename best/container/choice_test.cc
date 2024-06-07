#include "best/container/choice.h"

#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::choice_test {
using ::best_fodder::LeakTest;

static_assert(sizeof(best::choice<int&, best::empty>) == sizeof(int*));

best::test Nums = [](auto& t) {
  best::choice<int, float, bool> x0(best::index<0>, 42);
  best::choice<int, float, bool> x1(best::index<1>, 1.5);
  best::choice<int, float, bool> x2(best::index<2>, true);

  t.expect_eq(x0.which(), 0);
  t.expect_eq(x1.which(), 1);
  t.expect_eq(x2.which(), 2);

  t.expect_eq(x0[best::index<0>], 42);
  t.expect_eq(x1[best::index<1>], 1.5);
  t.expect_eq(x2[best::index<2>], true);

  t.expect_eq(x0, x0);
  t.expect_eq(x1, x1);
  t.expect_eq(x2, x2);
};

best::test Convert = [](auto& t) {
  best::choice<int, best::str> x0 = 42;
  best::choice<int, best::str> x1 = best::str("foo");

  t.expect_eq(x0.which(), 0);
  t.expect_eq(x1.which(), 1);
  t.expect_eq(x0[best::index<0>], 42);
  t.expect_eq(x1[best::index<1>], "foo");
};

best::test Accessors = [](auto& t) {
  best::choice<int, int> x0(best::index<0>, 42);

  t.expect_eq(x0[best::index<0>], 42);
  t.expect_eq(x0.at(best::index<0>), 42);
  t.expect_eq(x0.at(best::unsafe, best::index<0>), 42);
  t.expect_eq(*x0.as_ptr(best::index<0>), 42);

  x0[best::index<0>]++;
  t.expect_eq(x0[best::index<0>], 43);
  x0.at(best::index<0>).value()++;
  t.expect_eq(x0[best::index<0>], 44);
  x0.at(best::unsafe, best::index<0>)++;
  t.expect_eq(x0[best::index<0>], 45);
  (*x0.at(best::index<0>))++;
  t.expect_eq(x0[best::index<0>], 46);

  t.expect_eq(x0.at(best::index<1>), best::none);
  t.expect_eq(x0.as_ptr(best::index<1>), nullptr);
};

best::test Leaky = [](auto& t) {
  LeakTest l_(t);

  using Bubble = LeakTest::Bubble;

  best::choice<int, Bubble> x0 = 42;
  x0 = Bubble();
  x0.emplace<0>();
  x0.emplace<1>();

  auto x1 = x0;
  auto x2 = std::move(x0);
  auto x3 = x0;

  x2 = x1;
  x2 = std::move(x1);

  x0.emplace<0>();
  x0 = x2;
  x0.emplace<0>();
  x2 = x0;

  x0 = best::choice<int, Bubble>(best::index<1>);
  x2 = x0[best::index<1>];
  x0.emplace<1>();
  x2[best::index<1>] = x0[best::index<1>];
};

best::test Match = [](auto& t) {
  best::choice<int, float> x0(best::index<0>, 42);
  best::choice<int, float> x1(best::index<1>, 43.6);

  t.expect_eq(x0.match([](auto x) { return (int)x; }), 42);
  t.expect_eq(x1.match([](auto x) { return (int)x; }), 43);

  t.expect_eq(x0.match(                         //
                  [](int x) { return x * 2; },  //
                  [](float f) { return (int)f; }),
              84);
  t.expect_eq(x1.match(                         //
                  [](int x) { return x * 2; },  //
                  [](float f) { return (int)f; }),
              43);

  best::choice<int, int> x2(best::index<0>, 42);
  best::choice<int, int> x3(best::index<1>, 45);

  t.expect_eq(x2.match(                                           //
                  [](best::index_t<0>, int x) { return x * 2; },  //
                  [](int x) { return x; }),
              84);
  t.expect_eq(x3.match(                                           //
                  [](best::index_t<0>, int x) { return x * 2; },  //
                  [](int x) { return x; }),
              45);
};

best::test Permute = [](auto& t) {
  best::choice<int, float, int*> x0(best::index<1>, 42.5);
  best::choice<int*, int, float> x1 = x0.permute<2, 0, 1>();
  t.expect_eq(x1.at<2>(), 42.5);
};

best::test Comparisons = [](auto& t) {
  std::array<best::choice<int, float, bool>, 6> alts = {{
      {best::index<0>, 42},
      {best::index<0>, 45},
      {best::index<1>, 1.5},
      {best::index<1>, 1.7},
      {best::index<2>, false},
      {best::index<2>, true},
  }};

  for (size_t i = 0; i < 6; ++i) {
    for (size_t j = 0; i < 6; ++i) {
      if (i < j) {
        t.expect_lt(alts[i], alts[j]);
      } else if (i > j) {
        t.expect_gt(alts[i], alts[j]);
      } else {
        t.expect_eq(alts[i], alts[j]);
      }
    }
  }
};
}  // namespace best::choice_test