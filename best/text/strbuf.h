#ifndef BEST_TEXT_STRBUF_H_
#define BEST_TEXT_STRBUF_H_

#include <cstddef>

#include "best/container/span.h"
#include "best/container/vec.h"
#include "best/memory/allocator.h"
#include "best/text/rune.h"
#include "best/text/str.h"
#include "best/text/utf.h"

//! Unicode string buffers.
//!
//! `best::textbuf` is to `best::text` as `std::string` is to
//! `std::string_view`. It is a growable array of code units with support for
//! SSO and custom allocators.
//!
//! `best::strbuf`, `best::strbuf16`, and `best::strbuf32` are type aliases
//! corresponding to the UTF-8/16/32 specializations of the above.

namespace best {
template <best::encoding, best::allocator>
class textbuf;

/// # `best::str`
///
/// A reference to UTF-8 text data.
using strbuf = best::textbuf<utf8, best::malloc>;

/// # `best::str16`
///
/// A reference to UTF-16 text data.
using strbuf16 = best::textbuf<utf16, best::malloc>;

/// # `best::str32`
///
/// A reference to UTF-32 text data.
using strbuf32 = best::textbuf<utf32, best::malloc>;

/// # `best::text`
///
/// An reference to contiguous textual data.
///
/// This is a generalized view that allows specifying the encoding of the
/// underlying data. It is similar to `std::basic_string_view`, except it uses
/// a ztd.text-style encoding trait, and provides a generally nicer interface.
///
/// A `best::text` string can be created from a string literal; in this case,
/// it will be validated for being "correctly text" wrt to the encoding `E`.
/// It can also be constructed from a pointer, in which case no such check
/// occurs.
///
/// A `best::text` may not point to invalidly-text data. Constructors from
/// unauthenticated strings must go through factories that return
/// `best::optional`.
template <best::encoding E, best::allocator A = best::malloc>
class textbuf final {
 public:
  /// # `textbuf::encoding`
  ///
  /// The encoding for this string.
  using encoding = E;

  /// # `textbuf::code`
  ///
  /// The code unit for this encoding. This is the element type of an text
  /// stream.
  using code = encoding::code;

  /// # `textbuf::About`
  ///
  /// Metadata about this string's encoding.
  static constexpr best::encoding_about About = E::About;

  /// # `textbuf::alloc`
  ///
  /// This string's allocator type.
  using alloc = A;

  /// # `textbuf::view`
  ///
  /// The corresponding view type for this `textbuf`.
  using text = best::text<encoding>;

  /// # `textbuf::vec`
  ///
  /// The corresponding raw vector type for this `textbuf`.
  using buf = best::vec<code, best::vec_inline_default<code>(), alloc>;

  /// # `textbuf::textbuf()`
  ///
  /// Creates a new, empty string with the given encoding and/or allocator.
  textbuf() : textbuf(encoding{}) {}
  explicit textbuf(encoding enc, alloc alloc = {})
      : buf_(std::move(alloc)), enc_(std::move(enc)) {}

  /// # `textbuf::textbuf(textbuf)`
  ///
  /// Copyable and movable.
  textbuf(const textbuf&) = default;
  textbuf& operator=(const textbuf&) = default;
  textbuf(textbuf&&) = default;
  textbuf& operator=(textbuf&&) = default;

  /// # `textbuf::textbuf(text)`
  ///
  /// Creates a new `textbuf` by copying from a corresponding `text`.
  explicit textbuf(best::text<encoding> str)
      : buf_(alloc{}, str), enc_(str.enc()) {}
  textbuf(alloc alloc, best::text<encoding> str)
      : buf_(std::move(alloc), str), enc_(str.enc()) {}

  /// # `textbuf::textbuf("...")`
  ///
  /// Creates a new string from a string literal with an optional encoding.
  /// The array must be a constant, and it must contain validly-e\ncoded data.
  template <size_t n>
  textbuf(const code (&lit)[n], encoding enc) BEST_IS_VALID_LITERAL(lit, enc)
      : textbuf(in_place, buf(span(lit, n - 1)), std::move(enc)) {}
  template <size_t n>
  textbuf(const code (&lit)[n]) BEST_IS_VALID_LITERAL(lit, encoding{})
      : textbuf(in_place, buf(span(lit, n - 1)), encoding{}) {}
  template <size_t n>
  textbuf(alloc alloc, const code (&lit)[n], encoding enc)
      BEST_IS_VALID_LITERAL(lit, enc)
      : textbuf(in_place, buf(std::move(alloc), span(lit, n - 1)),
                std::move(enc)) {}
  template <size_t n>
  textbuf(alloc alloc, const code (&lit)[n])
      BEST_IS_VALID_LITERAL(lit, encoding{})
      : textbuf(in_place, buf(std::move(alloc), span(lit, n - 1)), encoding{}) {
  }

