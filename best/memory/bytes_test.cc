#include "best/memory/bytes.h"

#include "best/container/span.h"
#include "best/test/test.h"

namespace best::bytes_test {

// Equality checker that does not depend on best::span::operator==, which is
// what this test is testing in the first place.
template <typename T, typename U>
void eq(best::test& t, best::span<T> a, best::span<U> b,
        best::location loc = best::here) {
  if (!t.expect(a.size() == b.size(),
                {"expected sizes to be equal: {} != {}", loc}, a.size(),
                b.size())) {
    return;
  }

  bool equal = true;
  for (size_t i = 0; i < a.size(); ++i) {
    equal &= a[i] == b[i];
  }

  t.expect(equal, {"expected equal values:\n  {:x?}\n  {:x?}", loc}, a, b);
}

best::test Mutate = [](auto& t) {
  char a_[16] = {};
  auto a = best::span<char>(a_);
  best::fill_bytes(a, '?');
  eq(t, a, best::span("????????????????", 16));

  auto b = best::from_nul("abcdefgh");
  auto c = best::from_nul("xzyxzyxzyxzyxzyxzy");

  best::copy_bytes(a, b);
  eq(t, a, best::span("abcdefgh????????", 16));

  best::copy_overlapping_bytes(a[{.start = 7}], a[{.end = 7}]);
  eq(t, a, best::span("abcdefgabcdefg??", 16));

  best::copy_bytes(a, c);
  eq(t, a, c[{.end = 16}]);
};

best::test Equate = [](auto& t) {
  constexpr auto a = best::from_nul("abcdefgh1");
  constexpr auto b = best::from_nul("abcdefgh2");
  constexpr best::span<const char> e;

  t.expect(best::equate_bytes(e, e));
  t.expect(!best::equate_bytes(e, b));
  t.expect(!best::equate_bytes(a, e));

  t.expect(!best::equate_bytes(a, b));
  t.expect(!best::equate_bytes(a[{.end = 8}], b));
  t.expect(!best::equate_bytes(a, b[{.end = 8}]));
  t.expect(best::equate_bytes(a[{.end = 8}], b[{.end = 8}]));

#if BEST_CONSTEXPR_MEMCMP
  static_assert(best::equate_bytes(e, e));
  static_assert(!best::equate_bytes(e, b));
  static_assert(!best::equate_bytes(a, e));

  static_assert(!best::equate_bytes(a, b));
  static_assert(!best::equate_bytes(a[{.end = 8}], b));
  static_assert(!best::equate_bytes(a, b[{.end = 8}]));
  static_assert(best::equate_bytes(a[{.end = 8}], b[{.end = 8}]));
#endif

  int x[] = {1, 2, 3, 4, 5, 6, 7};
  int y[] = {1, 2, 3, 4, 5, 6, 8};

  best::span<const int> a2 = x;
  best::span<const int> b2 = y;
  best::span<const int> e2;

  t.expect(best::equate_bytes(e2, e2));
  t.expect(!best::equate_bytes(e2, b2));
  t.expect(!best::equate_bytes(a2, e2));

  t.expect(!best::equate_bytes(a2, b2));
  t.expect(!best::equate_bytes(a2[{.end = 6}], b2));
  t.expect(!best::equate_bytes(a2, b2[{.end = 6}]));
  t.expect(best::equate_bytes(a2[{.end = 6}], b2[{.end = 6}]));
};

best::test Compare = [](auto& t) {
  constexpr auto a = best::from_nul("abcdefgh1");
  constexpr auto b = best::from_nul("abcdefgh2");
  constexpr best::span<const char> e;

  t.expect(best::compare_bytes(e, e) == 0);
  t.expect(best::compare_bytes(e, b) < 0);
  t.expect(best::compare_bytes(a, e) > 0);

  t.expect(best::compare_bytes(a, b) < 0);
  t.expect(best::compare_bytes(a[{.end = 8}], b) < 0);
  t.expect(best::compare_bytes(a, b[{.end = 8}]) > 0);
  t.expect(best::compare_bytes(a[{.end = 8}], b[{.end = 8}]) == 0);

#if BEST_CONSTEXPR_MEMCMP
  static_assert(best::compare_bytes(e, e) == 0);
  static_assert(best::compare_bytes(e, b) < 0);
  static_assert(best::compare_bytes(a, e) > 0);

  static_assert(best::compare_bytes(a, b) < 0);
  static_assert(best::compare_bytes(a[{.end = 8}], b) < 0);
  static_assert(best::compare_bytes(a, b[{.end = 8}]) > 0);
  static_assert(best::compare_bytes(a[{.end = 8}], b[{.end = 8}]) == 0);
#endif

  int x[] = {1, 2, 3, 4, 5, 6, 7};
  int y[] = {1, 2, 3, 4, 5, 6, 8};

  best::span<const int> a2 = x;
  best::span<const int> b2 = y;
  best::span<const int> e2;

  t.expect(best::compare_bytes(e2, e2) == 0);
  t.expect(best::compare_bytes(e2, b2) < 0);
  t.expect(best::compare_bytes(a2, e2) > 0);

  t.expect(best::compare_bytes(a2, b2) < 0);
  t.expect(best::compare_bytes(a2[{.end = 6}], b2) < 0);
  t.expect(best::compare_bytes(a2, b2[{.end = 6}]) > 0);
  t.expect(best::compare_bytes(a2[{.end = 6}], b2[{.end = 6}]) == 0);
};

best::test Search = [](auto& t) {
  constexpr auto a = best::from_nul("abcddefgh");
  constexpr auto b = best::from_nul("abc");
  constexpr auto c = best::from_nul("def");
  constexpr auto d = best::from_nul("ghi");
  constexpr auto f = best::from_nul("abcddefghi");
  constexpr best::span<const char> e;

  t.expect_eq(best::search_bytes(a, a), 0);
  t.expect_eq(best::search_bytes(a, b), 0);
  t.expect_eq(best::search_bytes(a, c), 4);
  t.expect_eq(best::search_bytes(a, d), best::none);
  t.expect_eq(best::search_bytes(a, e), 0);
  t.expect_eq(best::search_bytes(a, f), best::none);

#if BEST_CONSTEXPR_MEMCMP
  static_assert(best::search_bytes(a, a) == 0);
  static_assert(best::search_bytes(a, b) == 0);
  static_assert(best::search_bytes(a, c) == 4);
  static_assert(best::search_bytes(a, d) == best::none);
  static_assert(best::search_bytes(a, e) == 0);
  static_assert(best::search_bytes(a, f) == best::none);
#endif

  int x_[] = {1, 2, 3, 4, 4, 5, 6, 7, 8, 9};
  best::span x = x_;

  best::span<const int> a2 = x[{.count = 8}];
  best::span<const int> b2 = x[{.count = 3}];
  best::span<const int> c2 = x[{.start = 4, .count = 3}];
  best::span<const int> d2 = x[{.start = 7, .count = 3}];
  best::span<const int> f2 = x;
  best::span<const int> e2;

  t.expect_eq(best::search_bytes(a2, a2), 0);
  t.expect_eq(best::search_bytes(a2, b2), 0);
  t.expect_eq(best::search_bytes(a2, c2), 4);
  t.expect_eq(best::search_bytes(a2, d2), best::none);
  t.expect_eq(best::search_bytes(a2, e2), 0);
  t.expect_eq(best::search_bytes(a2, f2), best::none);
};
}  // namespace best::bytes_test