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

#ifndef BEST_MATH_CONV_H_
#define BEST_MATH_CONV_H_

#include "best/math/int.h"
#include "best/math/overflow.h"
#include "best/meta/guard.h"
#include "best/text/str.h"

//! Number-string conversion primitives.

namespace best {
/// # `best::atoi_error`
///
/// An error returned by `best::atoi()`.
struct atoi_error final {
  friend void BestFmt(auto &fmt, atoi_error) {
    auto rec = fmt.record("atoi_error");
  }
};

/// # `best::atoi()`
///
/// Parses an integer from the given string type in the specified radix.
template <best::integer Int>
constexpr best::result<Int, best::atoi_error> atoi(const string_type auto &str,
                                                   uint32_t radix = 10) {
  if constexpr (best::is_pretext<decltype(str)>) {
    // Adapted slightly from the implementation found in Rust's
    // from_str_radix().

    if (radix > 36) {
      crash_internal::crash("from_digit() radix too large: %u > 36", radix);
    }

    auto runes = str.runes();
    auto next = runes.next();

    bool neg = false;
    if (next == '-') {
      neg = true;
      next = runes.next();
    } else if (next == '+') {
      next = runes.next();
    }

    if (!next) return best::atoi_error{};

    // We make an approximation that the number of digits provided by `str` is
    // no larger than its length in code units. The greatest information density
    // is when the radix is 16; in this case, if the length is less than or
    // equal to the maximum number if nybbles, it will not overflow. However, if
    // it is a signed type, we need to subtract off one extra code unit, since
    // e.g. `80` will overflow `int8_t`.
    size_t total_codes = best::size(str);
    size_t maximum_codes_without_overflow =
        sizeof(Int) * 2 - best::signed_int<Int>;
    size_t cannot_overflow =
        radix <= 16 && total_codes <= maximum_codes_without_overflow;

#define BEST_ATOI_LOOP_(result_, op_)                             \
  do {                                                            \
    result_ *= radix;                                             \
    auto digit = next->to_digit(radix).ok_or(best::atoi_error{}); \
    BEST_GUARD(digit);                                            \
    result_ op_ *digit;                                           \
  } while ((next = runes.next()))

    if (cannot_overflow) {
      Int result = 0;
      if (neg) {
        BEST_ATOI_LOOP_(result, -=);
      } else {
        BEST_ATOI_LOOP_(result, +=);
      }
      return result;
    }

#undef BEST_ATOI_LOOP_
#define BEST_ATOI_LOOP_(result_, op_)                             \
  do {                                                            \
    result_ *= radix;                                             \
    auto digit = next->to_digit(radix).ok_or(best::atoi_error{}); \
    BEST_GUARD(digit);                                            \
    result_ op_ *digit;                                           \
    BEST_GUARD(result_.checked().ok_or(best::atoi_error{}));      \
  } while ((next = runes.next()))

    best::overflow<Int> result = 0;
    if (neg) {
      BEST_ATOI_LOOP_(result, -=);
    } else {
      BEST_ATOI_LOOP_(result, +=);
    }
    return result.wrap();

#undef BEST_ATOI_LOOP_
  } else {
    return best::atoi<Int>(best::pretext(str), radix);
  }
}
}  // namespace best

#endif  // BEST_MATH_CONV_H_
