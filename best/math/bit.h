#ifndef BEST_MATH_BIT_H_
#define BEST_MATH_BIT_H_

#include <stddef.h>
#include <stdint.h>

#include <bit>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/math/int.h"
#include "best/math/overflow.h"

//! Bit manipulation operations.
//!
//! Essentially, this is the <bit> header with more bit tricks and cleaner
//! names.

namespace best {
/// Returns the number of zeros in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t count_zeros(integer auto x) {
  return std::popcount(~to_unsigned(x));
}

/// Returns the number of ones in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t count_ones(integer auto x) {
  return std::popcount(to_unsigned(x));
}

/// Returns the number of leading zeros in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t leading_zeros(integer auto x) {
  return std::countl_zero(to_unsigned(x));
}

/// Returns the number of leading ones in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t leading_ones(integer auto x) {
  return std::countl_one(to_unsigned(x));
}

/// Returns the number of trailing zeros in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t trailing_zeros(integer auto x) {
  return std::countr_zero(to_unsigned(x));
}

/// Returns the number of trailing ones in the binary representation of x.
BEST_INLINE_ALWAYS constexpr uint32_t trailing_ones(integer auto x) {
  return std::countr_one(to_unsigned(x));
}

/// Shifts x's bits to the left, shifting in zeros, regardless of the sign of x.
/// This is just logical shift, for the benefit of readers.
///
/// Unlike ordinary shift, if `shamt` is outside of the range
// {.start = 0, .end = bits_of<int>}, this operations saturates and returns
// zero.
template <integer Int>
BEST_INLINE_ALWAYS constexpr Int shift_left(Int x, uint32_t shamt) {
  auto mask = bits_of<Int> - 1;
  if (shamt != (shamt & mask)) return 0;

  return best::to_unsigned(x) << shamt;
}

/// Shifts x's bits to the right, shifting in zeros, regardless of the sign of
/// x. This is just logical shift, for the benefit of readers.
///
/// Unlike ordinary shift, if `shamt` is outside of the range
// {.start = 0, .end = bits_of<int>}, this operations saturates and returns
// zero.
template <integer Int>
BEST_INLINE_ALWAYS constexpr Int shift_right(Int x, uint32_t shamt) {
  auto mask = bits_of<Int> - 1;
  if (shamt != (shamt & mask)) return 0;

  return best::to_unsigned(x) >> shamt;
}

/// Shifts x's bits to the right, shifting in zeros, regardless of the sign of
/// x. This is just arithmetic shift, for the benefit of readers.
///
/// Unlike ordinary shift, if `shamt` is outside of the range
// {.start = 0, .end = bits_of<int>}, this operations saturates and returns
// zero or all-ones, depending on the sign of x.
template <integer Int>
BEST_INLINE_ALWAYS constexpr Int shift_sign(Int x, uint32_t shamt) {
  auto mask = bits_of<Int> - 1;
  if (shamt != (shamt & mask)) return -(x < 0);

  return best::to_signed(x) >> shamt;
}
/// Rotates x's bits to the left (towards high-order bits) by shamt bits.
template <integer Int>
BEST_INLINE_ALWAYS constexpr Int rotate_left(Int x, uint32_t shamt) {
  return std::rotl(x, shamt);
}

/// Rotates x's bits to the right (towards low-order bits) by shamt bits.
template <integer Int>
BEST_INLINE_ALWAYS constexpr Int rotate_right(Int x, uint32_t shamt) {
  return std::rotr(x, shamt);
}

/// Returns whether x is a power of 2.
BEST_INLINE_ALWAYS constexpr bool is_pow2(unsigned_int auto x) {
  return x > 0 && best::count_ones(x) == 1;
}

/// Returns one less than the next positive power of two.
///
/// Unlike best::next_pow2, this function cannot overflow.
template <unsigned_int Int>
BEST_INLINE_ALWAYS constexpr Int next_pow2_minus1(Int x) {
  if (x == 0) return 0;
  return best::shift_right(best::max_of<Int>, best::leading_zeros(x));
}

/// Returns the next positive power of two, or zero on overflow.
template <unsigned_int Int>
BEST_INLINE_ALWAYS constexpr Int wrapping_next_pow2(Int x) {
  return (best::next_pow2_minus1(x) + best::overflow(1)).wrap();
}

/// Returns the next positive power of two; returns best::none on overflow.
template <unsigned_int Int>
BEST_INLINE_ALWAYS constexpr best::option<Int> checked_next_pow2(Int x) {
  return (best::next_pow2_minus1(x) + best::overflow(1)).checked();
}

/// Returns the next positive power of two; returns crashes on overflow.
template <unsigned_int Int>
BEST_INLINE_ALWAYS constexpr Int next_pow2(Int x) {
  return (best::next_pow2_minus1(x) + best::overflow(1)).strict();
}

/// Computes the number of bits needed to store `x`.
///
/// best::bits_for(0) == 0.
template <unsigned_int Int>
BEST_INLINE_ALWAYS constexpr uint32_t bits_for(Int x) {
  if (x == 0) return 0;
  return best::trailing_zeros(best::wrapping_next_pow2(x));
}
}  // namespace best

#endif  // BEST_MATH_BIT_H_