#ifndef BEST_META_INTERNAL_TRAITS_H_
#define BEST_META_INTERNAL_TRAITS_H_

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
}  // namespace best::traits_internal

#endif  // BEST_META_INTERNAL_TRAITS_H_