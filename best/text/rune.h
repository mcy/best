#ifndef BEST_TEXT_RUNE_H_
#define BEST_TEXT_RUNE_H_

#include <cstddef>
#include <ios>
#include <type_traits>

#include "best/base/hint.h"
#include "best/container/option.h"
#include "best/container/result.h"
#include "best/container/span.h"
#include "best/log/internal/crash.h"

//! Unicode characters and encodings.
//!
//! best::rune is a Unicode character type, specifically, a Unicode Scalar
//! Value[1]. It is the entry-point to best's Unicode library.
//!
//! [1]: https://www.unicode.org/glossary/#unicode_scalar_value

namespace best {
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

/// # `best::encoding_error`
///
/// An error produced by an encoder.
enum class encoding_error : uint8_t {
  /// Insufficient space in the input/output buffer.
  OutOfBounds,
  /// Attempted to encode/decode a rune the encoding does not support.
  Invalid,
};
}  // namespace best

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
template <typename E_, typename E = std::remove_cvref_t<E_>>
concept encoding =
    best::copyable<E> && best::equatable<E> && requires(const E& e) {
      /// Required type aliases.
      typename E::code;

      /// Required constants.
      { E::About } -> std::convertible_to<best::encoding_about>;

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

/// # `best::code`
///
/// The code unit type of a particular encoding.
template <encoding E>
using code = E::code;

/// # `best::string_type`
///
/// A string type: a contiguous range that defines the `BestEncoding()` FTADLE
/// and whose data pointer matches that encoding.
template <typename T>
concept string_type = best::contiguous<T> && requires(const T& value) {
  { BestEncoding(best::types<const T&>, value) } -> best::encoding;
  {
    std::data(value)
  } -> best::same<const typename std::remove_cvref_t<decltype(BestEncoding(
        best::types<const T&>, value))>::code*>;
};

/// # `best::encoding_of()`
///
/// Extracts the encoding out of a string type.
constexpr const auto& encoding_of(const string_type auto& string) {
  return BestEncoding(best::types<decltype(string)>, string);
}

/// # `best::encoding_type<S>`
///
/// Extracts the encoding type out of some string type.
template <string_type S>
using encoding_type =
    std::remove_cvref_t<decltype(best::encoding_of(std::declval<S>()))>;

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
  static constexpr rune replacement() { return 0xfffd; }

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
  template <encoding E = utf8>
  constexpr static bool validate(best::span<const code<E>> input,
                                 const E& enc = {}) {
    while (!input.is_empty()) {
      if (!decode(&input, enc)) return false;
    }
    return true;
  }

  /// # `rune::size()`
  ///
  /// Returns the number of code units needed to encode this rune. Returns
  /// `best::none` if this rune is not encodable with `E`.
  template <encoding E = utf8>
  constexpr best::result<size_t, encoding_error> size(const E& = {}) const;

  /// # `rune::is_boundary()`
  ///
  /// Returns whether the code unit boundary given by `idx` is also a rune
  /// boundary.
  template <encoding E = utf8>
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
  template <encoding E = utf8>
  constexpr static best::result<rune, encoding_error> decode(
      best::span<const code<E>>* input, const E& enc = {});
  template <encoding E = utf8>
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
  template <encoding E = utf8>
  constexpr static best::result<rune, encoding_error> undecode(
      best::span<const code<E>>* input, const E& enc = {});
  template <encoding E = utf8>
  constexpr static best::result<rune, encoding_error> undecode(
      best::span<const code<E>> input, const E& enc = {}) {
    return undecode(&input, enc);
  }

  /// # `rune::iter`
  ///
  /// An iterator over some encoded span that yields runes. The span need not
  /// be well-encoded: if encoding errors are encountered, then either:
  ///
  /// 1. If the encoding is synchronizing, yields one `rune::replacement()`
  /// for
  ///    each bad code unit.
  ///
  /// 2. If the encoding is not synchronizing, yields one
  /// `rune::replacement()`
  ///    and halts further iteration.
  template <encoding>
  struct iter;

  template <typename S, typename E>
  iter(S, const E&) -> iter<E>;
  template <typename S>
  iter(const S& s) -> iter<std::remove_cvref_t<decltype(best::encoding_of(s))>>;

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

  /// Tempoaray hack until BestFmt.
  template <typename Os>
  friend Os& operator<<(Os& os, rune r) {
    return os << std::hex << r.to_int();
  }

  // best::rune has a niche representation.
  constexpr rune(niche) : value_(-1) {}
  constexpr bool operator==(niche) const { return value_ == -1; }

 private:
  BEST_INLINE_ALWAYS constexpr bool in(uint32_t a, uint32_t b) const {
    return value_ >= a && value_ <= b;
  }

  static constexpr char Alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  constexpr explicit rune(best::in_place_t, uint32_t value) : value_(value) {}

  uint32_t value_;
};

