#ifndef BEST_MATH_INTERNAL_COMMON_INT_H_
#define BEST_MATH_INTERNAL_COMMON_INT_H_

#include <stddef.h>

#include <type_traits>

namespace best::int_internal {
constexpr size_t widest(auto ints) {
  size_t idx = 0;
  size_t cur = 0;
  size_t size = 0;
  ints.each([&]<typename T> {
    if (sizeof(T) > size) {
      size = sizeof(T);
      cur = idx;
    }
    ++idx;
  });
  return cur;
}

constexpr bool any_unsigned(auto ints) {
  return ints.template map<std::is_unsigned>().any();
}

template <auto ints, bool u = any_unsigned(ints),
          typename W = decltype(ints)::template type<widest(ints)>>
std::conditional_t<u, std::make_unsigned_t<W>, W> common();
}  // namespace best::int_internal

#endif  // BEST_MATH_INTERNAL_COMMON_INT_H_