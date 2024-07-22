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

#ifndef BEST_TEXT_INTERNAL_UTF_H_
#define BEST_TEXT_INTERNAL_UTF_H_

#include <cstddef>

#include "best/math/bit.h"
#include "best/memory/span.h"
#include "best/text/encoding.h"

//! Low-level UTF encode/decode routines.
//!
//! These functions get pulled in by virtually any target using `best`, and
//! they're not templates, so we need them to be low on instantiations. Thus,
//! they operate almost exclusively directly on pointers.

namespace best::utf_internal {
// All functions return a rune as an int32. If the resulting value is negative,
// it is an error (neither UTF-8 nor UTF-16 can encode the full 32 bits needed
// for negative values: UTF-8 can encode at most 31).
inline constexpr int32_t OutOfBounds = ~int32_t(encoding_error::OutOfBounds);
inline constexpr int32_t Invalid = ~int32_t(encoding_error::Invalid);

constexpr bool validate_utf8_fast(const char* data, size_t len) {
  // This function is hit whenever we create a `best::str` from a literal, so
  // we need to avoid optional/span here.
  auto end = data + len;
  while (data != end) {
    uint32_t value = uint8_t(*data++);
    if (value < 0x80) {
      continue;  // ASCII fast path.
    }

    size_t bytes = best::leading_ones(uint8_t(value));
    value &= 0x7f >> bytes;

    if (bytes > 4 || (end - data) < bytes - 1) { return false; }
    while (--bytes > 0) {
      char c = data[0];
      ++data;
      if (best::leading_ones(c) != 1) { return false; }

      value <<= 6;
      value |= c & 0b00'111111;
    }

    if (value >= 0x11'0000 || (value >= 0xd800 && value <= 0xdfff)) {
      return false;
    }
  }
  return true;
}

constexpr int32_t encode8_size(uint32_t rune) {
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

constexpr int32_t decode8_size(best::span<const char> input) {
  if (input.is_empty()) { return OutOfBounds; }
  uint32_t ones = best::leading_ones(uint8_t(*input.data()));
  if (ones == 1 || ones > 4) { return Invalid; }
  return ones + (ones == 0);
}

// This function is unfortunately a build-time hot-spot, so we can't use
// nice helper functions from best::span here.
//
// This function expects the caller to pre-compute decode8_size.
constexpr int32_t decode8(const char* data, size_t rune_bytes) {
  uint32_t value = uint8_t(data[0]);

  // Fast-path for ASCII.
  if (rune_bytes == 1) { return value; }

  value &= uint8_t{-1} >> rune_bytes;
  for (size_t i = 1; i < rune_bytes; ++i) {
    char c = data[i];
    if (best::leading_ones(c) != 1) { return Invalid; }

    value <<= 6;
    value |= c & 0b00'111111;
  }

  // Reject oversized encodings.
  if (rune_bytes != encode8_size(value)) { return Invalid; }
  return value;
}

constexpr int32_t undecode8(best::span<const char>* input) {
  size_t len = 0;
  for (; len < 4; ++len) {
    if (input->size() < len) { return OutOfBounds; }
    if (best::leading_ones(input->data()[input->size() - len - 1]) != 1) {
      break;
    }
  }
  if (len == 4) { return Invalid; }
  *input = {input->data(), input->size() - len};

  return decode8(input->data() + input->size() - len, len);
}

// This function expects the caller to pre-compute encode8_size.
constexpr void encode8(char* output, uint32_t rune, size_t bytes) {
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
}

constexpr uint32_t trunc_to_u10(uint32_t value) {
  return value & ((uint32_t{1} << 10) - 1);
}

inline constexpr uint32_t High = 0xd800;
inline constexpr uint32_t Low = 0xdc00;
inline constexpr uint32_t Max = 0xe000;

constexpr bool is_high_surrogate(char16_t code) {
  return (code & 0xfc00) == High;
}
constexpr bool is_low_surrogate(char16_t code) {
  return (code & 0xfc00) == Low;
}

constexpr size_t decode16_size(best::span<const char16_t> input) {
  if (input.is_empty()) { return OutOfBounds; }
  auto value = input.data().raw()[0];
  if (is_high_surrogate(value)) { return 2; }
  if (is_low_surrogate(value)) { return Invalid; }
  return 1;
}

// This function expects the caller to pre-compute decode16_size.
constexpr int32_t decode16(const char16_t* data, size_t rune_words) {
  uint16_t hi = data[0];
  if (rune_words == 1) { return hi; }

  uint16_t lo = data[1];
  if (!is_low_surrogate(lo)) { return Invalid; }

  uint32_t value = trunc_to_u10(hi) << 10 | trunc_to_u10(lo);
  return value + 0x10000;
}

constexpr int32_t undecode16(best::span<const char16_t>* input) {
  if (input->is_empty()) { return OutOfBounds; }
  uint16_t lo = input->data()[input->size() - 1];
  if (is_high_surrogate(lo)) { return Invalid; }
  if (!is_low_surrogate(lo)) {
    *input = {input->data(), input->size() - 1};
    return lo;
  }

  if (input->size() < 2) { return OutOfBounds; }
  uint16_t hi = input->data()[input->size() - 2];

  if (!is_high_surrogate(hi)) { return Invalid; }
  *input = {input->data(), input->size() - 2};

  uint32_t value = trunc_to_u10(hi) << 10 | trunc_to_u10(lo);
  return value + 0x10000;
}

constexpr int32_t encode16(char16_t* output, size_t size, uint32_t rune) {
  if (rune < 0x10000 && size >= 1) {
    output[0] = rune;
    return 1;
  }

  if (size >= 2) {
    uint32_t reduced = rune - 0x10000;
    output[0] = trunc_to_u10(reduced >> 10) | 0xd800;
    output[1] = trunc_to_u10(reduced) | 0xdc00;
    return 2;
  }

  return OutOfBounds;
}

template <typename S, typename Code>
concept is_std_string = requires(S s, Code c) {
  typename S::traits_type;
  typename S::value_type;
  requires best::same<Code, typename S::value_type>;
  s.find_first_of(c);
  s.find_first_of(s);
};

}  // namespace best::utf_internal

#endif  // BEST_TEXT_INTERNAL_UTF_H_
