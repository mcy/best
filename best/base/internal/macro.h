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

#ifndef BEST_BASE_HINT222_H_
#define BEST_BASE_HINT222_H_

namespace best {
// clang-format off
#define BEST_COUNT_N_(                      \
         _01, _02, _03, _04, _05, _06, _07, \
    _08, _09, _10, _11, _12, _13, _14, _15, \
    _16, _17, _18, _19, _20, _21, _22, _23, \
    _24, _25, _26, _27, _28, _29, _30, _31, \
    _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, \
    _48, _49, _50, _51, _52, _53, _54, _55, \
    _56, _57, _58, _59, _60, _61, _62, _63, \
    N, ...) N
#define BEST_COUNT_SEQ_             \
    63, 62, 61, 60, 59, 58, 57, 56, \
    55, 54, 53, 52, 51, 50, 49, 48, \
    47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, \
    31, 30, 29, 28, 27, 26, 25, 24, \
    23, 22, 21, 20, 19, 18, 17, 16, \
    15, 14, 13, 12, 11, 10,  9,  8, \
     7,  6,  5,  4,  3,  2,  1,  0
// clang-format on

#define BEST_RTC_LAST_(...) __VA_OPT__(, ) __VA_ARGS__

#define BEST_EXPAND_(...) BEST_EXPAND_4_(__VA_ARGS__)
#define BEST_EXPAND_4_(...) \
  BEST_EXPAND_3_(BEST_EXPAND_3_(BEST_EXPAND_3_(__VA_ARGS__)))
#define BEST_EXPAND_3_(...) \
  BEST_EXPAND_2_(BEST_EXPAND_2_(BEST_EXPAND_2_(__VA_ARGS__)))
#define BEST_EXPAND_2_(...) \
  BEST_EXPAND_1_(BEST_EXPAND_1_(BEST_EXPAND_1_(__VA_ARGS__)))
#define BEST_EXPAND_1_(...) __VA_ARGS__

#define BEST_VARIADIC_(MACRO, ...) \
  __VA_OPT__(BEST_VARIADIC2_(MACRO, BEST_COUNT(__VA_ARGS__), __VA_ARGS__))
#define BEST_VARIADIC2_(MACRO, n_, ...) BEST_VARIADIC3_(MACRO, n_, __VA_ARGS__)
#define BEST_VARIADIC3_(MACRO, n_, ...) \
  BEST_VARIADIC4_ BEST_PARENS(MACRO, n_)(__VA_ARGS__)
#define BEST_VARIADIC4_(MACRO, n_) MACRO##_##n_##_

#define BEST_REMOVE_PARENS_(...) \
  BEST_REMOVE_PARENS2_(BEST_REMOVE_PARENS1_ __VA_ARGS__)
#define BEST_REMOVE_PARENS1_(...) BEST_REMOVE_PARENS1_ __VA_ARGS__
#define BEST_REMOVE_PARENS2_(...) BEST_REMOVE_PARENS3_(__VA_ARGS__)
#define BEST_REMOVE_PARENS3_(...) BEST_REMOVE_PARENS1_##__VA_ARGS__
#define BEST_REMOVE_PARENS1_BEST_REMOVE_PARENS1_

#include "best/base/internal/macro.inc"

}  // namespace best

#endif  // BEST_BASE_HINT_H_
