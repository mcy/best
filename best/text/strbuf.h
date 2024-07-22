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

#ifndef BEST_TEXT_STRBUF_H_
#define BEST_TEXT_STRBUF_H_

#include <cstddef>

#include "best/container/vec.h"
#include "best/func/arrow.h"
#include "best/memory/allocator.h"
#include "best/memory/span.h"
#include "best/text/encoding.h"
#include "best/text/rune.h"
#include "best/text/str.h"

//! Unicode string buffers.
//!
//! `best::textbuf` is to `best::text` as `std::string` is to
//! `std::string_view`. It is a growable array of code units with support for
//! SSO and custom allocators.
//!
//! `best::strbuf`, `best::strbuf16`, and `best::strbuf32` are type aliases
//! corresponding to the UTF-8/16/32 specializations of the above.

namespace best {
template <typename, best::allocator>
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
/// A `best::textbuf` string can be created from a string literal; in this case,
/// it will be validated for being "correctly text" wrt to the encoding `E`.
/// It can also be constructed from a pointer, in which case no such check
/// occurs.
///
/// A `best::textbuf` may not point to invalidly-text data. Constructors from
/// unauthenticated strings must go through factories that return
/// `best::optional`.
///
/// Note that `best::textbuf` only provides a subset of the `best::textbuf`
/// functions. To access the full suite of span operations, you must access them
/// through `->`, e.g., `buf->find()`.
template <typename E, best::allocator A = best::malloc>
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

  /// # `textbuf::text`, `textbuf::pretext`.
  ///
  /// The corresponding view types for this `textbuf`.
  using text = best::text<encoding>;
  using pretext = best::pretext<encoding>;

  /// # `textbuf::vec`
  ///
  /// The corresponding raw vector type for this `textbuf`.
  using buf = best::vec<code, best::vec_inline_default<code>(), alloc>;

  /// # `textbuf::textbuf()`
  ///
  /// Creates a new, empty string with the given encoding and/or allocator.
  textbuf() : textbuf(alloc{}) {}
  explicit textbuf(alloc alloc, encoding enc = {})
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
  textbuf(const code (&lit)[n]) BEST_IS_VALID_LITERAL(lit, encoding{})
    : textbuf(unsafe("statically validated"), buf(span(lit, n - 1)),
              encoding{}) {}

  /// # `textbuf::text(unsafe)`
  ///
  /// Creates a new string by wrapping a code buffer or a pretext. It is up to
  /// the caller to ensure the data is well-encoded.
  explicit textbuf(unsafe, buf buf, encoding enc = {})
    : buf_(std::move(buf)), enc_(std::move(enc)) {}
  explicit textbuf(unsafe, pretext text)
    : buf_(std::move(text)), enc_(std::move(text.enc())) {}

  /// # `textbuf::from()`
  ///
  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  static best::option<textbuf> from(pretext text) { return from({}, text); }
  static best::option<textbuf> from(alloc alloc, pretext text);
  static best::option<textbuf> from(buf data, encoding enc = {});

  /// # `textbuf::from_nul()`
  ///
  /// Creates a new string by parsing it from a NUL-terminated string. It must
  /// end in `code{0}`. If `data == `nullptr`, returns an empty string.
  static best::option<textbuf> from_nul(const code* data, encoding enc = {}) {
    return from_nul({}, data, std::move(enc));
  }
  static best::option<textbuf> from_nul(alloc alloc, const code* data,
                                        encoding enc = {}) {
    return from(std::move(alloc), pretext::from_nul(data, std::move(enc)));
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
    textbuf out(alloc);
    if (!out.push(that)) { return best::none; }
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

  /// # `textbuf::as_text()`, `vec::operator->()`
  ///
  /// Returns the span of code units that backs this string. This is also
  /// an implicit conversion.
  ///
  /// All of the text methods, including those not explicitly delegated, are
  /// accessible through `->`. For example, `my_str->size()` works.
  text as_text() const {
    return text(unsafe("buf_ is always validly encoded"),
                {buf_.as_span(), enc()});
  }
  operator text() const { return as_text(); }
  best::arrow<text> operator->() const { return as_text(); }

  /// # `textbuf::operator buf`
  ///
  /// Moves out of this string and returns the raw code unit vector.This is also
  /// an implicit conversion.
  buf into_buf() && { return std::move(buf_); }
  operator buf() && { return std::move(buf_); }

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

  /// # `text::rune_iter`, `text::runes()`.
  ///
  /// An iterator over the runes of a `best::text`.
  using rune_iter = text::rune_iter;
  rune_iter runes() const { return as_text().runes(); }

  /// # `text::rune_index_iter`, `text::rune_indices()`.
  ///
  /// An iterator over the runes of a `best::text` and the indices they
  /// occur at in the underlying code span.
  using rune_index_iter = text::rune_index_iter;
  rune_index_iter rune_indices() const { return as_text().rune_indices(); }

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
    if (count > size()) { return; }
    (void)operator[]({.count = count});  // Perform a bounds check.
    buf_.truncate(count);
  }

