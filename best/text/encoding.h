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

#ifndef BEST_TEXT_ENCODING_H_
#define BEST_TEXT_ENCODING_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/container/result.h"
#include "best/container/span.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"

//! Unicode encodings.
//!
//! `best::encoding` encapsulates a way to encode Unicode, such as UTF-8,
//! UTF-16, WTF-8, ASCII, Latin1, EBDIC, etc. The concept of a string type is
//! defined downstream of this.

namespace best {
/// # `best::encoding`
///
/// A text encoding type. This type usually won't be used on its own; instead,
/// helpers from `best::rune` should be used instead.
///
/// A text encoding is any type that fulfills a contract in the spirit of the
/// "Lucky 7" encoding API from ztd.text.
/// <https://ztdtext.readthedocs.io/en/latest/design/lucky%207.html>
///
/// To be supported by `best`, an encoding must be:
///
/// * Stateless. Decoding any one rune may not depend on what runes came
///   before it.
/// * Reversible. At any position within a stream, assuming it is a rune
///   boundary, it is possible to decode a unique rune in reverse order, and
///   reverse decoding agrees with forward decoding.
/// * Injective. Every rune is encoded as exactly one sequence of one or more
///   code units.
/// * ISO-646 compliant. Every printable ISO 646 character is encodable.
template <typename E_, typename E = best::as_auto<E_>>
concept encoding =
    best::copyable<E> && best::equatable<E> && requires(const E& e) {
      /// Required type aliases.
      typename E::code;

      /// Required constants.
      { E::About } -> best::converts_to<best::encoding_about>;

      /// It must provide the following operations. `best::rune` provides
      /// wrappers for them, which specifies what each of these functions must
      /// do.
      requires requires(size_t idx, rune r,
                        best::span<const typename E::code>& input,
                        best::span<typename E::code>& output) {
        { e.is_boundary(input, idx) } -> same<bool>;
        { e.encode(&output, r) } -> same<result<void, encoding_error>>;
        { e.decode(&input) } -> same<result<rune, encoding_error>>;
        { e.undecode(&input) } -> same<result<rune, encoding_error>>;
      };
    };

/// # `best::encoding_error`
///
/// An error produced by an encoder.
enum class encoding_error : uint8_t {
  /// Insufficient space in the input/output buffer.
  OutOfBounds,
  /// Attempted to encode/decode a rune the encoding does not support.
  Invalid,
};

/// # `best::code`
///
/// The code unit type of a particular encoding.
template <encoding E>
using code = best::as_auto<E>::code;

/// # `best::encoding_about`
///
/// Details about an encoding. Every encoding must provide a `constexpr` member
/// named `About` of this type.
///
/// In the future, this requirement may be relaxed for an encoding to provide
/// a dynamic value for this type.
struct encoding_about final {
  /// The maximum number of codes `write_rune()` can write. Must be positive.
  size_t max_codes_per_rune = 0;

  /// Whether this encoding is self-synchronizing.
  ///
  /// A self-synchronizing encoding is one where attempting to decode a rune
  /// using a suffix of an encoded rune is detectable as an error without
  /// context.
  ///
  /// For example, x86 machine code is not self-synchronizing, because jumping
  /// into the middle of an instruction may decode a different, valid
  /// instruction. UTF-8, UTF-16, and UTF-32 are self-synchronizing.
  ///
  /// We assume that self-synchronizing encodings are stateless. It is possible
  /// to construct a non-synchronizing, stateful encoding, but those don't
  /// really occur in practice because being self-synchronizing is an extremely
  /// strong property.
  ///
  /// Many string algorithms are only available for self-synchronizing
  /// encodings. https://en.wikipedia.org/wiki/Self-synchronizing_code
  bool is_self_syncing = false;

  /// Whether encoded runes are lexicographic.
  ///
  /// An encoding has the lexicographic property if, given two sequences of
  /// runes `r1` and `r2`, and their corresponding encoded code sequences
  /// `c1` and `c2`, then `r1 <=> r2 == c1 <=> c2`, as `best::span`s.
  ///
  /// UTF-8 and UTF-32 have this property. UTF-16 does not, because runes
  /// greater than U+FFFF are encoded with a pair of surrogates, both of which
  /// start with hex digit `0xd`; this means that `U+FFFF > U+10000` when
  /// encoded as UTF-16.
  bool is_lexicographic = false;

