#ifndef BEST_MATH_INT_H_
#define BEST_MATH_INT_H_

#include <stddef.h>

#include <compare>
#include <limits>
#include <type_traits>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/math/internal/int.h"
#include "best/meta/concepts.h"

//! Utilities for working with primitive integer types.

// Bibliography:
//  [HD13] Warren, H. S. Jr. Hacker's Delight. 2013 Addison Wesley, 2nd ed.

namespace best {
/// Any primitive integer type.
///
/// This explicitly excludes `bool` and non-`char` character types.
template <typename T>
concept integer =
    std::is_integral_v<T> && !best::same<best::as_dequal<T>, bool> &&
    !best::same<best::as_dequal<T>, wchar_t> &&
    !best::same<best::as_dequal<T>, char16_t> &&
    !best::same<best::as_dequal<T>, char32_t>;

/// Any primitive signed int.
template <typename T>
concept signed_int = best::integer<T> && std::is_signed_v<T>;

/// Any primitive unsigned int.
template <typename T>
concept unsigned_int = best::integer<T> && std::is_unsigned_v<T>;

/// Casts an integer to its signed counterpart, if it is not already signed.
BEST_INLINE_ALWAYS constexpr auto to_signed(integer auto x) {
  return (std::make_signed_t<decltype(x)>)x;
}

/// Casts an integer to its unsigned counterpart, if it is not already unsigned.
BEST_INLINE_ALWAYS constexpr auto to_unsigned(integer auto x) {
  return (std::make_unsigned_t<decltype(x)>)x;
}

/// The minimum value for a particular integer type.
template <integer Int>
inline constexpr Int min_of = std::numeric_limits<Int>::min();

/// The maximum value for a particular integer type.
template <integer Int>
inline constexpr Int max_of = std::numeric_limits<Int>::max();

template <integer Int>
inline constexpr size_t bits_of = sizeof(Int) * 8;

/// Computes a "common int" type among the given integers.
///
/// This is defined to be the larges integer type among them. If any of them
/// are unsigned, the type is also unsigned.
template <integer... Ints>
using common_int = decltype(best::int_internal::common<best::types<Ints...>>());

/// Compares two integers as if they were unsigned.
BEST_INLINE_ALWAYS constexpr std::strong_ordering uint_cmp(integer auto x,
                                                           integer auto y) {
  return best::to_unsigned(x) <=> best::to_unsigned(y);
}

/// Compares two integers as if they were signed.
BEST_INLINE_ALWAYS constexpr std::strong_ordering sint_cmp(integer auto x,
                                                           integer auto y) {
  return best::to_signed(x) <=> best::to_signed(y);
}

/// Compares two integers as if they had infinite-precision.
BEST_INLINE_ALWAYS constexpr std::strong_ordering int_cmp(integer auto x,
                                                          integer auto y) {
  if constexpr (signed_int<decltype(x)> == signed_int<decltype(y)>) {
    return x <=> y;
  } else if (x < 0) {
    return std::strong_ordering::less;
  } else if (y < 0) {
    return std::strong_ordering::greater;
  } else {
    return best::uint_cmp(x, y);
  }
}

/// Casts an integer to another type, returning best::none if the cast would
/// not be exact.
template <integer Int>
BEST_INLINE_ALWAYS constexpr best::option<Int> checked_cast(integer auto x) {
  if (best::int_cmp(x, min_of<Int>) < 0 || best::int_cmp(max_of<Int>, x) < 0) {
    return {};
  }
  return x;
}
}  // namespace best

#endif  // BEST_MATH_INT_H_