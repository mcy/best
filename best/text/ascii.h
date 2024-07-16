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

#ifndef BEST_TEXT_ASCII_H_
#define BEST_TEXT_ASCII_H_

#include <cstddef>

#include "best/base/guard.h"
#include "best/container/option.h"
#include "best/memory/span.h"
#include "best/text/rune.h"

//! ASCII and other 7- and 8-bit encodings. Unlike the encodings from `utf.h`,
//! none of these encodings are universal.

namespace best {
/// # `best::ascii`
///
/// A `best::encoding` representing 7-bit ASCII.
struct ascii final {
  using code = char;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 1,
      .is_self_syncing = true,
      .is_lexicographic = true,
  };

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return idx <= input.size();
  }

  static constexpr best::result<void, encoding_error> encode(
      best::span<char>* output, rune rune) {
    if (!rune.is_ascii()) return best::encoding_error::Invalid;
    auto next = output->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    (*next.ok())[0] = rune;
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char>* input) {
    auto next = input->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0])
        .filter(&rune::is_ascii)
        .ok_or(encoding_error::Invalid);
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto next = input->take_last(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0])
        .filter(&rune::is_ascii)
        .ok_or(encoding_error::Invalid);
  }

  constexpr bool operator==(const ascii&) const = default;
};

/// # `best::latin1`
///
/// A `best::encoding` representing 8-bit "Latin1", i.e., ASCII plus the Latin1
/// Supplement.
struct latin1 final {
  using code = char;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 1,
      .is_self_syncing = true,
      .is_lexicographic = true,
  };

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return idx <= input.size();
  }

  static constexpr best::result<void, encoding_error> encode(
      best::span<char>* output, rune rune) {
    if (rune > 0xff) return best::encoding_error::Invalid;
    auto next = output->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    (*next.ok())[0] = rune;
    return best::ok();
  }

  static constexpr best::result<rune, encoding_error> decode(
      best::span<const char>* input) {
    auto next = input->take_first(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0]).ok_or(encoding_error::Invalid);
  }

  static constexpr best::result<rune, encoding_error> undecode(
      best::span<const char>* input) {
    auto next = input->take_last(1).ok_or(encoding_error::OutOfBounds);
    BEST_GUARD(next);

    return rune::from_int((*next.ok())[0]).ok_or(encoding_error::Invalid);
  }

  constexpr bool operator==(const latin1&) const = default;
};
}  // namespace best

#endif  // BEST_TEXT_ASCII_H_
