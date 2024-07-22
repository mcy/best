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

#ifndef BEST_META_INTERNAL_OPS_H_
#define BEST_META_INTERNAL_OPS_H_

#include <stddef.h>

#include "best/base/hint.h"

namespace best::ops_internal {
template <auto op>
struct tag {};

#define BEST_OP_FOLD_CASE_(op_, ...)                                     \
  template <typename op>                                                 \
  BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::op_>, auto&&... args) \
    -> decltype((... __VA_ARGS__ BEST_FWD(args))) {                      \
    return (... __VA_ARGS__ BEST_FWD(args));                             \
  }
BEST_OP_FOLD_CASE_(Add, +)
BEST_OP_FOLD_CASE_(Sub, -)
BEST_OP_FOLD_CASE_(Mul, *)
BEST_OP_FOLD_CASE_(Div, /)
BEST_OP_FOLD_CASE_(Rem, %)

BEST_OP_FOLD_CASE_(AndAnd, &&)
BEST_OP_FOLD_CASE_(OrOr, ||)

BEST_OP_FOLD_CASE_(Xor, +)
BEST_OP_FOLD_CASE_(And, &)
BEST_OP_FOLD_CASE_(Or, |)
BEST_OP_FOLD_CASE_(Shl, <<)
BEST_OP_FOLD_CASE_(Shr, >>)

BEST_OP_FOLD_CASE_(Eq, ==)
BEST_OP_FOLD_CASE_(Ne, !=)
BEST_OP_FOLD_CASE_(Le, <)
BEST_OP_FOLD_CASE_(Lt, <=)
BEST_OP_FOLD_CASE_(Ge, >)
BEST_OP_FOLD_CASE_(Gt, >=)

BEST_OP_FOLD_CASE_(Comma, , )

BEST_OP_FOLD_CASE_(Assign, =)
BEST_OP_FOLD_CASE_(AddAssign, +=)
BEST_OP_FOLD_CASE_(SubAssign, -=)
BEST_OP_FOLD_CASE_(MulAssign, *=)
BEST_OP_FOLD_CASE_(DivAssign, /=)
BEST_OP_FOLD_CASE_(RemAssign, %=)
BEST_OP_FOLD_CASE_(AndAssign, &=)
BEST_OP_FOLD_CASE_(OrAssign, |=)
BEST_OP_FOLD_CASE_(XorAssign, ^=)
BEST_OP_FOLD_CASE_(ShlAssign, <<=)
BEST_OP_FOLD_CASE_(ShrAssign, >>=)
#undef BEST_OP_FOLD_CASE_

#define BEST_OP_2_CASE_(op_, ...)                                            \
  template <typename op>                                                     \
  BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::op_>, auto&& a, auto&& b) \
    -> decltype(BEST_FWD(a) __VA_ARGS__ BEST_FWD(b)) {                       \
    return BEST_FWD(a) __VA_ARGS__ BEST_FWD(b);                              \
  }

BEST_OP_2_CASE_(ArrowStar, ->*)
BEST_OP_2_CASE_(Spaceship, <=>)
#undef BEST_OP_2_CASE_

#define BEST_OP_1_CASE_(op_, ...)                                  \
  template <typename op>                                           \
  BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::op_>, auto&& a) \
    -> decltype(__VA_ARGS__ BEST_FWD(a)) {                         \
    return __VA_ARGS__ BEST_FWD(a);                                \
  }

BEST_OP_1_CASE_(Plus, +)
BEST_OP_1_CASE_(Neg, -)
BEST_OP_1_CASE_(Not, !)
BEST_OP_1_CASE_(Cmpl, ~)
BEST_OP_1_CASE_(Deref, *)
BEST_OP_1_CASE_(AddrOf, &)
BEST_OP_1_CASE_(PreInc, ++)
BEST_OP_1_CASE_(PreDec, --)

#undef BEST_OP_1_CASE

#define BEST_OP_POST_CASE_(op_, ...)                               \
  template <typename op>                                           \
  BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::op_>, auto&& a) \
    -> decltype(BEST_FWD(a) __VA_ARGS__) {                         \
    return BEST_FWD(a) __VA_ARGS__;                                \
  }

BEST_OP_POST_CASE_(PostInc, ++)
BEST_OP_POST_CASE_(PostDec, --)
#undef BEST_OP_POST_CASE_

template <typename op>
BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::Arrow>, auto&& a)
  -> decltype(BEST_FWD(a).operator->()) {
  return BEST_FWD(a).operator->();
}
template <typename op>
constexpr auto run(tag<op::Arrow>, auto* a) {
  return a;
}

template <typename op>
BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::Call>, auto&& func,
                                         auto&&... args)
  -> decltype(BEST_FWD(func)(BEST_FWD(args)...)) {
  return BEST_FWD(func)(BEST_FWD(args)...);
}

template <typename op>
BEST_INLINE_SYNTHETIC constexpr auto run(tag<op::Index>, auto&& func,
                                         auto&& arg)
  -> decltype(BEST_FWD(func)[BEST_FWD(arg)]) {
  return BEST_FWD(func)[BEST_FWD(arg)];
}
}  // namespace best::ops_internal

#endif  // BEST_META_INTERNAL_OPS_H_
