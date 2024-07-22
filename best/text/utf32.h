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

#ifndef BEST_TEXT_UTF32_H_
#define BEST_TEXT_UTF32_H_

#include <cstddef>

#include "best/base/guard.h"
#include "best/container/option.h"
#include "best/memory/span.h"
#include "best/text/internal/utf.h"
#include "best/text/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-32 encoding.

namespace best {
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
    auto next = output->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    (*next.ok())[0] = rune;
    return best::ok();
  }

  template <int = 0>
  static constexpr best::result<rune, encoding_error> decode(
    best::span<const char32_t>* input) {
    auto next = input->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0]).ok_or(encoding_error::Invalid);
  }

  template <int = 0>
  static constexpr best::result<rune, encoding_error> undecode(
    best::span<const char32_t>* input) {
    auto next = input->take_last(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0]).ok_or(encoding_error::Invalid);
  }

  constexpr bool operator==(const utf32&) const = default;
};

constexpr const utf32& BestEncoding(
  auto, const utf_internal::is_std_string<char32_t> auto&) {
  return best::val<utf32{}>::value;
}
template <size_t n>
constexpr const utf32& BestEncoding(auto, const char32_t (&)[n]) {
  return best::val<utf32{}>::value;
}
}  // namespace best

#endif  // BEST_TEXT_UTF32_H_
