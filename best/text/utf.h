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

#ifndef BEST_TEXT_UTF_H_
#define BEST_TEXT_UTF_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/container/span.h"
#include "best/text/internal/utf.h"
#include "best/text/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-8, UTF-16, UTF-32, and WTF-8 encodings.

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
    auto result = best::utf_internal::encode8(*output, rune);
    BEST_GUARD(result);

    *output = (*output)[{.start = *result.ok()}];
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char>* input) {
    // This function is unfortunately a build-time hot-spot, so we can't use
    // nice helper functions from best::option here.
    auto result = best::utf_internal::decode8(*input);
    if (!result) return *result.err();
    auto [bytes, code] = *result.ok();
    // Elide a bounds check here; this shaves off milliseconds off of the
    // per-rune cost of validating a best::format_template.
    *input = best::span(input->data() + bytes, input->size() - bytes);
    return rune::from_int(code).ok_or(encoding_error::Invalid);
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto result = best::utf_internal::undecode8(*input);
    BEST_GUARD(result);
    auto [bytes, code] = *result.ok();

    *input = (*input)[{.end = input->size() - bytes}];
    return rune::from_int(code).ok_or(encoding_error::Invalid);
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
    auto result = best::utf_internal::decode8(*input);
    BEST_GUARD(result);
    auto [bytes, code] = *result.ok();

    *input = (*input)[{.start = bytes}];
    return rune::from_int_allow_surrogates(code).ok_or(encoding_error::Invalid);
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto result = best::utf_internal::undecode8(*input);
    BEST_GUARD(result);
    auto [bytes, code] = *result.ok();

    *input = (*input)[{.end = input->size() - bytes}];
    return rune::from_int_allow_surrogates(code).ok_or(encoding_error::Invalid);
  }

  constexpr bool operator==(const wtf8&) const = default;
};

/// # `best::utf16`
///
/// A best::encoding representing UTF-16.
struct utf16 final {
  using code = char16_t;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 2,
      .is_self_syncing = true,
      .is_universal = true,
  };

  static constexpr bool is_boundary(best::span<const char16_t> input,
                                    size_t idx) {
    return input.size() == idx ||
           input.at(idx)
               .then([](char16_t c) {
                 return rune::from_int_allow_surrogates(c);
               })
               .has_value([](rune r) { return !r.is_low_surrogate(); });
  }

  static constexpr best::result<void, encoding_error> encode(
      best::span<char16_t>* output, rune rune) {
    auto result = best::utf_internal::encode16(*output, rune);
    BEST_GUARD(result);

    *output = (*output)[{.start = *result.ok()}];
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char16_t>* input) {
    auto result = best::utf_internal::decode16(*input);
    BEST_GUARD(result);
    auto [bytes, code] = *result.ok();

    *input = (*input)[{.start = bytes}];
    return rune::from_int(code).ok_or(encoding_error::Invalid);
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char16_t>* input) {
    auto result = best::utf_internal::undecode16(*input);
    BEST_GUARD(result);
    auto [bytes, code] = *result.ok();

    *input = (*input)[{.end = input->size() - bytes}];
    return rune::from_int(code).ok_or(encoding_error::Invalid);
  }

  bool operator==(const utf16&) const = default;
};

/// # `best::utf32`
///
/// A best::encoding representing UTF-32.
struct utf32 final {
  using code = char32_t;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 1,
      .is_self_syncing = true,
      .is_lexicographic = true,
      .is_universal = true,
  };

  // Make all of these functions have delayed instantiation. Virtually no code
  // uses utf32, but this is included in every file, and that's a significant
  // flat compilation cost.

  template <int = 0>
  static constexpr bool is_boundary(best::span<const char32_t> input,
                                    size_t idx) {
    return idx <= input.size();
  }

  template <int = 0>
  static constexpr best::result<void, encoding_error> encode(
      best::span<char32_t>* output, rune rune) {
    auto next = output->take_first(1);
    BEST_GUARD(next.ok_or(encoding_error::OutOfBounds));

    (*next)[0] = rune;
    return best::ok();
  }

  template <int = 0>
  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char32_t>* input) {
    auto next = input->take_first(1);
    BEST_GUARD(next.ok_or(encoding_error::OutOfBounds));

    return rune::from_int((*next)[0]).ok_or(encoding_error::Invalid);
  }

  template <int = 0>
  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char32_t>* input) {
    auto next = input->take_last(1);
    BEST_GUARD(next.ok_or(encoding_error::OutOfBounds));

    return rune::from_int((*next)[0]).ok_or(encoding_error::Invalid);
  }

  constexpr bool operator==(const utf32&) const = default;
};

static_assert(encoding<utf8>);
static_assert(encoding<wtf8>);
static_assert(encoding<utf16>);

constexpr const utf8& BestEncoding(
    auto, const utf_internal::is_std_string<char> auto&) {
  return best::val<utf8{}>::value;
}
template <size_t n>
constexpr const utf8& BestEncoding(auto, const char (&)[n]) {
  return best::val<utf8{}>::value;
}

constexpr const utf16& BestEncoding(
    auto, const utf_internal::is_std_string<char16_t> auto&) {
  return best::val<utf16{}>::value;
}
template <size_t n>
constexpr const utf16& BestEncoding(auto, const char16_t (&)[n]) {
  return best::val<utf16{}>::value;
}

constexpr const utf32& BestEncoding(
    auto, const utf_internal::is_std_string<char32_t> auto&) {
  return best::val<utf32{}>::value;
}
template <size_t n>
constexpr const utf32& BestEncoding(auto, const char32_t (&)[n]) {
  return best::val<utf32{}>::value;
}

}  // namespace best

#endif  // BEST_TEXT_UTF_H_
