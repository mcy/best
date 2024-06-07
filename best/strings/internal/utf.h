#ifndef BEST_STRINGS_INTERNAL_UTF_H_
#define BEST_STRINGS_INTERNAL_UTF_H_

#include <bit>
#include <cstddef>

#include "best/container/option.h"
#include "best/container/span.h"

//! Low-level UTF encode/decode routines.

namespace best::utf_internal {
constexpr best::option<std::pair<size_t, uint32_t>> decode8(
    best::span<const char> input) {
  auto first = input.at(0);
  if (first.is_empty()) return best::none;

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
      return best::none;
  }

  auto rest = input.at({.start = 1, .count = bytes - 1});
  if (!rest.has_value()) return best::none;

  for (uint8_t c : *rest) {
    if (std::countl_one(c) != 1) return best::none;

    value <<= 6;
    value |= c & 0b00'111111;
  }

  return {{bytes, value}};
}

constexpr best::option<size_t> encode8(best::span<char> output, uint32_t rune) {
  size_t bytes = 0;
  if (rune < 0x80) {
    bytes = 1;
  } else if (rune < 0x800) {
    bytes = 2;
  } else if (rune < 0x10000) {
    bytes = 3;
  } else {
    bytes = 4;
  }

  if (output.size() < bytes) return false;

  for (size_t i = bytes; i > 1; --i) {
    output[i - 1] = (rune & 0xb111111) | 0b10'000000;
    rune >>= 6;
  }

  switch (bytes) {
    case 1:
      output[0] = rune & 0b0'1111111;
      break;
    case 2:
      output[0] = (rune & 0b000'11111) | 0b110'00000;
      break;
    case 3:
      output[0] = (rune & 0b0000'1111) | 0b1110'0000;
      break;
    case 4:
      output[0] = (rune & 0b00000'111) | 0b11110'000;
      break;
  }

  return bytes;
}

constexpr uint32_t trunc_to_10(uint32_t value) {
  return value & ((uint32_t{1} << 10) - 1);
}

inline constexpr uint32_t High = 0xd800;
inline constexpr uint32_t Low = 0xdc00;
inline constexpr uint32_t Max = 0xe000;

constexpr best::option<std::pair<size_t, uint32_t>> decode16(
    best::span<const char16_t> input) {
  auto hi = input.at(0);
  if (hi.is_empty()) return best::none;

  if (hi < High || hi >= Max) {
    return {{1, *hi}};
  }

  if (hi >= Low) return best::none;

  auto lo = input.at(1);
  if (lo.is_empty() || lo < Low || lo >= Max) return best::none;

  uint32_t value = trunc_to_10(*hi) << 10 | trunc_to_10(*lo);
  return {{2, value + 0x10000}};
}

constexpr best::option<size_t> encode16(best::span<char16_t> output,
                                        uint32_t rune) {
  if (rune < 0x10000 && output.size() >= 1) {
    output[0] = rune;
    return 1;
  } else if (output.size() >= 2) {
    uint32_t reduced = rune - 0x10000;
    output[0] = trunc_to_10(reduced >> 10) | 0xd800;
    output[1] = trunc_to_10(reduced) | 0xdc00;
    return 2;
  }

  return best::none;
}
}  // namespace best::utf_internal

#endif  // BEST_STRINGS_INTERNAL_UTF_H_