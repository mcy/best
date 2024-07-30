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

#include "best/base/guard.h"
#include "best/math/int.h"
#include "best/math/overflow.h"
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
template <best::is_int Int>
constexpr best::result<Int, best::atoi_error> atoi(
  const best::is_string auto &str, uint32_t radix = 10);

/// # `best::atoi_with_prefix()`
///
/// Similar to `best::atoi()`, but determines which radix to parse in based
/// on a prefix, such as `0x`, `0b`, `0o`, or `0`.
template <best::is_int Int>
constexpr best::result<Int, best::atoi_error> atoi_with_prefix(
  const best::is_string auto &str);

/// # `best::atoi_with_sign()`
///
/// Similar to `best::atoi()`, but takes the sign of the value as a separate
/// argument.
template <best::is_int Int>
constexpr best::result<Int, best::atoi_error> atoi_with_sign(
  const best::is_string auto &str, bool is_negative, uint32_t radix) {
  if constexpr (best::is_pretext<decltype(str)>) {
    // Adapted slightly from the implementation found in Rust's
    // from_str_radix().

    if (radix > 36) {
      crash_internal::crash("from_digit() radix too large: %u > 36", radix);
    }

    auto runes = str.runes();
    auto next = runes.next();

    if (!next) { return best::atoi_error{}; }

    // We make an approximation that the number of digits provided by `str` is
    // no larger than its length in code units. The greatest information density
    // is when the radix is 16; in this case, if the length is less than or
    // equal to the maximum number if nybbles, it will not overflow. However, if
    // it is a signed type, we need to subtract off one extra code unit, since
    // e.g. `80` will overflow `int8_t`.
    size_t total_codes = best::size(str);
    size_t maximum_codes_without_overflow =
      sizeof(Int) * 2 - best::is_signed<Int>;
    size_t cannot_overflow =
      radix <= 16 && total_codes <= maximum_codes_without_overflow;

#define BEST_ATOI_LOOP_(result_, op_)                             \
  do {                                                            \
    result_ *= Int(radix);                                        \
    auto digit = next->to_digit(radix).ok_or(best::atoi_error{}); \
    BEST_GUARD(digit);                                            \
    result_ op_ Int(*digit);                                      \
  } while ((next = runes.next()))

    if (cannot_overflow) {
      Int result = 0;
      if (is_negative) {
        BEST_ATOI_LOOP_(result, -=);
      } else {
        BEST_ATOI_LOOP_(result, +=);
      }
      return result;
    }

#undef BEST_ATOI_LOOP_
#define BEST_ATOI_LOOP_(result_, op_)                             \
  do {                                                            \
    result_ *= Int(radix);                                        \
    auto digit = next->to_digit(radix).ok_or(best::atoi_error{}); \
    BEST_GUARD(digit);                                            \
    result_ op_ Int(*digit);                                      \
    BEST_GUARD(result_.checked().ok_or(best::atoi_error{}));      \
  } while ((next = runes.next()))

    best::overflow<Int> result = 0;
    if (is_negative) {
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

template <best::is_int Int>
constexpr best::result<Int, best::atoi_error> atoi(
  const best::is_string auto &str_, uint32_t radix) {
  if constexpr (best::is_pretext<decltype(str_)>) {
    if (radix > 36) {
      crash_internal::crash("from_digit() radix too large: %u > 36", radix);
    }

    auto str = str_;
    bool neg = str.consume_prefix('-');
    if (!neg) { str.consume_prefix('+'); }

    return best::atoi_with_sign<Int>(str, neg, radix);
  } else {
    return best::atoi<Int>(best::pretext(str_), radix);
  }
}

template <best::is_int Int>
constexpr best::result<Int, best::atoi_error> atoi_with_prefix(
  const best::is_string auto &str_) {
  if constexpr (best::is_pretext<decltype(str_)>) {
    auto str = str_;
    bool neg = str.consume_prefix('-');
    if (!neg) { str.consume_prefix('+'); }

    if (str == "0") { return 0; }

    int radix = 10;
    if (str.consume_prefix("0x")) {
      radix = 16;
    } else if (str.consume_prefix("0b")) {
      radix = 2;
    } else if (str.consume_prefix("0o") || str.consume_prefix("0")) {
      radix = 8;
    }

    return best::atoi_with_sign<Int>(str, neg, radix);
  } else {
    return best::atoi_with_prefix<Int>(best::pretext(str_));
  }
}

}  // namespace best

#endif  // BEST_MATH_CONV_H_
