#ifndef BEST_META_INTERNAL_ABOMINABLE_H_
#define BEST_META_INTERNAL_ABOMINABLE_H_

//! Implementation of best::tame_func.

namespace best {
namespace abominable_internal {

template <typename F>
struct tame {
  using type = F;
  static constexpr bool c{}, v{}, l{}, r{};
};

#define BEST_TAME_(c_, v_, l_, r_, suffix_)           \
  template <typename O, typename... I>                \
  struct tame<O(I...) suffix_> {                      \
    using type = O(I...);                             \
    static constexpr bool c{c_}, v{v_}, l{l_}, r{r_}; \
  };                                                  \
  template <typename O, typename... I>                \
  struct tame<O(I..., ...) suffix_> {                 \
    using type = O(I..., ...);                        \
    static constexpr bool c{c_}, v{v_}, l{l_}, r{r_}; \
  }

BEST_TAME_(0, 0, 0, 0, );
BEST_TAME_(0, 0, 1, 0, &);
BEST_TAME_(0, 0, 0, 1, &&);
BEST_TAME_(1, 0, 0, 0, const);
BEST_TAME_(1, 0, 1, 0, const&);
BEST_TAME_(1, 0, 0, 1, const&&);
BEST_TAME_(0, 1, 1, 0, volatile);
BEST_TAME_(0, 1, 0, 1, volatile&);
BEST_TAME_(0, 1, 0, 0, volatile&&);
BEST_TAME_(1, 1, 0, 0, const volatile);
BEST_TAME_(1, 1, 1, 0, const volatile&);
BEST_TAME_(1, 1, 0, 1, const volatile&&);
BEST_TAME_(0, 0, 0, 0, noexcept);
BEST_TAME_(0, 0, 0, 0, & noexcept);
BEST_TAME_(0, 0, 0, 0, && noexcept);
BEST_TAME_(1, 0, 0, 0, const noexcept);
BEST_TAME_(1, 0, 1, 0, const& noexcept);
BEST_TAME_(1, 0, 0, 1, const&& noexcept);
BEST_TAME_(0, 1, 0, 0, volatile noexcept);
BEST_TAME_(0, 1, 1, 0, volatile& noexcept);
BEST_TAME_(0, 1, 0, 1, volatile&& noexcept);
BEST_TAME_(1, 1, 0, 0, const volatile noexcept);
BEST_TAME_(1, 1, 1, 0, const volatile& noexcept);
BEST_TAME_(1, 1, 0, 1, const volatile&& noexcept);

#undef BEST_TAME_

}  // namespace abominable_internal
}  // namespace best

#endif  // BEST_META_INTERNAL_ABOMINABLE_H_