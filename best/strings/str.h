#ifndef BEST_STRINGS_STR_H_
#define BEST_STRINGS_STR_H_

#include <cstddef>
#include <iterator>
#include <string_view>
#include <type_traits>

#include "best/container/span.h"
#include "best/meta/init.h"
#include "best/strings/encoding.h"
#include "best/strings/utf.h"

//! Unicode strings.
//!
//! best::encoded is a Unicode string, i.e., an encoded sequence of best::runes.
//! It is essentially std::basic_string_view with a nicer API (compare with
//! best::span).
//!
//! best::encoded_buf (NYI) is a buffer that backs a best::encoded. It
//! corresponds to std::basic_string.
//!
//! best::str and best::str_buf are type aliases corresponding to the UTF-8
//! specializations of the above.

namespace best {
/// An reference to contiguous textual data.
///
/// This is a generalized view that allows specifying the encoding of the
/// underlying data. It is similar to std::basic_string_view, except it uses
/// a ztd.text-style encoding trait, and provides a generally nicer interface.
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
class encoded final {
 public:
  /// The encoding for this string.
  using encoding = E;

  /// The code unit for this encoding. This is the element type of an encoded
  /// stream.
  using code = encoding::code;

  /// Creates a new, empty string.
  constexpr encoded() : encoded(nullptr, 0, encoding{}) {}
  constexpr encoded(std::nullptr_t)
    requires best::constructible<encoding>
      : encoded(nullptr, 0, encoding{}) {}
  constexpr explicit encoded(encoding enc)
      : encoded(nullptr, 0, std::move(enc)) {}
  constexpr encoded(std::nullptr_t, encoding enc)
      : encoded(nullptr, 0, std::move(enc)) {}

  constexpr encoded(const encoded&) = default;
  constexpr encoded& operator=(const encoded&) = default;

  /// Creates a new string from a string literal.
  ///
  /// The array must be a constant, and it must contain validly-encoded data.
  template <size_t n>
  constexpr encoded(const code (&lit)[n])
    requires best::constructible<encoding>
  BEST_ENABLE_IF_CONSTEXPR(lit)
      BEST_ENABLE_IF(best::encoder<encoding>::validate(encoding{},
                                                       best::span(lit, n - 1)),
                     "string must be validly encoded")
      : encoded(lit, n - 1) {}

  template <size_t n>
  constexpr encoded(const code (&lit)[n], encoding enc)
      BEST_ENABLE_IF_CONSTEXPR(lit) BEST_ENABLE_IF_CONSTEXPR(enc)
          BEST_ENABLE_IF(
              best::encoder<encoding>::validate(enc, best::span(lit, n - 1)),
              "string must be validly encoded")
      : encoded(lit, n - 1, std::move(enc)) {}

  /// Creates a new string from a possibly-null, NUL-terminated pointer.
  constexpr explicit encoded(const code* data)
    requires best::constructible<encoding>
      : encoded(data, encoding()) {}
  constexpr explicit encoded(const code* data, encoding enc)
    requires best::constructible<encoding>
      : encoded(std::move(enc)) {
    if (data != nullptr) {
      auto ptr = data;
      while (*ptr++)
        ;

      *this = encoded(data, ptr - data - 1, std::move(encoding_));
    }
  }

  /// Creates a new string code the given data and length.
  ///
  /// Crashes if the data contains invalid encoding.
  constexpr encoded(const code* data, size_t size)
    requires best::constructible<encoding>
      : encoded(data, size, encoding()) {}
  constexpr encoded(const code* data, size_t size, encoding enc)
      : span_{data == nullptr ? &empty : data, size},
        encoding_{std::move(enc)} {}

  /// Creates a new string from some other string type.
  template <string_type Str>
  constexpr encoded(const Str& that)
    requires(!std::is_array_v<Str> && !std::is_pointer_v<Str>)
      : encoded(std::data(that), std::size(that), best::get_encoding(that)) {}

  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  constexpr best::option<encoded> from(best::span<const code> data,
                                       encoding enc = {}) {
    if (!best::encoder<encoding>::validate(enc, data)) {
      return best::none;
    }

    return encoded(best::in_place, data, std::move(enc));
  }

  /// Returns the size of the string, in code units.
  constexpr size_t size() const { return span_.size(); }

  /// Returns the string's data pointer.
  ///
  /// This value is never null.
  constexpr const code* data() const { return span_.data(); }

  /// Returns the underlying text encoding.
  constexpr const encoding& get_encoding() const { return encoding_; }

  /// Checks whether the string is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// Gets the byte at the given index.
  ///
  /// Crashes on out-of-bounds access.
  constexpr encoded operator[](best::bounds::with_location range) const {
    return span_[range];
  }

  /// An iterator over the runes of a best::encoded.
  struct rune_iter;
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
    return span_ == span;
  }
  template <size_t n>
  bool operator==(const code (&lit)[n]) const {
    return span_ == best::span<const code>(lit, n - 1);
  }

 private:
  constexpr explicit encoded(best::in_place_t, best::span<const code> span,
                             encoding enc)
      : span_(span), encoding_(std::move(enc)) {}

  best::span<const code> span_{&empty, 0};
  [[no_unique_address]] encoding encoding_;

  static constexpr code empty{};
};

template <encoding E>
struct encoded<E>::rune_iter final {
 public:
  constexpr bool operator==(const rune_iter& it) const = default;
  constexpr rune operator*() const { return next_.value_or(); }

  constexpr rune_iter& operator++() {
    if (str_.is_empty()) {
      next_ = best::none;
      return *this;
    }

    next_ = state_.read_rune(&str_);
    if (!next_.has_value()) {
      next_ = rune::replacement();
      if (best::self_syncing_encoding<E>) {
        str_ = str_[{.start = 1}];
      } else {
        str_ = {nullptr, 0};
      }
    }

    return *this;
  }

  constexpr rune_iter operator++(int) {
    auto prev = *this;
    ++*this;
    return prev;
  }

  constexpr rune_iter begin() const { return *this; }
  constexpr rune_iter end() const { return rune_iter(state_); }

 private:
  friend encoded;
  rune_iter(encoded str) : str_(str), state_(str.get_encoding()) { ++*this; }
  rune_iter(best::encoder<E> enc) : state_(std::move(enc)) {}

  best::option<rune> next_;
  best::span<const code> str_;
  best::encoder<E> state_;
};

using str = best::encoded<utf8>;
using str16 = best::encoded<utf16>;
using str32 = best::encoded<utf32>;
}  // namespace best

#endif  // BEST_STRINGS_STR_H_