  /// Whether this encoding can encode all of Unicode, not including the
  /// unpaired surrogates.
  ///
  /// A universal encoding's encode() function will never fail when called on
  /// a buffer at least `max_codes_per_rune` in length, unless it is passed an
  /// unpaired surrogate.
  bool is_universal = false;

  /// Whether this encoding allows encoding unpaired surrogates.
  bool allows_surrogates = false;
};

/// # `best::string_type`
///
/// A string type: a contiguous range that defines the `BestEncoding()` FTADLE
/// and whose data pointer matches that encoding.
template <typename T>
concept string_type =
    best::contiguous<T> && requires(best::ftadle& tag, const T& value) {
      { BestEncoding(tag, value) } -> best::encoding;
      {
        best::data(value)
      } -> best::same<code<decltype(BestEncoding(tag, value))> const*>;
    };

/// # `best::encoding_of()`
///
/// Extracts the encoding out of a string type.
constexpr const auto& encoding_of(const string_type auto& string) {
  return BestEncoding(best::ftadle{}, string);
}

/// # `best::encoding_type<S>`
///
/// Extracts the encoding type out of some string type.
template <string_type S>
using encoding_type = best::as_auto<decltype(best::encoding_of(best::lie<S>))>;

/// # `best::same_encoding()`
///
/// Returns whether two string values have the same encoding. This verifies that
/// their encodings compare as equal.
constexpr bool same_encoding(const string_type auto& lhs,
                             const string_type auto& rhs) {
  if constexpr (best::equatable<encoding_type<decltype(lhs)>,
                                encoding_type<decltype(rhs)>>) {
    return best::encoding_of(lhs) == best::encoding_of(rhs);
  }

  return false;
}

/// # `best::same_encoding_code()`
///
/// Returns whether two string types have the same code unit type.
template <string_type S1, string_type S2>
constexpr bool same_encoding_code() {
  return best::same<code<encoding_type<S1>>, code<encoding_type<S2>>>;
}

template <typename rune = best::rune>
void BestFmt(auto& fmt, const best::string_type auto& str) {
  // Taken liberally from Rust's implementation of Formatter::pad().

  using iter = rune::template iter<best::encoding_type<decltype(str)>>;

  if (fmt.current_spec().method == 'q' || fmt.current_spec().debug) {
    // Quoted string.
    fmt.write('"');
    for (rune r : iter(str)) {
      fmt.format("{}", r.escaped());
    }
    fmt.write('"');
    return;
  }

  const auto& spec = fmt.current_spec();
  if (spec.width == 0 && !spec.prec) {
    // Fast path.
    fmt.write(str);
    return;
  }

  iter it(str);
  auto data = it.rest();

  if (auto prec = spec.prec) {
    size_t max = *prec;
    size_t end = 0;
    for (auto r : it) {
      if (--max == 0) break;
      end += r.size(best::encoding_of(str)).ok().value_or(1);
    }
    data = data[{.end = end}];
  }

  if (spec.width == 0) {
    // No need to pad here!
    fmt.write(data, best::encoding_of(str));
    return;
  }

  // Otherwise, we need to figure out the number of characters and potentially
  // write some padding.
  size_t runes = 0;
  for (auto ignored : iter(str)) ignored, ++runes;
  if (runes >= spec.width) {
    // No need to pad here either!
    fmt.write(data, best::encoding_of(str));
    return;
  }

  auto fill = fmt.current_spec().fill;
  auto [pre, post] =
      fmt.current_spec().compute_padding(runes, fmt.current_spec().Left);
  for (size_t i = 0; i < pre; ++i) fmt.write(fill);
  fmt.write(data, best::encoding_of(str));
  for (size_t i = 0; i < post; ++i) fmt.write(fill);
}
constexpr void BestFmtQuery(auto& query, best::string_type auto*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.supports_prec = true;
  query.uses_method = [](auto r) { return r == 'q'; };
}
}  // namespace best

#endif  // BEST_TEXT_ENCODING_H_