  /// # `textbuf::text(unsafe)`
  ///
  /// Creates a new string from some other `best::string_type`.
  ///
  /// Crashes if the string is not correctly textbuf::
  explicit textbuf(unsafe, buf buf, encoding enc = {})
      : textbuf(in_place, data, std::move(enc)) {}

  /// # `textbuf::from()`
  ///
  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  static best::option<textbuf> from(best::span<const code> data,
                                    encoding enc = {}) {
    return from({}, data, std::move(enc));
  }
  static best::option<textbuf> from(alloc alloc, best::span<const code> data,
                                    encoding enc = {});
  static best::option<textbuf> from(buf data, encoding enc = {});
  static best::option<textbuf> from(const string_type auto& that) {
    return from(alloc{}, that);
  }
  static best::option<textbuf> from(alloc alloc, const string_type auto& that) {
    return from(std::move(alloc), span(best::data(that), best::size(that)),
                best::encoding_of(that));
  }

  /// # `textbuf::from_nul()`
  ///
  /// Creates a new string by parsing it from a NUL-terminated string. It must
  /// end in `code{0}`. If `data == `nullptr`, returns an empty string.
  static best::option<textbuf> from_nul(const code* data, encoding enc = {}) {
    return from_nul({}, data, std::move(enc));
  }
  static best::option<textbuf> from_nul(alloc alloc, const code* data,
                                        encoding enc = {}) {
    return from(std::move(alloc), best::span<const code>::from_nul(data),
                std::move(enc));
  }

  /// # `textbuf::transcode()`
  ///
  /// Creates a new string by transcoding from a different encoding. Returns
  /// `none` if `that` contains runes that this string's encoding cannot
  /// represent.
  static best::option<textbuf> transcode(const string_type auto& that) {
    return transcode(alloc{}, that);
  }
  static best::option<textbuf> transcode(alloc alloc,
                                         const string_type auto& that) {
    textbuf out(alloc, that.enc());
    if (!out.append(that)) return best::none;
    return out;
  }

  /// # `textbuf::data()`
  ///
  /// Returns the string's data pointer.
  /// This value is never null.
  const code* data() const { return buf_.data(); }
  code* data() { return buf_.data(); }

  /// # `textbuf::size()`
  ///
  /// Returns the size of the string, in code units.
  size_t size() const { return buf_.size(); }

  /// # `textbuf::is_empty()`
  ///
  /// Checks whether the string is empty.
  bool is_empty() const { return size() == 0; }

  /// # `textbuf::size()`
  ///
  /// Returns this strings's capacity (the number of code units it can hold
  /// before being forced to resize).
  size_t capacity() const { return buf_.capacity(); }

  /// # `textbuf::allocator()`
  ///
  /// Returns a reference to his vector's allocator.
  best::as_ref<const alloc> allocator() const { return buf_.alloc(); }
  best::as_ref<alloc> allocator() { return buf_.alloc(); }

  /// # `textbuf::enc()`
  ///
  /// Returns the underlying text encoding.
  const encoding& enc() const { return enc_; }

  /// # `textbuf::as_text()`
  ///
  /// Returns the span of code units that backs this string. This is also
  /// an implicit conversion.
  text as_text() const {
    return text(unsafe("buf_ is always validly encoded"), buf_.as_span(),
                enc());
  }
  operator text() const { return as_text(); }

  /// # `textbuf::operator buf`
  ///
  /// Moves out of this string and returns the raw code unit vector.This is also
  /// an implicit conversion.
  buf into_buf() && { return std::move(buf_); }
  operator buf() && { return std::move(buf_); }

  /// # `textbuf::as_codes()`
  ///
  /// Returns the span of code units that backs this string.
  best::span<const code> as_codes() const { return buf_; }

  /// # `textbuf::is_rune_boundary()`
  ///
  /// Returns whether or not `idx` is a rune boundary or not. Returns `false`
  /// for oud-of-bounds indices.
  ///
  /// For stateless encodings, this is an O(1) check. For non-synchronizing
  /// encodings, it is O(n).
  bool is_rune_boundary(size_t idx) const {
    return as_text().is_rune_boundary(idx);
  }

