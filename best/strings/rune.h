#ifndef BEST_STRINGS_RUNE_H_
#define BEST_STRINGS_RUNE_H_

#include <cstddef>
#include <ios>

#include "best/container/option.h"
#include "best/container/span.h"
#include "best/strings/encoding.h"
#include "best/strings/internal/utf.h"

//! Unicode characters.
//!
//! best::rune is a Unicode character type, specifically, a Unicode Scalar
//! Value[1]. It is the entry-point to best's Unicode library.
//!
//! [1]: https://www.unicode.org/glossary/#unicode_scalar_value

namespace best {
/// A Unicode scalar value, called a "rune" in the p9 tradition.
///
/// This value corresponds to a valid Unicode scalar value, which may
/// potentially be an unpaired surrogate. This is to allow encodings that allow
/// unpaired surrogates, such as WTF-8, to produce best::runes.
class rune final {
 private:
  static constexpr bool in_range(uint32_t value) { return value < 0x11'0000; }
  static constexpr bool is_surrogate(uint32_t value) {
    return value >= 0xd800 && value < 0xe000;
  }

 public:
  /// Returns the Unicode replacement character.
  static constexpr rune replacement() { return 0xfffd; }

  /// Creates a new rune corresponding to NUL.
  constexpr rune() = default;

  constexpr rune(const rune&) = default;
  constexpr rune& operator=(const rune&) = default;
  constexpr rune(rune&&) = default;
  constexpr rune& operator=(rune&&) = default;

  // best::rune has a niche representation.
  constexpr rune(niche) : value_(0x11'0000) {}

  /// Creates a new rune from an integer.
  ///
  /// The integer must be a constant, and it must be a valid Unicode scalar
  /// value, and *not* an unpaired surrogate.
  constexpr rune(uint32_t value) BEST_ENABLE_IF_CONSTEXPR(value)
      BEST_ENABLE_IF(in_range(value) && !is_surrogate(value),
                     "rune value not within the valid Unicode range")
      : value_(value) {}

  /// Parses a rune from an integer.
  ///
  /// Returns none if this integer is not in the Unicode scalar value range.
  constexpr static best::option<rune> from_int(uint32_t value) {
    if (!in_range(value) || is_surrogate(value)) {
      return best::none;
    }
    return rune(best::in_place, value);
  }
  constexpr static best::option<rune> from_int(int32_t value) {
    return from_int(static_cast<uint32_t>(value));
  }

  /// Like from_int(), but allows unpaired surrogates.
  constexpr static best::option<rune> from_int_allow_surrogates(
      uint32_t value) {
    if (!in_range(value)) {
      return best::none;
    }
    return rune(best::in_place, value);
  }
  constexpr static best::option<rune> from_int_allow_surrogates(int32_t value) {
    return from_int_allow_surrogates(static_cast<uint32_t>(value));
  }

  /// Converts this rune into the underlying integer.
  constexpr uint32_t to_int() const { return value_; }
  constexpr operator uint32_t() const { return value_; }

  /// Returns whether this rune is an unpaired surrogate.
  constexpr bool is_unpaired_surrogate() const { return is_surrogate(value_); }

  /// Returns whether this rune is a "low" unpaired surrogate.
  constexpr bool is_low_surrogate() const {
    return is_unpaired_surrogate() && value_ < 0xdc00;
  }

  /// Returns whether this rune is an "high" unpaired surrogate.
  constexpr bool is_high_surrogate() const {
    return is_unpaired_surrogate() && value_ >= 0xdc00;
  }

  /// Returns the size of this rune in the given encoding.
  template <stateless_encoding E>
  constexpr size_t size() const {
    return E::size(*this);
  }

  /// Attempts to decode a single rune from a span of bytes, using the given
  /// encoding.
  ///
  /// Returns best::none if decoding fails; advances `span` past the read
  /// bytes.
  template <stateless_encoding E>
  constexpr static best::option<best::rune> decode(
      best::span<const typename E::code>& span)
    requires best::constructible<encoder<E>>
  {
    return encoder<E>().decode_one(span);
  }

  /// Attempts to encode a single rune to a span of bytes, using the given
  /// encoding.
  ///
  /// Returns best::none if encoding fails; advances `span` past the written
  /// bytes.
  template <stateless_encoding E>
  constexpr best::option<best::span<typename E::code_unit>> encode(
      best::span<typename E::code_unit>& span)
    requires best::constructible<encoder<E>>
  {
    return encoder<E>().encode_one(*this);
  }

  constexpr bool operator==(niche) const { return value_ == 0x11'0000; }

  template <typename Os>
  friend Os& operator<<(Os& os, rune r) {
    char encoded[5] = {};
    best::utf_internal::encode8(encoded, r);

    return os << encoded << "/" << std::hex << r.to_int();
  }

 private:
  constexpr explicit rune(best::in_place_t, uint32_t value) : value_(value) {}

  uint32_t value_;
};
}  // namespace best

#endif  // BEST_STRINGS_RUNE_H_