  /// # `textbuf::push()`.
  ///
  /// Pushes a rune or string to this vector. Returns `false` if input text
  /// contains characters that cannot be transcoded to this strings's encoding.
  bool push(rune r);
  bool push(const string_type auto& that);

  /// # `textbuf::push_lossy()`.
  ///
  /// Pushes a rune or string to this vector. If the input text contains
  /// characters that cannot be transcoded into this string's encoding, they
  /// are replaced with `rune::Replacement`, or if that cannot be encoded, with
  /// `?`.
  void push_lossy(rune r);
  void push_lossy(const string_type auto& that);

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

  constexpr best::ord operator<=>(rune r) const { return as_text() <=> r; }
  constexpr best::ord operator<=>(const string_type auto& s) const {
    return as_text() <=> s;
  }
  constexpr best::ord operator<=>(best::span<const code> span) const {
    return as_text() <=> span;
  }
  constexpr best::ord operator<=>(const code* lit) const {
    return as_text() <=> best::span<const code>::from_nul(lit);
  }

 private:
  buf buf_;
  [[no_unique_address]] encoding enc_;
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename E, allocator A>
best::option<textbuf<E, A>> textbuf<E, A>::from(alloc alloc, pretext t) {
  auto validated = text::from(t);
  BEST_GUARD(validated);

  return textbuf(best::unsafe("just did validation above"), *validated);
}

template <typename E, allocator A>
best::option<textbuf<E, A>> textbuf<E, A>::from(buf data, encoding enc) {
  if (!rune::validate(data, enc)) { return best::none; }

  return textbuf(best::in_place, std::move(data), std::move(enc));
}

template <typename E, allocator A>
bool textbuf<E, A>::push(rune r) {
  code buf[About.max_codes_per_rune];
  if (auto codes = r.encode(buf, enc())) {
    buf_.append(*codes);
    return true;
  }
  return false;
}
template <typename E, allocator A>
bool textbuf<E, A>::push(const string_type auto& that) {
  if constexpr (best::is_text<decltype(that)> &&
                best::same_encoding_code<textbuf, decltype(that)>()) {
    if (best::same_encoding(*this, that)) {
      buf_.append(that);
      return true;
    }
  }

  if constexpr (best::is_text<decltype(that)> ||
                best::is_pretext<decltype(that)>) {
    size_t watermark = size();
    for (auto r : that.runes()) {
      reserve(About.max_codes_per_rune);
      best::span<code> buf{buf_.data() + buf_.size(), About.max_codes_per_rune};
      if (auto codes = r.encode(buf, this->enc())) {
        buf_.set_size(unsafe("we just wrote this much data in encode()"),
                      size() + codes.ok()->size());
        continue;
      }
      truncate(watermark);
      return false;
    }
    return true;
  } else {
    return push(best::pretext(that));
  }
}

template <typename E, allocator A>
void textbuf<E, A>::push_lossy(rune r) {
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

template <typename E, allocator A>
void textbuf<E, A>::push_lossy(const string_type auto& that) {
  if constexpr (best::is_text<decltype(that)> &&
                best::same_encoding_code<textbuf, decltype(that)>()) {
    if (best::same_encoding(*this, that)) {
      buf_.append(that);
      return;
    }
  }

  if constexpr (best::is_text<decltype(that)> ||
                best::is_pretext<decltype(that)>) {
    for (auto r : that.runes()) {
      reserve(About.max_codes_per_rune);
      best::span<code> buf = {buf_.data() + buf_.size(),
                              About.max_codes_per_rune};

      unsafe u("we just wrote this much data in encode()");
      if (auto codes = r.encode(buf, this->enc())) {
        buf_.set_size(u, size() + codes.ok()->size());
      } else if (auto codes = rune::Replacement.encode(buf, this->enc())) {
        buf_.set_size(u, size() + codes.ok()->size());
      } else {
        codes = rune('?').encode(buf, this->enc());
        buf_.set_size(u, size() + codes.ok()->size());
      }
    }
  } else {
    push_lossy(best::pretext(that));
  }
}
}  // namespace best

#endif  // BEST_TEXT_STRBUF_H_
