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

#ifndef BEST_TEXT_UTF16_H_
#define BEST_TEXT_UTF16_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/memory/span.h"
#include "best/text/encoding.h"
#include "best/text/internal/utf.h"
#include "best/text/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-16 encoding.

namespace best {
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
             .then(
               [](char16_t c) { return rune::from_int_allow_surrogates(c); })
             .has_value([](rune r) { return !r.is_low_surrogate(); });
  }

  static constexpr best::result<void, encoding_error> encode(
    best::span<char16_t>* output, rune rune) {
    auto size =
      best::utf_internal::encode16(output->data().raw(), output->size(), rune);
    if (size < 0) { return encoding_error(~size); }

    *output = (*output)[{.start = size}];
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
    best::span<const char16_t>* input) {
    auto words = best::utf_internal::decode16_size(*input);
    if (words < 0) { return encoding_error(~words); }

    auto code = best::utf_internal::decode16(input->data().raw(), words);
    if (code < 0) { return encoding_error(~code); }

    *input = (*input)[{.start = words}];
    if (auto r = rune::from_int(code)) { return *r; }
    return encoding_error::Invalid;
  }

  static constexpr best::result<rune, encoding_error> undecode(
    best::span<const char16_t>* input) {
    auto code = best::utf_internal::undecode16(input);
    if (code < 0) { return encoding_error(~code); }

    if (auto r = rune::from_int(code)) { return *r; }
    return encoding_error::Invalid;
  }

  bool operator==(const utf16&) const = default;
};

constexpr const utf16& BestEncoding(
  auto, const utf_internal::is_std_string<char16_t> auto&) {
  return best::val<utf16{}>::value;
}
template <size_t n>
constexpr const utf16& BestEncoding(auto, const char16_t (&)[n]) {
  return best::val<utf16{}>::value;
}
}  // namespace best

#endif  // BEST_TEXT_UTF16_H_