/// See `rune::iter` above.
template <encoding E>
struct rune::iter final {
 public:
  /// # `iter::sentinel`
  ///
  /// The end-of-iteration sentinel.
  struct sentinel {};

  /// # `iter::iter()`
  ///
  /// Constructs a new iterator over the given span of code units.
  constexpr explicit iter(best::span<const code<E>> codes, const E& enc)
      : codes_(codes), enc_(std::addressof(enc)) {}

  /// # `iter::iter(str)`
  ///
  /// Constructs a new iterator over a string type.
  constexpr explicit iter(const best::string_type auto& str)
      : iter(str, best::encoding_of(str)) {}
  template <size_t n>
  constexpr explicit iter(const auto (&str)[n])
      : iter(best::span(str, n - 1), best::encoding_of(str)) {}

  /// # `iter::next()`
  ///
  /// Advances the iterator and returns the next value.
  constexpr best::option<rune> next() {
    ++*this;
    return next_;
  }

  /// # `iter::rest()`
  ///
  /// Returns the part of the code unit span still left to advance.
  constexpr best::span<const code<E>> rest() const { return codes_; }

  constexpr iter begin() const { return *this; }
  constexpr sentinel end() const { return {}; }

  constexpr bool operator==(sentinel) const {
    return next_.is_empty() && codes_.is_empty();
  };
  constexpr rune operator*() {
    if (!next_) ++*this;
    return next_.value_or();
  }

  constexpr iter& operator++() {
    if (codes_.is_empty()) {
      next_ = best::none;
      return *this;
    }

    next_ = decode(&codes_, *enc_).ok();
    if (next_) return *this;

    next_ = rune::replacement();
    if (E::About.is_self_syncing) {
      codes_ = codes_[{.start = 1}];
    } else {
      codes_ = best::span<code<E>>();
    }

    return *this;
  }

  constexpr iter operator++(int) {
    auto prev = *this;
    ++*this;
    return prev;
  }

 private:
  best::option<rune> next_;
  best::span<const code<E>> codes_;
  const E* enc_;
};

/// --- IMPLEMENTATION DETAILS BELOW ---
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
  return enc.encode(output, *this)
      .map([&] {
        size_t written = orig.size() - output->size();
        return orig[{.count = written}];
      })
      .map_err([&](auto e) {
        *output = orig;
        return e;
      });
}

template <encoding E>
constexpr best::result<rune, encoding_error> rune::decode(
    best::span<const code<E>>* input, const E& enc) {
  auto orig = *input;
  return enc.decode(input).map_err([&](auto e) {
    *input = orig;
    return e;
  });
}
template <encoding E>
constexpr best::result<rune, encoding_error> rune::undecode(
    best::span<const code<E>>* input, const E& enc) {
  auto orig = *input;
  return enc.undecode(input).map_err([&](auto e) {
    *input = orig;
    return e;
  });
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

}  // namespace best

#endif  // BEST_TEXT_RUNE_H_