  /// # `text[{...}]`
  ///
  /// Gets the substring in the given range. Crashes on out-of-bounds access
  /// or, if this encoding is stateless, if `range` slices through a non-rune
  /// boundary.
  ///
  /// Beware: this check is O(n) for non-synchronizing encoding.
  text operator[](best::bounds::with_location range) const {
    return as_text()[range];
  }

  /// # `textbuf::at()`
  ///
  /// Gets the substring in the given range. Returns `best::none` where
  /// `operator[]` would crash.
  ///
  /// Beware: this check is O(n) for non-synchronizing encoding.
  best::option<text> at(best::bounds range) const {
    return as_text().at(range);
  }

  /// # `textbuf::at(unsafe)`
  ///
  /// Gets the substring in the given range, performing no bounds checks.
  text at(unsafe u, best::bounds range) const { return as_text().at(u, range); }

  /// # `textbuf::starts_with()`
  ///
  /// Checks whether this string begins with the specifies substring or rune.
  bool starts_with(rune r) const { return as_text().starts_with(r); }
  bool starts_with(const string_type auto& s) const {
    return as_text().starts_with(s);
  }

  /// # `textbuf::trim_prefix()`
  ///
  /// If this string starts with the given prefix, returns a copy of this string
  /// with that prefix removed.
  best::option<text> trim_prefix(rune r) const {
    return as_text().starts_with(r);
  }
  best::option<text> trim_prefix(const string_type auto& s) const {
    return as_text().starts_with(s);
  }

  /// # `textbuf::contains()`
  ///
  /// Whether this string contains a particular substring or rune..
  bool contains(rune r) const { return as_text().contains(r); }
  bool contains(const string_type auto& s) const {
    return as_text().contains(s);
  }

  /// # `textbuf::find()`
  ///
  /// Finds the first occurrence of a substring or rune within this string.
  ///
  /// If any invalidly text characters are encountered during the search, in
  /// either the haystack or the needle, this function returns `best::none`.
  best::option<size_t> find(rune r) const { return as_text().find(r); }
  best::option<size_t> find(const string_type auto& s) const {
    return as_text().find(s);
  }
  best::option<size_t> find(best::callable<bool(rune)> auto&& p) const {
    return as_text().find(BEST_FWD(p));
  }

  /// # `textbuf::split_at()`
  ///
  /// Splits this string into two on the given index. If the desired split point
  /// is out of bounds, returns `best::none`.
  best::option<best::row<text, text>> split_at(size_t n) const {
    return as_text().split_at(n);
  }

  /// # `textbuf::split_on()`
  ///
  /// Splits this string into two on the first occurrence of the given substring
  /// or rune, or when the callback returns true. If the desired split point
  /// is not found, returns `best::none`.
  best::option<best::row<text, text>> split_on(rune r) const {
    return as_text().split_on(r);
  }
  best::option<best::row<text, text>> split_on(
      const string_type auto& s) const {
    return as_text().split_on(s);
  }
  best::option<best::row<text, text>> split_on(
      best::callable<bool(rune)> auto&& p) const {
    return as_text().split_on(p);
  }

  /// # `textbuf::rune_iter`, `textbuf::runes()`.
  ///
  /// An iterator over the runes of a `best::textbuf`.
  ///
  /// A `best::text` may point to invalidly-text data. If the encoding is
  /// self-synchronizing, the stream of Unicode characters is interpreted as
  /// replacing each invalid code unit with a Unicode replacement character
  /// (U+FFFD). If the encoding is not self-synchronizing, the stream is
  /// interpreted to end at that position, with a replacement character.
  rune::iter<E> runes() const { return rune::iter<E>(buf_, enc_); }

  /// # `textbuf::reserve()`.
  ///
  /// Ensures that pushing an additional `count` code units would not cause this
  /// string to resize, by resizing the internal array eagerly.
  void reserve(size_t count) { buf_.reserve(count); }

  /// # `textbuf::truncate()`.
  ///
  /// Shortens the string to be at most `count` code units long.
  /// If `count > size()`, this function does nothing. Crashes if this would
  /// slice through a character boundary.
  void truncate(size_t count) {
    if (count > size()) return;
    (void)operator[]({.count = count});  // Perform a bounds check.
    buf_.truncate(count);
  }

