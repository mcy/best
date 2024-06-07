#ifndef BEST_META_INTERNAL_ABOMINABLE_H_
#define BEST_META_INTERNAL_ABOMINABLE_H_

//! Implementation of best::tame_func.

namespace best {
namespace abominable_internal {

template <typename F>
struct tame {
  using type = F;
};

#define BEST_TAME_(suffix_)            \
  template <typename O, typename... I> \
  struct tame<O(I...) suffix_> {       \
    using type = O(I...);              \
  };                                   \
  template <typename O, typename... I> \
  struct tame<O(I..., ...) suffix_> {  \
    using type = O(I..., ...);         \
  }

BEST_TAME_();
BEST_TAME_(&);
BEST_TAME_(&&);
BEST_TAME_(const);
BEST_TAME_(const&);
BEST_TAME_(const&&);
BEST_TAME_(volatile);
BEST_TAME_(volatile&);
BEST_TAME_(volatile&&);
BEST_TAME_(const volatile);
BEST_TAME_(const volatile&);
BEST_TAME_(const volatile&&);
BEST_TAME_(noexcept);
BEST_TAME_(& noexcept);
BEST_TAME_(&& noexcept);
BEST_TAME_(const noexcept);
BEST_TAME_(const& noexcept);
BEST_TAME_(const&& noexcept);
BEST_TAME_(volatile noexcept);
BEST_TAME_(volatile& noexcept);
BEST_TAME_(volatile&& noexcept);
BEST_TAME_(const volatile noexcept);
BEST_TAME_(const volatile& noexcept);
BEST_TAME_(const volatile&& noexcept);

#undef BEST_TAME_

}  // namespace abominable_internal
}  // namespace best

#endif  // BEST_META_INTERNAL_ABOMINABLE_H_