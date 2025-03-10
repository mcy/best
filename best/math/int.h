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

#ifndef BEST_MATH_INT_H_
#define BEST_MATH_INT_H_

#include <stddef.h>

#include <type_traits>

#include "best/base/fwd.h"
#include "best/base/hint.h"
#include "best/math/internal/common_int.h"
#include "best/meta/traits/quals.h"

//! Utilities for working with primitive integer types.
//!
//! See also `overflow.h` and `bit.h` for more utilities.

namespace best {
/// # `best::is_int`
///
/// Any primitive integer type.
///
/// This explicitly excludes `bool` and non-`char` character types.
template <typename T>
concept is_int = int_internal::is_int<best::un_qual<T>>;

/// # `best::is_byte`
///
/// Any primitive byte type: `char`, `unsigned char`, and `signed char`.
template <typename T>
concept is_byte = best::is_int<T> && sizeof(T) == 1;

/// # `best::bits_of<T>`
///
/// The number of bits in `Int`.
template <best::is_int Int>
inline constexpr size_t bits_of = sizeof(Int) * 8;

/// # `best::is_signed`
///
/// Any primitive signed integer.
template <typename T>
concept is_signed = best::is_int<T> && std::is_signed_v<T>;

/// # `best::to_signed()`
///
/// Casts an integer to its signed counterpart, if it is not already signed.
/// This operation never loses precision.
BEST_INLINE_ALWAYS constexpr auto to_signed(best::is_int auto x) {
  return (std::make_signed_t<decltype(x)>)x;
}

/// # `best::signed_cmp()`
///
/// Compares two integers as if they were signed.
BEST_INLINE_ALWAYS constexpr std::strong_ordering signed_cmp(
  best::is_int auto x, best::is_int auto y) {
  return best::to_signed(x) <=> best::to_signed(y);
}

/// # `best::is_unsigned`
///
/// Any primitive unsigned integer.
template <typename T>
concept is_unsigned = best::is_int<T> && std::is_unsigned_v<T>;

/// # `best::to_unsigned()`
///
/// Casts an integer to its unsigned counterpart, if it is not already unsigned.
/// This operation never loses precision.
BEST_INLINE_ALWAYS constexpr auto to_unsigned(best::is_int auto x) {
  return (std::make_unsigned_t<decltype(x)>)x;
}

/// # `best::unsigned_cmp()`
///
/// Compares two integers as if they were unsigned.
BEST_INLINE_ALWAYS constexpr std::strong_ordering unsigned_cmp(
  best::is_int auto x, best::is_int auto y) {
  return best::to_unsigned(x) <=> best::to_unsigned(y);
}

/// # `best::int_cmp()`
///
/// Compares two integers as if they had infinite-precision.
BEST_INLINE_ALWAYS constexpr std::strong_ordering int_cmp(best::is_int auto x,
                                                          best::is_int auto y) {
  if constexpr (best::is_signed<decltype(x)> == best::is_signed<decltype(y)>) {
    return x <=> y;
  } else if (x < 0) {
    return std::strong_ordering::less;
  } else if (y < 0) {
    return std::strong_ordering::greater;
  } else {
    return best::unsigned_cmp(x, y);
  }
}

/// # `best::max_of<T>`
///
/// The maximum value for a particular integer type.
template <best::is_int Int>
inline constexpr Int max_of =
  // If Int is unsigned, this is 0x11...11.
  // If Int is signed, this is 0x01...11.
  to_unsigned<Int>(-1) >> best::is_signed<Int>;

/// # `best::min_of<T>`
///
/// The minimum value for a particular integer type.
template <best::is_int Int>
inline constexpr Int min_of = ~max_of<Int>;

/// # `best::int_fits()`
///
/// Checks whether an integer is representable by another integer type.
template <best::is_int Int>
BEST_INLINE_ALWAYS constexpr bool int_fits(best::is_int auto x) {
  return best::int_cmp(x, min_of<Int>) >= 0 &&
         best::int_cmp(max_of<Int>, x) >= 0;
}

/// # `best::checked_cast()`
///
/// Casts an integer to another type, returning `best::none` if the cast would
/// not be exact.
template <best::is_int Int>
BEST_INLINE_ALWAYS constexpr best::option<Int> checked_cast(
  best::is_int auto x) {
  if (!best::int_fits<Int>(x)) { return {}; }
  return x;
}

/// # `best::common_int<...>`
///
/// Computes a "common int" type among the given integers.
///
/// This is defined to be the larges integer type among them. If any of them
/// are unsigned, the type is also unsigned.
template <best::is_int... Ints>
using common_int = decltype(best::int_internal::common<Ints...>());

/// # `best::min()`
///
/// Computes the minimum from a collection of signed or unsigned integers.
template <best::is_unsigned... Ints>
BEST_INLINE_ALWAYS constexpr best::common_int<Ints...> min(Ints... args)
  requires (sizeof...(args) > 0)
{
  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wunused-value")
  best::common_int<Ints...> output = (args, ...);
  BEST_POP_GCC_DIAGNOSTIC()
  ((output > args ? output = args : 0), ...);
  return output;
}
template <best::is_signed... Ints>
BEST_INLINE_ALWAYS constexpr best::common_int<Ints...> min(Ints... args)
  requires (sizeof...(args) > 0)
{
  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wunused-value")
  best::common_int<Ints...> output = (args, ...);
  BEST_POP_GCC_DIAGNOSTIC()
  ((output > args ? output = args : 0), ...);
  return output;
}

/// # `best::max()`
///
/// Computes the maximum from a collection of signed or unsigned integers.
template <best::is_unsigned... Ints>
BEST_INLINE_ALWAYS constexpr best::common_int<Ints...> max(Ints... args)
  requires (sizeof...(args) > 0)
{
  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wunused-value")
  best::common_int<Ints...> output = (args, ...);
  BEST_POP_GCC_DIAGNOSTIC()
  ((output < args ? output = args : 0), ...);
  return output;
}
template <best::is_signed... Ints>
BEST_INLINE_ALWAYS constexpr best::common_int<Ints...> max(Ints... args)
  requires (sizeof...(args) > 0)
{
  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wunused-value")
  best::common_int<Ints...> output = (args, ...);
  BEST_POP_GCC_DIAGNOSTIC()
  ((output < args ? output = args : 0), ...);
  return output;
}

/// # `best::smallest_unsigned`
///
/// Computes the smallest unsigned integer type that can represent `n`.
template <uint64_t n>
using smallest_uint_t = best::select<  //
  best::int_fits<uint8_t>(n), uint8_t,
  best::select<  //
    best::int_fits<uint16_t>(n), uint16_t,
    best::select<  //
      best::int_fits<uint32_t>(n),
      uint32_t,  //
      uint64_t>>>;
}  // namespace best

#endif  // BEST_MATH_INT_H_
