/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

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
