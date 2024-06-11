#include "best/container/vec.h"

#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::vec_test {
using ::best_fodder::LeakTest;

best::test Empty = [](auto& t) {
  best::vec<int> empty;
  static_assert(sizeof(empty) == 3 * sizeof(void*));

  t.expect(empty.is_empty());
  t.expect_eq(empty.size(), 0);

  t.expect_eq(empty[{.start = 0}], best::span<int>{});
  t.expect_eq(empty[{.end = 0}], best::span<int>{});
  t.expect_eq(empty[{.count = 0}], best::span<int>{});
};

best::test Chars = [](auto& t) {
  best::vec<std::byte> bytes;
  t.expect_eq(bytes.capacity(), sizeof(void*) * 3 - 1);

  for (size_t i = 0; i < bytes.capacity(); ++i) {
    bytes.push(std::byte(i));
  }

  // Verify we're still in inlined mode.
  t.expect_eq(&bytes, bytes.data());
  t.expect(bytes.is_inlined());

  // Construct a vector to compare with.
  std::vector<std::byte> std_bytes;
  for (size_t i = 0; i < bytes.size(); ++i) {
    std_bytes.push_back(std::byte(i));
  }

  t.expect_eq(bytes, best::span(std_bytes));

  // Push one more byte to push it over to the heap.
  bytes.push(std::byte(bytes.size()));
  std_bytes.push_back(std::byte(bytes.size() - 1));

  // Verify we're *not* in inlined mode.
  t.expect_ne(&bytes, bytes.data());
  t.expect(bytes.is_on_heap());
  t.expect_eq(bytes, best::span(std_bytes));
};

best::test CopyMove = [](auto& t) {
  best::vec ints = {1, 2, 3, 4, 5};
  t.expect_eq(ints.size(), 5);
  t.expect_eq(ints, best::span{1, 2, 3, 4, 5});

  auto ints2 = ints;
  t.expect_eq(ints2, best::span{1, 2, 3, 4, 5});
  auto ints3 = std::move(ints);
  t.expect_eq(ints3, best::span{1, 2, 3, 4, 5});
  t.expect(ints.is_empty());

  ints2.push(6);
  t.expect_eq(ints2, best::span{1, 2, 3, 4, 5, 6});
  ints = ints2;
  t.expect_eq(ints, best::span{1, 2, 3, 4, 5, 6});

  ints.push(7);
  ints.push(8);
  t.expect_eq(ints, best::span{1, 2, 3, 4, 5, 6, 7, 8});
  ints = std::move(ints2);
  t.expect_eq(ints, best::span{1, 2, 3, 4, 5, 6});
};

best::test Leaky = [](auto& t) {
  LeakTest l_(t);

  using Bubble = LeakTest::Bubble;

  best::vec<Bubble> x0;
  x0 = best::vec{Bubble()};
  x0.clear();
  x0.push();
  x0.push();

  auto x1 = x0;
  auto x2 = std::move(x0);
  auto x3 = x0;

  x2 = x1;
  x2 = std::move(x1);

  x0.clear();
  x0 = x2;
  x0.clear();
  x2 = x0;

  x0.push();
  x2.push(std::move(x0[0]));
  x0.push();
  x2[0] = x0[0];
};
}  // namespace best::vec_test