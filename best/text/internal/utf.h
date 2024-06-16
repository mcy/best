#ifndef BEST_STRINGS_INTERNAL_UTF_H_
#define BEST_STRINGS_INTERNAL_UTF_H_

#include <bit>
#include <cstddef>

#include "best/container/option.h"
#include "best/container/result.h"
#include "best/container/span.h"
#include "best/math/bit.h"
#include "best/meta/guard.h"
#include "best/text/rune.h"

//! Low-level UTF encode/decode routines.

namespace best::utf_internal {
constexpr size_t size8(uint32_t rune) {
  if (rune < 0x80) {
    return 1;
  } else if (rune < 0x800) {
    return 2;
  } else if (rune < 0x10000) {
    return 3;
  } else {
    return 4;
  }
}

constexpr best::result<std::pair<size_t, uint32_t>, encoding_error> decode8(
    best::span<const char> input) {
  auto first = input.first().ok_or(encoding_error::OutOfBounds);
  BEST_GUARD(first);

  size_t bytes = 0;
  uint32_t value = static_cast<uint8_t>(*first);
  switch (std::countl_one<uint8_t>(value)) {
    case 0:
      bytes = 1;
      break;
    case 2:
      bytes = 2;
      value &= 0b000'11111;
      break;
    case 3:
      bytes = 3;
      value &= 0b0000'1111;
      break;
    case 4:
      bytes = 4;
      value &= 0b00000'111;
      break;
    default:
      return encoding_error::Invalid;
  }

  auto rest = input.at({.start = 1, .count = bytes - 1})
                  .ok_or(encoding_error::OutOfBounds);
  BEST_GUARD(rest);

  for (uint8_t c : *rest) {
    if (std::countl_one(c) != 1) return encoding_error::Invalid;

    value <<= 6;
    value |= c & 0b00'111111;
  }

  // Reject oversized encodings.
  if (bytes != size8(value)) return encoding_error::Invalid;

  return best::ok(bytes, value);
}

constexpr best::result<std::pair<size_t, uint32_t>, encoding_error> undecode8(
    best::span<const char> input) {
  size_t len = 0;
  for (; len < 4; ++len) {
    auto next =
        input.at(input.size() - len - 1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    if (best::leading_ones(*next) != 1) {
      break;
    }
  }
  if (len == 4) return encoding_error::Invalid;

  auto result = decode8(input[{.start = input.size() - len - 1}]);
  if (!result || result->first != len) return encoding_error::Invalid;
  return result;
}

constexpr best::result<size_t, encoding_error> encode8(best::span<char> output,
                                                       uint32_t rune) {
  size_t bytes = size8(rune);
  if (output.size() < bytes) return encoding_error::OutOfBounds;

  for (size_t i = bytes; i > 1; --i) {
    output[i - 1] = (rune & 0b0011'1111) | 0b1000'0000;
    rune >>= 6;
  }

  constexpr std::array<std::array<uint8_t, 2>, 4> Masks = {{
      {0b0111'1111, 0b0000'0000},
      {0b0001'1111, 0b1100'0000},
      {0b0000'1111, 0b1110'0000},
      {0b0000'0111, 0b1111'0000},
  }};

  output[0] = (rune & Masks[bytes - 1][0]) | Masks[bytes - 1][1];
  return bytes;
}

constexpr uint32_t trunc_to_10(uint32_t value) {
  return value & ((uint32_t{1} << 10) - 1);
}

inline constexpr uint32_t High = 0xd800;
inline constexpr uint32_t Low = 0xdc00;
inline constexpr uint32_t Max = 0xe000;

constexpr best::result<std::pair<size_t, uint32_t>, encoding_error> decode16(
    best::span<const char16_t> input) {
  auto hi = input.first().ok_or(encoding_error::OutOfBounds);

  if (hi < High || hi >= Max) return best::ok(1, *hi);
  if (hi >= Low) return encoding_error::Invalid;

  auto lo = input.at(1).ok_or(encoding_error::OutOfBounds);
  BEST_GUARD(lo);

  if (lo < Low || lo >= Max) return encoding_error::Invalid;

  uint32_t value = trunc_to_10(*hi) << 10 | trunc_to_10(*lo);
  return best::ok(2, value + 0x10000);
}

constexpr best::result<std::pair<size_t, uint32_t>, encoding_error> undecode16(
    best::span<const char16_t> input) {
  auto hi = input.first().ok_or(encoding_error::OutOfBounds);
  BEST_GUARD(hi);

  auto is_surrogate = *hi >= High && *hi < Max;
  auto len = is_surrogate ? 2 : 1;
  if (input.size() < len) return encoding_error::OutOfBounds;

  auto result = decode16(input[{.start = input.size() - len}]);
  if (!result || result->first != len) return encoding_error::Invalid;
  return result;
}

constexpr best::result<size_t, encoding_error> encode16(
    best::span<char16_t> output, uint32_t rune) {
  if (rune < 0x10000 && output.size() >= 1) {
    output[0] = rune;
    return 1;
  } else if (output.size() >= 2) {
    uint32_t reduced = rune - 0x10000;
    output[0] = trunc_to_10(reduced >> 10) | 0xd800;
    output[1] = trunc_to_10(reduced) | 0xdc00;
    return 2;
  }

  return encoding_error::OutOfBounds;
}
}  // namespace best::utf_internal

#endif  // BEST_STRINGS_INTERNAL_UTF_H_