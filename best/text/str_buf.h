#ifndef BEST_TEXT_STR_BUF_H_
#define BEST_TEXT_STR_BUF_H_

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "best/container/span.h"
#include "best/container/vec.h"
#include "best/meta/init.h"
#include "best/strings/encoding.h"
#include "best/strings/str.h"

//! Unicode string buffers.
//!
//! best::encoded_buf is a buffer containing encoded best::runes. This is
//! essentially best::basic_string.
//!
//! best::str_buf, best::str_buf16, and best::str_buf32 correspond to the
//! UTF-8/16/32 specializations of the above.

namespace best {
/// A buffer of textual data.
///
/// This is a generalized string buffer that allows specifying the encoding
/// of the underlying data. It is similar to std::basic_string, except it
/// uses a ztd.text-style encoding trait, and provides a generally nicer
/// interface.
///
/// A best::encoded string can be created from a string literal; in this case,
/// it will be validated for being "correctly encoded" wrt to the encoding E.
/// It can also be constructed from a pointer, in which case no such check
/// occurs.
///
/// A best::encoded may point to invalidly-encoded data. If the encoding is
/// self-synchronizing, the stream of Unicode characters is interpreted as
/// replacing each invalid code unit with a Unicode replacement character
/// (U+FFFD). If the encoding is not self-synchronizing, the stream is
/// interpreted to end at that position, with a replacement character. The
/// runes() iterator performs this decoding operation.
template <best::encoding E>
class encoded_buf final {
 public:
  /// The encoding for this string.
  using encoding = E;

  /// The code unit for this encoding. This is the element type of an encoded
  /// stream.
  using code = encoding::code;

  /// The corresponding view for this type.
  using encoded = best::encoded<E>;

  /// Creates a new, empty string.
  encoded_buf() : encoded_buf(encoded{}) {}
  explicit encoded_buf(encoding enc) : encoded_buf(encoded{std::move(enc)}) {}

  encoded_buf(const encoded_buf&) = default;
  encoded_buf& operator=(const encoded_buf&) = default;

  explicit encoded_buf(encoded str)
      : buf_(str), encoding_(str.get_encoding()) {}

  /// Creates a new string from a string literal.
  ///
  /// The array must be a constant, and it must contain validly-encoded data.
  template <size_t n>
  encoded_buf(const code (&lit)[n])
    requires best::constructible<encoding>
  BEST_ENABLE_IF_CONSTEXPR(lit)
      BEST_ENABLE_IF(best::encoder<encoding>::validate(encoding{},
                                                       best::span(lit, n - 1)),
                     "string must be validly encoded")
      : encoded_buf(encoded(lit, n - 1)) {}
  template <size_t n>
  encoded_buf(const code (&lit)[n], encoding enc) BEST_ENABLE_IF_CONSTEXPR(lit)
      BEST_ENABLE_IF_CONSTEXPR(enc) BEST_ENABLE_IF(
          best::encoder<encoding>::validate(enc, best::span(lit, n - 1)),
          "string must be validly encoded")
      : encoded_buf(encoded(lit, n - 1, std::move(enc))) {}

  /// Creates a new string from some other string type.
  template <string_type Str>
  explicit encoded_buf(const Str& that)
    requires(!std::is_array_v<Str>)
      : encoded_buf(encoding(that)) {}

  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  constexpr best::option<encoded_buf> from(best::span<const code> data,
                                           encoding enc = {}) {
    if (!best::encoder<encoding>::validate(enc, data)) {
      return best::none;
    }

    return encoded(best::in_place, data, std::move(enc));
  }

  /// Returns an encoded view over this vector.
  encoded as_span() const { return *this; }

  /// Returns the size of the string, in code units.
  size_t size() const { return as_span().size(); }

  /// Returns the string's data pointer.
  ///
  /// This value is never null.
  const code* data() const { return as_span().data(); }

  /// Returns the underlying text encoding.
  const encoding& get_encoding() const { return encoding_; }

  /// Checks whether the string is empty.
  bool is_empty() const { return size() == 0; }

  /// Gets the byte at the given index.
  ///
  /// Crashes on out-of-bounds access.
  constexpr encoded operator[](best::bounds::with_location range) const {
    return as_span()[range];
  }

  /// An iterator over the runes of a best::encoded.
  using rune_iter = encoded::rune_iter;
  constexpr rune_iter runes() const { return {*this}; }

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, encoded str) {
    if constexpr (best::same<encoding, utf8>) {
      return os << std::string_view(str.data(), str.size());
    }

    char u8[utf8::MaxCodesPerRune];
    for (rune r : str.runes()) {
      if (auto chars = best::encoder<utf8>().write_rune(u8, r)) {
        os << std::string_view(chars->data(), chars->size());
        continue;
      }
    }
  }

  bool operator==(const encoded&) const = default;
  bool operator==(const best::span<const code>& span) const {
    return as_span() == span;
  }
  template <size_t n>
  bool operator==(const code (&lit)[n]) const {
    return as_span() == best::span<const code>(lit, n - 1);
  }

 private:
  best::vec<code> buf_;
  [[no_unique_address]] encoding encoding_;

  static constexpr code empty{};
};

using str_buf = best::encoded_buf<utf8>;
using str16_buf = best::encoded_buf<utf16>;
using str32_buf = best::encoded_buf<utf32>;
}  // namespace best

#endif  // BEST_TEXT_STR_BUF_H_