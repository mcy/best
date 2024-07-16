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

#ifndef BEST_TEXT_UTF8_H_
#define BEST_TEXT_UTF8_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/memory/span.h"
#include "best/text/encoding.h"
#include "best/text/internal/utf.h"
#include "best/text/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-8 and WTF-8 encodings.

namespace best {
/// # `best::utf8`
///
/// A `best::encoding` representing UTF-8.
struct utf8 final {
  using code = char;  // Not char8_t because the standard messed up.
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 4,
      .is_self_syncing = true,
      .is_lexicographic = true,
      .is_universal = true,
  };

  static constexpr bool validate(best::span<const char> input) {
    return best::utf_internal::validate_utf8_fast(input.data(), input.size());
  }

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return input.size() == idx || input.at(idx).has_value([](char c) {
      return best::leading_ones(c) != 1;
    });
  }

  static constexpr best::result<void, encoding_error> encode(
      best::span<char>* output, rune rune) {
    size_t bytes = best::utf_internal::encode8_size(rune);
    if (output->size() < bytes) return encoding_error::OutOfBounds;
    best::utf_internal::encode8(output->data(), rune, bytes);

    *output = (*output)[{.start = bytes}];
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char>* input) {
    auto bytes = best::utf_internal::decode8_size(*input);
    if (bytes < 0) return encoding_error(~bytes);

    auto code = best::utf_internal::decode8(input->data(), bytes);
    if (code < 0) return encoding_error(~code);

    // Elide a bounds check here; this shaves off milliseconds off of the
    // per-rune cost of validating a best::format_template.
    *input = best::span(input->data() + bytes, input->size() - bytes);
    if (auto r = rune::from_int(code)) return *r;
    return encoding_error::Invalid;
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto code = best::utf_internal::undecode8(input);
    if (code < 0) return encoding_error(~code);

    if (auto r = rune::from_int(code)) return *r;
    return encoding_error::Invalid;
  }

  constexpr bool operator==(const utf8&) const = default;
};

/// # `best::wtf8`
///
/// A best::encoding representing WTF-8 (Wobbly Transformation Format).
///
/// Its only difference with UTF-8 is that it allows decoded runes to be
/// unpaired surrogates (in the range U+D800 to U+DFFF).
struct wtf8 final {
  using code = char;  // Not char8_t because the standard messed up.
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 4,
      .is_self_syncing = true,
      .is_lexicographic = true,
      .is_universal = true,
      .allows_surrogates = true,
  };

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return utf8::is_boundary(input, idx);
  }

  static constexpr best::result<void, encoding_error> encode(
      best::span<char>* output, rune rune) {
    return utf8::encode(output, rune);
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char>* input) {
    auto bytes = best::utf_internal::decode8_size(*input);
    if (bytes < 0) return encoding_error(~bytes);

    auto code = best::utf_internal::decode8(input->data(), bytes);
    if (code < 0) return encoding_error(~code);

    // Elide a bounds check here; this shaves off milliseconds off of the
    // per-rune cost of validating a best::format_template.
    *input = best::span(input->data() + bytes, input->size() - bytes);
    if (auto r = rune::from_int_allow_surrogates(code)) return *r;
    return encoding_error::Invalid;
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto code = best::utf_internal::undecode8(input);
    if (code < 0) return encoding_error(~code);

    if (auto r = rune::from_int_allow_surrogates(code)) return *r;
    return encoding_error::Invalid;
  }

  constexpr bool operator==(const wtf8&) const = default;
};

constexpr const utf8& BestEncoding(
    auto, const utf_internal::is_std_string<char> auto&) {
  return best::val<utf8{}>::value;
}
template <size_t n>
constexpr const utf8& BestEncoding(auto, const char (&)[n]) {
  return best::val<utf8{}>::value;
}
}  // namespace best

#endif  // BEST_TEXT_UTF8_H_