  /// # `textbuf::push()`.
  ///
  /// Pushes a rune or string to this vector. Returns `false` if input text
  /// contains characters that cannot be transcoded to this strings's encoding.
  bool push(rune r) {
    code buf[About.max_codes_per_rune];
    if (auto codes = r.encode(buf, enc())) {
      buf_.append(*codes);
      return true;
    }
    return false;
  }
  bool push(const string_type auto& that) {
    rune::iter it(that);
    return push(it.rest(), best::encoding_of(that));
  }
  template <best::encoding E2>
  bool push(best::span<const best::code<E2>> data, const E2& enc) {
    rune::iter it(data, enc);
    if constexpr (best::same<code, best::code<E2>> && best::equatable<E, E2>) {
      if (this->enc() == enc) {
        buf_.append(it.rest());
        return true;
      }
    }

    size_t watermark = size();
    for (rune r : it) {
      reserve(About.max_codes_per_rune);
      best::span<code> buf = {buf_.data() + buf_.size(),
                              About.max_codes_per_rune};
      if (auto codes = r.encode(buf, this->enc())) {
        buf_.set_size(unsafe("we just wrote this much data in encode()"),
                      size() + codes->size());
        continue;
      }
      truncate(watermark);
      return false;
    }
    return true;
  }

  /// # `textbuf::push_lossy()`.
  ///
  /// Pushes a rune or string to this vector. If the input text contains
  /// characters that cannot be transcoded into this string's encoding, they
  /// are replaced with `rune::Replacement`, or if that cannot be encoded, with
  /// `?`.
  void push_lossy(rune r) {
    code buf[About.max_codes_per_rune];
    if (auto codes = r.encode(buf, enc())) {
      buf_.append(*codes);
    } else if (auto codes = rune::Replacement.encode(buf, enc())) {
      buf_.append(*codes);
    } else {
      codes = rune('?').encode(buf, enc());
      buf_.append(*codes);
    }
  }
  void push_lossy(const string_type auto& that) {
    rune::iter it(that);
    push_lossy(it.rest(), best::encoding_of(that));
  }
  template <best::encoding E2>
  void push_lossy(best::span<const best::code<E2>> data, const E2& enc) {
    rune::iter it(data, enc);
    if constexpr (best::same<code, best::code<E2>> && best::equatable<E, E2>) {
      if (this->enc() == enc) {
        buf_.append(it.rest());
        return;
      }
    }

    for (rune r : it) {
      reserve(About.max_codes_per_rune);
      best::span<code> buf = {buf_.data() + buf_.size(),
                              About.max_codes_per_rune};

      unsafe u("we just wrote this much data in encode()");
      if (auto codes = r.encode(buf, this->enc())) {
        buf_.set_size(u, size() + codes->size());
      } else if (auto codes = rune::Replacement.encode(buf, this->enc())) {
        buf_.set_size(u, size() + codes->size());
      } else {
        codes = rune('?').encode(buf, this->enc());
        buf_.set_size(u, size() + codes->size());
      }
    }
  }

  /// # `textbuf::clear()`.
  ///
  /// Clears this string. This resizes it to zero without changing the capacity.
  void clear() { buf_.clear(); }

  /// # `textbuf::operator==`
  ///
  /// Strings can be compared regardless of encoding, and they may be
  /// compared with runes, too.
  bool operator==(rune r) const { return as_text() == r; }
  bool operator==(const string_type auto& s) const { return as_text() == s; }
  bool operator==(best::span<const code> span) const {
    return as_text() == span;
  }
  bool operator==(const code* lit) const { return as_text() == lit; }

  // Make this into a best::string_type.
  friend const encoding& BestEncoding(auto, const textbuf& t) {
    return t.enc();
  }

 private:
  explicit textbuf(best::in_place_t, buf buf, encoding enc)
      : buf_(std::move(buf)), enc_(std::move(enc)) {}
  buf buf_;
  [[no_unique_address]] encoding enc_;
};
}  // namespace best

/******************************************************************************/

///////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! /////////////////////

/******************************************************************************/

namespace best {
template <encoding E, allocator A>
best::option<textbuf<E, A>> textbuf<E, A>::from(alloc alloc,
                                                best::span<const code> data,
                                                encoding enc) {
  if (!rune::validate(data, enc)) {
    return best::none;
  }

  return textbuf(best::in_place, buf(std::move(alloc), data), std::move(enc));
}

template <encoding E, allocator A>
best::option<textbuf<E, A>> textbuf<E, A>::from(buf data, encoding enc) {
  if (!rune::validate(data, enc)) {
    return best::none;
  }

  return textbuf(best::in_place, std::move(data), std::move(enc));
}
}  // namespace best

#endif  // BEST_TEXT_STRBUF_H_