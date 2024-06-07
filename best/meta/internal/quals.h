#ifndef BEST_META_INTERNAL_QUALS_H_
#define BEST_META_INTERNAL_QUALS_H_

#include <type_traits>

namespace best::quals_internal {
template <typename Dst, typename Src>
struct quals {
  using copied = Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, const Src> {
  using copied = const Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, volatile Src> {
  using copied = volatile Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, const volatile Src> {
  using copied = const volatile Dst;
};

template <typename Dst, typename Src>
struct refs {
  using copied = quals<Dst, Src>::copied;
};
template <typename Dst, typename Src>
struct refs<Dst, Src&> {
  using copied = std::add_lvalue_reference_t<typename quals<Dst, Src>::copied>;
};
template <typename Dst, typename Src>
struct quals<Dst, Src&&> {
  using copied = std::add_rvalue_reference_t<typename quals<Dst, Src>::copied>;
};
}  // namespace best::quals_internal

#endif  // BEST_META_INTERNAL_QUALS_H_