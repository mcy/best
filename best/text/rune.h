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

#ifndef BEST_TEXT_RUNE_H_
#define BEST_TEXT_RUNE_H_

#include <cstddef>

#include "best/base/hint.h"
#include "best/container/option.h"
#include "best/container/result.h"
#include "best/log/internal/crash.h"
#include "best/memory/span.h"
#include "best/text/encoding.h"

//! Unicode characters and encodings.
//!
//! best::rune is a Unicode character type, specifically, a Unicode Scalar
//! Value[1]. It is the entry-point to best's Unicode library.
//!
//! [1]: https://www.unicode.org/glossary/#unicode_scalar_value

namespace best {
/// # `best::rune`
///
/// A Unicode scalar value, called a "rune" in the p9 tradition.
///
/// this rune corresponds to a valid Unicode scalar value, which may
/// potentially be an unpaired surrogate. This is to allow encodings that
/// allow unpaired surrogates, such as WTF-8, to produce best::runes.
class rune final {
 private:
  static constexpr bool is_unicode(uint32_t value) { return value < 0x11'0000; }
  static constexpr bool is_surrogate(uint32_t value) {
    return value >= 0xd800 && value < 0xe000;
  }

 public:
  /// # `rune::replacement()`
  ///
  /// Returns the Unicode replacement character.
  static const rune Replacement;

  /// # `rune::rune()`
  ///
  /// Creates a new rune corresponding to NUL.
  constexpr rune() = default;

  /// # `rune::rune(rune)`
  ///
  /// Trivially copyable.
  constexpr rune(const rune&) = default;
  constexpr rune& operator=(const rune&) = default;
  constexpr rune(rune&&) = default;
  constexpr rune& operator=(rune&&) = default;

  /// # `rune::rune(int)`
  ///
  /// Creates a new rune from an integer.
  ///
  /// The integer must be a constant, and it must be a valid Unicode scalar
  /// value, and *not* an unpaired surrogate.
  constexpr rune(uint32_t value) BEST_ENABLE_IF_CONSTEXPR(value)
      BEST_ENABLE_IF(is_unicode(value) && !is_surrogate(value),
                     "rune value not within the valid Unicode range")
      : value_(value) {}

  /// # `rune::from_int()`
  ///
  /// Parses a rune from an integer.
  /// Returns `best::none` if this integer is not in the Unicode scalar value
  /// range.
  constexpr static best::option<rune> from_int(uint32_t);
  constexpr static best::option<rune> from_int(int32_t);

  /// # `rune::from_int_allow_surrogates()`
  ///
  /// Like `rune::from_int()`, but allows unpaired surrogates.
  constexpr static best::option<rune> from_int_allow_surrogates(uint32_t);
  constexpr static best::option<rune> from_int_allow_surrogates(int32_t);

  /// # `rune::to_int()`
  ///
  /// Converts this rune into the underlying 32-bit integer.
  constexpr uint32_t to_int() const { return value_; }
  constexpr operator uint32_t() const { return value_; }

  /// # `rune::validate()`
  ///
  /// Validates whether a span of code units is correctly encoded per `E`.
  template <best::encoding E = best::utf8>
  constexpr static bool validate(best::span<const code<E>> input,
                                 const E& enc = {}) {
    if constexpr (requires { enc.validate(input); }) {
      return enc.validate(input);
    }

    while (!input.is_empty()) {
      if (!decode(&input, enc)) return false;
    }
    return true;
  }

  /// # `rune::size()`
  ///
  /// Returns the number of code units needed to encode this rune. Returns
  /// `best::none` if this rune is not encodable with `E`.
  template <best::encoding E = best::utf8>
  constexpr best::result<size_t, encoding_error> size(const E& = {}) const;

  /// # `rune::is_boundary()`
  ///
  /// Returns whether the code unit boundary given by `idx` is also a rune
  /// boundary.
  template <best::encoding E = best::utf8>
  constexpr static bool is_boundary(best::span<const code<E>> input, size_t idx,
                                    const E& enc = {}) {
    return enc.is_boundary(input, idx);
  }

  /// # `rune::encode()`
  ///
  /// Performs a single indivisible encoding operation.
  ///
  /// Returns the part of `output` written to. If `output` is passed by
  /// pointer rather than by value, it is automatically advanced.
  ///
  /// Returns `best::none` on failure; in this case, `output` is not advanced.
  template <encoding E = utf8>
  constexpr best::result<best::span<code<E>>, encoding_error> encode(
      best::span<code<E>>* output, const E& = {}) const;
  template <encoding E = utf8>
  constexpr best::result<best::span<code<E>>, encoding_error> encode(
      best::span<code<E>> output, const E& enc = {}) const {
    return encode(&output, enc);
  }

  /// # `rune::decode()`
  ///
  /// Performs a single indivisible decoding operation.
  ///
  /// Returns the decoded rune. If `input` is passed by pointer rather than by
  /// value, it is automatically advanced.
  ///
  /// Returns `best::none` on failure; in this case, `input` is not advanced.
  template <best::encoding E = best::utf8>
  constexpr static best::result<rune, encoding_error> decode(
      best::span<const code<E>>* input, const E& enc = {});
  template <best::encoding E = best::utf8>
  constexpr static best::result<rune, encoding_error> decode(
      best::span<const code<E>> input, const E& enc = {}) {
    return decode(&input, enc);
  }

  /// # `rune::undecode()`
  ///
  /// Performs a single indivisible decoding operation, in reverse.
  ///
  /// Returns the decoded rune. If `input` is passed by pointer rather than by
  /// value, it is automatically advanced.
  ///
  /// Returns `best::none` on failure; in this case, `input` is not advanced.
  template <best::encoding E = best::utf8>
  constexpr static best::result<rune, encoding_error> undecode(
      best::span<const code<E>>* input, const E& enc = {});
  template <best::encoding E = best::utf8>
  constexpr static best::result<rune, encoding_error> undecode(
      best::span<const code<E>> input, const E& enc = {}) {
    return undecode(&input, enc);
  }

  /// # `rune::from_digit()`
  ///
  /// Returns the appropriate character to represent `num` in the given
  /// `radix` (i.e., base). Crashes if `radix > 36`.
  constexpr static best::option<rune> from_digit(uint32_t num,
                                                 uint32_t radix = 10);

  /// # `rune::is_digit()`
  ///
  /// Returns this is a "digit", i.e., a value matching `[0-9a-zA-Z]` and
  /// is within the given `radix`. For example, if `radix` is 10, this checks
  /// for whether this character matches `[0-9]`. Crashes if `radix > 36`.
  constexpr bool is_digit(uint32_t radix = 10) const {
    return to_digit(radix).has_value();
  }

  /// # `rune::to_digit()`
  ///
  /// Returns the value of this character when interpreted as a digit in the
  /// given `radix`.
  constexpr best::option<uint32_t> to_digit(uint32_t radix = 10) const;

  /// # `rune::is_unpaired_surrogate()`
  ///
  /// Returns whether this rune is an unpaired surrogate.
  constexpr bool is_unpaired_surrogate() const { return in(0xd800, 0xdfff); }

  /// # `rune::is_low_surrogate()`
  ///
  /// Returns whether this rune is a "low" unpaired surrogate.
  constexpr bool is_low_surrogate() const { return in(0xdc00, 0xdfff); }

  /// # `rune::is_high_surrogate()`
  ///
  /// Returns whether this rune is an "high" unpaired surrogate.
  constexpr bool is_high_surrogate() const { return in(0xd800, 0xdbff); }

  /// # `rune::is_ascii()`
  ///
  /// Returns whether this rune is in the ASCII range (up to U+007F)
  constexpr bool is_ascii() const { return in(0x0000, 0x007f); }

  /// # `rune::is_ascii_alpha()`
  ///
  /// Returns whether this rune is an ASCII letter.
  constexpr bool is_ascii_alpha() const {
    return is_ascii_lower() || is_ascii_upper();
  }

  /// # `rune::is_ascii_alnum()`
  ///
  /// Returns whether this rune is an ASCII letter. or digit
  constexpr bool is_ascii_alnum() const {
    return is_ascii_alpha() || is_ascii_digit();
  }

  /// # `rune::is_ascii_control()`
  ///
  /// Returns whether this rune is an ASCII control character. This includes
  /// most whitespace, except for ' ' (U+0020).
  constexpr bool is_ascii_control() const {
    return in(0x0000, 0x001f) || value_ == 0x007f;
  }

  /// # `rune::is_ascii_digit()`
  ///
  /// Returns whether this rune is an ASCII digit.
  constexpr bool is_ascii_digit() const { return in('0', '9'); }

  /// # `rune::is_ascii_hex()`
  ///
  /// Returns whether this rune is an ASCII hexadecimal digit.
  constexpr bool is_ascii_hex() const { return is_digit(16); }

  /// # `rune::is_ascii_lower()`
  ///
  /// Returns whether this rune is an ASCII lowercase letter.
  constexpr bool is_ascii_lower() const { return in('a', 'z'); }

  /// # `rune::to_ascii_lower()`
  ///
  /// Converts this rune to its ASCII lowercase counterpart, if it is ASCII
  /// uppercase.
  constexpr rune to_ascii_lower() const {
    if (!is_ascii_upper()) return *this;
    return rune(in_place, value_ - 'A' + 'a');
  }

  /// # `rune::is_ascii_upper()`
  ///
  /// Returns whether this rune is an ASCII uppercase letter.
  constexpr bool is_ascii_upper() const { return in('A', 'Z'); }

  /// # `rune::to_ascii_upper()`
  ///
  /// Converts this rune to its ASCII uppercase counterpart, if it is ASCII
  /// lowercase.
  constexpr rune to_ascii_upper() const {
    if (!is_ascii_lower()) return *this;
    return rune(in_place, value_ - 'a' + 'A');
  }

  /// # `rune::is_ascii_punct()`
  ///
  /// Returns whether this rune is an ASCII punctuation character.
  constexpr bool is_ascii_punct() const {
    return in('!', '/') || in(':', '@') || in('[', '`') || in('{', '~');
  }

  /// # `rune::is_ascii_space()`
  ///
  /// Returns whether this rune is an ASCII whitespace character.
  constexpr bool is_ascii_space() const;

  /// # `rune::escaped()`
  ///
  /// Returns a value that, when formatted, is the value of this rune after
  /// replacing it with an appropriate C++ escape sequence, if necessary.

  constexpr auto escaped() const {
    struct x {
      rune _private;
    };
    return x{*this};
  }

  friend void BestFmt(auto& fmt, rune r) {
    // TODO: Implement padding.
    if (fmt.current_spec().method != 'q' && !fmt.current_spec().debug) {
      fmt.write(r);
      return;
    }

    // Quoted rune.
    fmt.format("'{}'", r.escaped());
  }
  constexpr friend void BestFmtQuery(auto& query, rune*) {
    query.requires_debug = false;
    query.supports_width = true;
    query.uses_method = [](rune r) { return r == 'q'; };
  }

  // best::rune has a niche representation.
  constexpr explicit rune(niche) : value_(-1) {}
  constexpr bool operator==(niche) const { return value_ == -1; }

 private:
  BEST_INLINE_ALWAYS constexpr bool in(uint32_t a, uint32_t b) const {
    return value_ >= a && value_ <= b;
  }

  static constexpr char Alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  constexpr explicit rune(best::in_place_t, uint32_t value) : value_(value) {}

  uint32_t value_;
};

inline constexpr rune rune::Replacement = 0xfffd;
}  // namespace best

/******************************************************************************/

///////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! /////////////////////

/******************************************************************************/

namespace best {
constexpr best::option<rune> rune::from_int(uint32_t value) {
  if (!is_unicode(value) || is_surrogate(value)) return best::none;
  return rune(best::in_place, value);
}
constexpr best::option<rune> rune::from_int(int32_t value) {
  return from_int(static_cast<uint32_t>(value));
}
constexpr best::option<rune> rune::from_int_allow_surrogates(uint32_t value) {
  if (!is_unicode(value)) return best::none;
  return rune(best::in_place, value);
}
constexpr best::option<rune> rune::from_int_allow_surrogates(int32_t value) {
  return from_int_allow_surrogates(static_cast<uint32_t>(value));
}

template <encoding E>
constexpr best::result<size_t, encoding_error> rune::size(const E& enc) const {
  code<E> codes[E::About.max_codes_per_rune];
  return encode(codes, enc).map([](auto sp) { return sp.size(); });
}

template <encoding E>
constexpr best::result<best::span<code<E>>, encoding_error> rune::encode(
    best::span<code<E>>* output, const E& enc) const {
  auto orig = *output;
  auto result = enc.encode(output, *this);
  if (result) {
    size_t written = orig.size() - output->size();
    return orig[{.count = written}];
  }
  *output = orig;
  return *result.err();
}

template <encoding E>
constexpr best::result<rune, encoding_error> rune::decode(
    best::span<const code<E>>* input, const E& enc) {
  auto orig = *input;
  auto result = enc.decode(input);
  if (result.err()) {
    *input = orig;
  }
  return result;
}
template <encoding E>
constexpr best::result<rune, encoding_error> rune::undecode(
    best::span<const code<E>>* input, const E& enc) {
  auto orig = *input;
  auto result = enc.undecode(input);
  if (result.err()) {
    *input = orig;
  }
  return result;
}

constexpr best::option<rune> rune::from_digit(uint32_t num, uint32_t radix) {
  if (radix > 36) {
    crash_internal::crash("from_digit() radix too large: %u > 36", radix);
  }
  if (num >= radix) return best::none;

  return rune{in_place, best::to_unsigned(Alphabet[num])};
}

constexpr best::option<uint32_t> rune::to_digit(uint32_t radix) const {
  if (radix > 36) {
    crash_internal::crash("from_digit() radix too large: %u > 36", radix);
  }
  uint32_t value;
  if (is_ascii_digit()) {
    value = value_ - '0';
  } else if (is_ascii_alpha()) {
    value = to_ascii_lower().value_ - 'a' + 10;
  } else {
    return best::none;
  }

  if (value >= radix) return best::none;
  return value;
}

constexpr bool rune::is_ascii_space() const {
  switch (value_) {
    case ' ':
    case '\t':
    case '\n':
    case '\f':
    case '\r':
      return true;
    default:
      return false;
  }
}

void BestFmt(auto& fmt, const decltype(rune().escaped())& esc) {
  switch (esc._private) {
    case '\'':
      fmt.write("\\\'");
      return;
    case '"':
      fmt.write("\\\"");
      return;
    case '\\':
      fmt.write("\\\\");
      return;
    case '\0':
      fmt.write("\\0");
      return;
    case '\a':
      fmt.write("\\a");
      return;
    case '\b':
      fmt.write("\\b");
      return;
    case '\f':
      fmt.write("\\f");
      return;
    case '\n':
      fmt.write("\\n");
      return;
    case '\r':
      fmt.write("\\r");
      return;
    case '\t':
      fmt.write("\\t");
      return;
    case '\v':
      fmt.write("\\v");
      return;
  }

  if (esc._private.is_ascii_control()) {  // TODO: unicode is_control
    fmt.format("\\x{:02}", esc._private.to_int());
  } else if (esc._private == 0x200d) {
    // Handle the ZWJ explicitly for now, since it appears in some of our tests.
    fmt.write("\\u200D");
  } else {
    fmt.write(esc._private);
  }
}
constexpr void BestFmtQuery(auto& query, decltype(rune().escaped())*) {
  query.requires_debug = false;
  query.supports_width = true;
}
}  // namespace best

#endif  // BEST_TEXT_RUNE_H_
