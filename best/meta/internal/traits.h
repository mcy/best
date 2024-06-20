#ifndef BEST_META_INTERNAL_TRAITS_H_
#define BEST_META_INTERNAL_TRAITS_H_

#include <type_traits>

namespace best::traits_internal {
template <typename T, typename...>
struct dependent {
  using type = T;
};

template <bool cond, typename A, typename B>
struct select {
  using type = A;
};
template <typename A, typename B>
struct select<false, A, B> {
  using type = B;
};

template <typename T>
concept nonvoid = !std::is_void_v<T>;
}  // namespace best::traits_internal

#endif  // BEST_META_INTERNAL_TRAITS_H_