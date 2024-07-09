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

#ifndef BEST_TEXT_STR_H_
#define BEST_TEXT_STR_H_

#include <stdio.h>

#include <cstddef>

#include "best/base/guard.h"
#include "best/container/span.h"
#include "best/math/overflow.h"
#include "best/memory/bytes.h"
#include "best/meta/taxonomy.h"
#include "best/text/encoding.h"
#include "best/text/rune.h"
#include "best/text/utf8.h"

//! Unicode strings.
//!
//! best::text is a Unicode string, i.e., an text sequence of best::runes.
//! It is essentially std::basic_string_view with a nicer API (compare with
//! best::span).
//!
//! best::str, best::str16, and best::str32 are type aliases corresponding to
//! the UTF-8/16/32 specializations of the above.

namespace best {
/// # `best::str`
///
/// A reference to UTF-8 text data.
using str = best::text<utf8>;

/// # `best::str16`
///
/// A reference to UTF-16 text data.
using str16 = best::text<utf16>;

/// # `best::str32`
///
/// A reference to UTF-32 text data.
using str32 = best::text<utf32>;

/// # `BEST_IS_VALID_LITERAL()`
///
/// A function requirement that verifies that `literal_` is a valid string
/// literal for `enc_`.
///
/// This is intended to be placed after any `requires` clauses.
#define BEST_IS_VALID_LITERAL(literal_, enc_)                               \
  BEST_ENABLE_IF_CONSTEXPR(literal_)                                        \
  BEST_ENABLE_IF(                                                           \
      rune::validate(best::span(literal_, best::size(literal_) - 1), enc_), \
      "string literal must satisfy rune::validate() for the chosen encoding")

/// # `best::is_text`
///
/// Whether `T` is `best::text<E>` for some encoding `E`.
template <typename T>
concept is_text = requires {
  typename best::as_auto<T>::encoding;
  requires best::same<best::as_auto<T>,
                      best::text<typename best::as_auto<T>::encoding>>;
};

/// # `best::is_pretext`
///
/// Whether `T` is `best::pretext<E>` for some encoding `E`.
template <typename T>
concept is_pretext = requires {
  typename best::as_auto<T>::encoding;
  requires best::same<best::as_auto<T>,
                      best::pretext<typename best::as_auto<T>::encoding>>;
};

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
/// A `best::text` may not point to invalidly-encoded data; use `best::pretext`
/// for that. Constructors from unauthenticated strings must go through
/// factories that return `best::optional`.
template <typename E>  // Not best::encoding so we can forward-declare it.
class text final {
 public:
  /// # `text::encoding`
  ///
  /// The encoding for this string.
  using encoding = E;

  /// # `text::code`
  ///
  /// The code unit for this encoding. This is the element type of an text
  /// stream.
  using code = encoding::code;

  /// # `text::pretext`
  ///
  /// The corresponding unvalidated string type.
  using pretext = best::pretext<E>;

  /// # `text::About`
  ///
  /// Metadata about this strings's encoding.
  static constexpr best::encoding_about About = E::About;

  /// # `text::text()`
  ///
  /// Creates a new, empty string with the given encoding.
  constexpr text() : text_() {}
  constexpr explicit text(encoding enc) : text_(std::move(enc)) {}

  /// # `text::text(text)`
  ///
  /// Copyable and movable.
  constexpr text(const text&) = default;
  constexpr text& operator=(const text&) = default;
  constexpr text(text&&) = default;
  constexpr text& operator=(text&&) = default;

  /// # `text::text("...")`
  ///
  /// Creates a new string from a string literal with an optional encoding.
  /// The array must be a constant, and it must contain validly-e\ncoded data.
  template <size_t n>
  constexpr text(const code (&lit)[n], encoding enc)
      BEST_IS_VALID_LITERAL(lit, enc)
      : text_(lit, std::move(enc)) {}
  template <size_t n>
  constexpr text(const code (&lit)[n]) BEST_IS_VALID_LITERAL(lit, encoding{})
      : text_(lit) {}

  /// # `text::text(unsafe)`
  ///
  /// Creates a new string from a pretext. This function performs no validation,
  /// and misuse may lead to erratic behavior.
  constexpr explicit text(unsafe, pretext text) : text_(std::move(text)) {}

  /// # `text::from()`
  ///
  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  constexpr static best::option<text> from(pretext text);

  /// # `text::from_partial()`
  ///
  /// Creates a new string by decoding the longest valid prefix of `data`.
  /// Returns the valid prefix, and the rest of `data`.
  constexpr static best::row<text, pretext> from_partial(pretext text);

  /// # `text::from_nul()`
  ///
  /// Creates a new string by parsing it from a NUL-terminated string. It must
  /// end in `code{0}`. If `data == `nullptr`, returns an empty string.
  constexpr static best::option<text> from_nul(const code* data,
                                               encoding enc = {}) {
    return from(
        pretext(best::span<const code>::from_nul(data), std::move(enc)));
  }

  /// # `text::data()`
  ///
  /// Returns the string's data pointer.
  /// This value is never null.
  constexpr const code* data() const { return text_.data(); }

  /// # `text::is_empty()`
  ///
  /// Checks whether the string is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// # `text::size()`
  ///
  /// Returns the size of the string, in code units.
  constexpr size_t size() const { return text_.size(); }

  /// # `text::enc()`
  ///
  /// Returns the underlying text encoding.
  constexpr const encoding& enc() const { return text_.enc(); }

  /// # `text::as_codes()`
  ///
  /// Returns the span of code units that backs this string.
  constexpr best::span<const code> as_codes() const { return text_.as_codes(); }

  /// # `text::is_rune_boundary()`
  ///
  /// Returns whether or not `idx` is a rune boundary or not. Returns `false`
  /// for oud-of-bounds indices.
  ///
  /// For stateless encodings, this is an O(1) check. For non-synchronizing
  /// encodings, it is O(n).
  constexpr bool is_rune_boundary(size_t idx) const;

  /// # `text[{...}]`
  ///
  /// Gets the substring in the given range. Crashes on out-of-bounds access
  /// or, if this encoding is stateless, if `range` slices through a non-rune
  /// boundary.
  ///
  /// Beware: this check is O(n) for non-synchronizing encoding.
  constexpr text operator[](best::bounds::with_location range) const;

  /// # `text::at()`
  ///
  /// Gets the substring in the given range. Returns `best::none` where
  /// `operator[]` would crash.
  ///
  /// Beware: this check is O(n) for non-synchronizing encoding.
  constexpr best::option<text> at(best::bounds range) const;

  /// # `text::at(unsafe)`
  ///
  /// Gets the substring in the given range, performing no bounds checks.
  constexpr text at(unsafe u, best::bounds range) const {
    return text(u, text_.at(u, range));
  }

 private:
  class rune_iter_impl;
  class rune_index_iter_impl;

 public:
  /// # `text::rune_iter`, `text::runes()`.
  ///
  /// An iterator over the runes of a `best::text`.
  using rune_iter = best::iter<rune_iter_impl>;
  constexpr rune_iter runes() const { return rune_iter(rune_iter_impl(*this)); }

  /// # `text::rune_index_iter`, `text::rune_indices()`.
  ///
  /// An iterator over the runes of a `best::text` and the indices they
  /// occur at in the underlying code span.
  using rune_index_iter = best::iter<rune_index_iter_impl>;
  constexpr rune_index_iter rune_indices() const {
    return rune_index_iter(rune_index_iter_impl(*this));
  }

  /// # `text::starts_with()`
  ///
  /// Checks whether this string begins with the specified substring or rune.
  constexpr bool starts_with(rune prefix) const {
    return text_.starts_with(prefix);
  }
  constexpr bool starts_with(const string_type auto& prefix) const {
    return text_.starts_with(prefix);
  }
  constexpr bool starts_with(best::callable<bool(rune)> auto&& pred) const {
    return text_.starts_with(BEST_FWD(pred));
  }

  /// # `text::trim_prefix()`
  ///
  /// If this string starts with the given prefix, returns a copy of this string
  /// with that prefix removed.
  constexpr best::option<text> strip_prefix(rune prefix) const {
    if (auto suffix = text_.strip_prefix(prefix)) {
      return text(unsafe("suffix was created from a best::text"), *suffix);
    }
    return best::none;
  }
  constexpr best::option<text> strip_prefix(
      const string_type auto& prefix) const {
    if (auto suffix = text_.strip_prefix(prefix)) {
      return text(unsafe("suffix was created from a best::text"), *suffix);
    }
    return best::none;
  }
  constexpr best::option<text> strip_prefix(
      best::callable<bool(rune)> auto&& prefix) const {
    if (auto suffix = text_.strip_prefix(BEST_FWD(prefix))) {
      return text(unsafe("suffix was created from a best::text"), *suffix);
    }
    return best::none;
  }

  /// # `text::consume_prefix()`
  ///
  /// If this string starts with the given prefix, returns `true` and updates
  /// this string to the result of `trim_prefix()`. Otherwise, returns `false`
  /// and leaves this string unchanged.
  constexpr bool consume_prefix(rune r) {
    auto suffix = strip_prefix(r);
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }
  constexpr bool consume_prefix(const string_type auto& s) {
    auto suffix = strip_prefix(s);
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }
  constexpr bool consume_prefix(best::callable<bool(rune)> auto&& p) {
    auto suffix = trim_prefix(BEST_FWD(p));
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }

  /// # `text::split_at()`
  ///
  /// Splits this string into two on the given index. If the desired split point
  /// is out of bounds, returns `best::none`.
  constexpr best::option<best::row<text, text>> split_at(size_t n) const {
    auto prefix = at({.end = n});
    BEST_GUARD(prefix);
    return {{*prefix, at(unsafe("already did a bounds check"), {.start = n})}};
  }

  /// # `text::find()`.
  ///
  /// Finds the first occurrence of a pattern by linear search, and returns its
  /// position.
  ///
  /// A pattern may be:
  ///
  /// - A rune.
  /// - A string type.
  /// - A rune predicate.
  ///
  /// Where possible, this function will automatically call vectorized
  /// implementations of e.g. `memchr` and `memcmp` for finding the desired
  /// pattern. Therefore, when possible, prefer to provide a needle by value.
  constexpr best::option<size_t> find(best::rune needle) const;
  constexpr best::option<size_t> find(
      const best::string_type auto& needle) const;
  constexpr best::option<size_t> find(
      best::callable<bool(rune)> auto&& pred) const;

  /// # `text::contains()`
  ///
  /// Determines whether a substring exists that matches some pattern.
  ///
  /// A pattern may be as in `text::find()`.
  constexpr bool contains(rune needle) const {
    return find(needle).has_value();
  }
  constexpr bool contains(const string_type auto& needle) const {
    return find(needle).has_value();
  }
  constexpr bool contains(best::callable<bool(rune)> auto&& needle) const {
    return find(BEST_FWD(needle)).has_value();
  }

  /// # `text::split_once()`
  ///
  /// Calls `text::find()` to find the first occurrence of some pattern, and
  /// if found, returns the substrings before and after the separator.
  ///
  /// A pattern for a separator may be as in `text::find()`.
  constexpr best::option<best::row<text, text>> split_once(rune needle) const;
  constexpr best::option<best::row<text, text>> split_once(
      const string_type auto& needle) const;
  constexpr best::option<best::row<text, text>> split_once(
      best::callable<bool(rune)> auto&& pred) const;

  /// # `text::split()`.
  ///
  /// Returns an iterator over substrings separated by some pattern. Internally,
  /// it calls `text::split_once()` until it is out of string.
  ///
  /// A pattern for a separator may be as in `text::find()`.
  constexpr auto split(best::rune needle) const;
  constexpr auto split(const best::string_type auto& needle) const;
  constexpr auto split(best::callable<bool(rune)> auto&& pred) const;

 private:
  template <typename P>
  class split_impl;
  template <typename P>
  using split_iter = best::iter<split_impl<P>>;

 public:
  /// # `text::operator==`, `text::operator<=>`
  ///
  /// Strings can be compared regardless of encoding, and they may be compared
  /// with runes, too.
  constexpr bool operator==(rune r) const { return text_ == r; }
  constexpr bool operator==(const string_type auto& s) const {
    return text_ == s;
  }
  constexpr bool operator==(const text&) const = default;
  constexpr bool operator==(best::span<const code> span) const {
    return text_ == span;
  }
  constexpr bool operator==(const code* lit) const {
    return text_ == best::span<const code>::from_nul(lit);
  }

  constexpr best::ord operator<=>(rune r) const { return text_ <=> r; }
  constexpr best::ord operator<=>(const string_type auto& s) const {
    return text_ <=> s;
  }
  constexpr best::ord operator<=>(best::span<const code> span) const {
    return text_ <=> span;
  }
  constexpr best::ord operator<=>(const code* lit) const {
    return text_ <=> best::span<const code>::from_nul(lit);
  }

  // Make this into a best::string_type.
  constexpr friend const encoding& BestEncoding(auto, const text& t) {
    return t.enc();
  }

 private:
  best::pretext<E> text_;
};

/// # `best::pretext`
///
/// A `best::text` without the well-encoded guarantee. This is what you get
/// before you build a valid text: a "pre" text.
///
/// There are many situations in which we might want to operate on a span of
/// code units that is "probably" valid: for example, the correct type for a
/// POSIX file path is `best::pretext<utf8>`: it can be almost any byte string,
/// but we like to believe it's *probably* valid UTF-8.
///
/// What's nice is that as long as you stay within the same encoding, pretty
/// much all of the usual string algorithms are well-defined. If you go
/// cross-encoding though, things get hairy. We try to fall back in "reasonable"
/// cases, but for non-synchronizing encodings there may be little that can be
/// done.
template <typename E>  // Not best::encoding so we can forward-declare it.
class pretext final {
 public:
  /// # `pretext::encoding`
  ///
  /// The encoding for this string.
  using encoding = E;

  /// # `pretext::code`
  ///
  /// The code unit for this encoding. This is the element type of an text
  /// stream.
  using code = encoding::code;

  /// # `pretext::About`
  ///
  /// Metadata about this strings's encoding.
  static constexpr best::encoding_about About = E::About;

  /// # `pretext::pretext()`
  ///
  /// Creates a new, empty string with the given encoding.
  constexpr pretext() : pretext(encoding{}) {}
  constexpr explicit pretext(encoding enc)
      : pretext(best::span(&empty, 0), std::move(enc)) {}

  /// # `pretext::pretext("...")`
  ///
  /// Creates a new string from a string literal with an optional encoding.
  /// This assumes you're constructing from a string literal. In C++, every
  /// string literal is an array "helpfully" suffixed with a NUL. These
  /// functions discard that NUL.
  template <size_t n>
  constexpr pretext(const code (&lit)[n], encoding enc = {})
      : pretext(span(lit, n - 1), std::move(enc)) {}

  /// # `pretext::pretext(string)`
  ///
  /// Creates a new string from some other string type whose encoding we can
  /// divine.
  constexpr pretext(const best::string_type auto& data)
    requires best::same<encoding, best::encoding_type<decltype(data)>>
      : span_(data), enc_(best::encoding_of(data)) {}

  /// # `pretext::pretext(span)`
  ///
  /// Creates a new string from an arbitrary span.
  template <best::contiguous R>
  constexpr pretext(const R& data, encoding enc = {})
    requires best::same<best::unqual<best::data_type<R>>, code> &&
                 (!best::string_type<R> ||
                  !best::same<encoding, best::encoding_type<decltype(data)>>)
      : span_(data), enc_(BEST_MOVE(enc)) {}

  /// # `pretext::from_nul()`
  ///
  /// Creates a new string by parsing it from a NUL-terminated string. It must
  /// end in `code{0}`. If `data == `nullptr`, returns an empty string.
  constexpr static pretext from_nul(const code* data, encoding enc = {}) {
    return pretext(best::span<const code>::from_nul(data), std::move(enc));
  }

  /// # `pretext::data()`
  ///
  /// Returns the string's data pointer.
  /// This value is never null.
  constexpr const code* data() const { return span_.data(); }

  /// # `pretext::is_empty()`
  ///
  /// Checks whether the string is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// # `pretext::size()`
  ///
  /// Returns the size of the string, in code units.
  constexpr size_t size() const { return span_.size(); }

  /// # `pretext::enc()`
  ///
  /// Returns the underlying text encoding.
  constexpr const encoding& enc() const { return enc_; }

  /// # `pretext::as_codes()`
  ///
  /// Returns the span of code units that backs this string.
  constexpr best::span<const code> as_codes() const { return span_; }

  /// # `pretext[{...}]`
  ///
  /// Gets the substring in the given range. Crashes on out-of-bounds access.
  constexpr pretext operator[](best::bounds::with_location range) const {
    return pretext(span_[range], enc());
  }

  /// # `text::at()`
  ///
  /// Gets the substring in the given range. Returns `best::none` where
  /// `operator[]` would crash.
  ///
  /// Beware: this check is O(n) for non-synchronizing encoding.
  constexpr best::option<pretext> at(best::bounds range) const {
    return span_.at(range).map([&](auto sp) { return pretext(sp, enc()); });
  }

  /// # `text::at(unsafe)`
  ///
  /// Gets the substring in the given range, performing no bounds checks.
  constexpr pretext at(unsafe u, best::bounds range) const {
    return pretext(span_.at(u, range), enc());
  }

 private:
  class rune_iter_impl;
  class rune_index_iter_impl;
  class rune_try_iter_impl;

 public:
  /// # `pretext::rune_iter`, `pretext::runes()`.
  //
  /// An iterator over the runes of a `best::pretext`.
  ///
  /// A `best::pretext` may point to invalidly-text data. If the encoding is
  /// self-synchronizing, the stream of Unicode characters is interpreted as
  /// replacing each invalid code unit with a Unicode replacement character
  /// (U+FFFD). If the encoding is not self-synchronizing, the stream is
  /// interpreted to end at that position, with a replacement character.
  using rune_iter = best::iter<rune_iter_impl>;
  constexpr rune_iter runes() const {
    return rune_iter(rune_iter_impl(try_runes()));
  }

  /// # `pretext::rune_index_iter`, `pretext::rune_indices()`.
  ///
  /// An iterator over the runes of a `best::pretext` and the indices they
  /// occur at in the underlying code span.
  ///
  /// This has the same replacement character behavior as `runes()`.
  using rune_index_iter = best::iter<rune_index_iter_impl>;
  constexpr rune_index_iter rune_indices() const {
    return rune_index_iter(rune_index_iter_impl(try_runes()));
  }

  /// # `pretext::rune_try_iter`, `pretext::try_runes()`.
  ///
  /// A fallible iterator over the runes of a `best::pretext`.
  ///
  /// Similar to `pretext::runes()`, except it will return a `best::result` on
  /// decode failure.
  using rune_try_iter = best::iter<rune_try_iter_impl>;
  constexpr rune_try_iter try_runes() const {
    return rune_try_iter(rune_try_iter_impl(*this));
  }

  /// # `pretext::split_at()`
  ///
  /// Splits this string into two on the given index. If the desired split point
  /// is out of bounds, returns `best::none`.
  constexpr best::option<best::row<pretext, pretext>> split_at(size_t n) const {
    auto prefix = at({.end = n});
    BEST_GUARD(prefix);
    return {{*prefix, at(unsafe("already did a bounds check"), {.start = n})}};
  }

  /// # `pretext::find()`.
  ///
  /// Finds the first occurrence of a pattern by linear search, and returns its
  /// position.
  ///
  /// A pattern may be:
  ///
  /// - A rune.
  /// - A string type.
  /// - A rune predicate.
  ///
  /// Where possible, this function will automatically call vectorized
  /// implementations of e.g. `memchr` and `memcmp` for finding the desired
  /// pattern. Therefore, when possible, prefer to provide a needle by value.
  constexpr best::option<size_t> find(best::rune needle) const;
  constexpr best::option<size_t> find(
      const best::string_type auto& needle) const;
  constexpr best::option<size_t> find(
      best::callable<bool(rune)> auto&& pred) const;

  /// # `pretext::contains()`
  ///
  /// Determines whether a substring exists that matches some pattern.
  ///
  /// A pattern may be as in `pretext::find()`.
  constexpr bool contains(best::rune needle) const {
    return find(needle).has_value();
  }
  constexpr bool contains(const best::string_type auto& needle) const {
    return find(needle).has_value();
  }
  constexpr bool contains(best::callable<bool(rune)> auto&& pred) const {
    return find(BEST_FWD(pred)).has_value();
  }

  /// # `pretext::split_once()`
  ///
  /// Calls `pretext::find()` to find the first occurrence of some pattern, and
  /// if found, returns the substrings before and after the separator.
  ///
  /// A pattern for a separator may be as in `pretext::find()`.
  constexpr best::option<best::row<pretext, pretext>> split_once(
      best::rune needle) const;
  constexpr best::option<best::row<pretext, pretext>> split_once(
      const best::string_type auto& needle) const;
  constexpr best::option<best::row<pretext, pretext>> split_once(
      best::callable<bool(rune)> auto&& pred) const;

  /// # `pretext::split()`.
  ///
  /// Returns an iterator over substrings separated by some pattern. Internally,
  /// it calls `pretext::split_once()` until it is out of string.
  ///
  /// A pattern for a separator may be as in `pretext::find()`.
  constexpr auto split(best::rune needle) const;
  constexpr auto split(const best::string_type auto& needle) const;
  constexpr auto split(best::callable<bool(rune)> auto&& pred) const;

 private:
  template <typename P>
  class split_impl;
  template <typename P>
  using split_iter = best::iter<split_impl<P>>;

 public:
  /// # `pretext::starts_with()`, `pretext::ends_with()`
  ///
  /// Checks if this string starts with a particular substring.
  constexpr bool starts_with(best::rune prefix) const {
    return strip_prefix(prefix).has_value();
  }
  constexpr bool starts_with(const best::string_type auto& prefix) const {
    return strip_prefix(prefix).has_value();
  }

  /// # `pretext::strip_prefix()`
  ///
  /// If this string starts with `prefix` removes it and returns the rest;
  /// otherwise returns `best::none.
  constexpr best::option<pretext> strip_prefix(best::rune prefix) const;
  constexpr best::option<pretext> strip_prefix(
      const best::string_type auto& prefix) const;
  constexpr best::option<pretext> strip_prefix(
      best::callable<bool(rune)> auto&& pred) const;

  /// # `pretext::consume_prefix()`
  ///
  /// Like `strip_prefix()` but returns a bool on success and updates the span
  /// in-place.
  constexpr bool consume_prefix(best::rune prefix) {
    auto rest = strip_prefix(prefix);
    if (rest) *this = *rest;
    return rest.has_value();
  }
  constexpr bool consume_prefix(const best::string_type auto& prefix) {
    auto rest = strip_prefix(prefix);
    if (rest) *this = *rest;
    return rest.has_value();
  }

  constexpr bool consume_prefix(best::callable<bool(rune)> auto&& pred) {
    auto rest = strip_prefix(BEST_FWD(pred));
    if (rest) *this = *rest;
    return rest.has_value();
  }

  /// # `text::operator==`, `text::operator<=>`
  ///
  /// Strings can be compared regardless of encoding, and they may be compared
  /// with runes, too.
  constexpr bool operator==(rune) const;
  constexpr bool operator==(const string_type auto&) const;
  constexpr bool operator==(const pretext&) const = default;
  constexpr bool operator==(best::span<const code> span) const {
    return span_ == span;
  }
  constexpr bool operator==(const code* lit) const {
    return span_ == best::span<const code>::from_nul(lit);
  }

  constexpr best::ord operator<=>(rune) const;
  constexpr best::ord operator<=>(const string_type auto&) const;
  constexpr best::ord operator<=>(best::span<const code> span) const {
    return span_ <=> span;
  }
  constexpr best::ord operator<=>(const code* lit) const {
    return span_ <=> best::span<const code>::from_nul(lit);
  }

  // Make this into a best::string_type. Need to make this into a funny template
  // to avoid infinite recursion while checking
  // best::string_type<best::pretext>.
  constexpr friend const encoding& BestEncoding(auto, const auto& t)
    requires best::same<decltype(t), const pretext&>
  {
    return t.enc();
  }

 private:
  best::span<const code> span_{&empty, 0};
  [[no_unique_address]] encoding enc_;

  static constexpr code empty{};
};
template <best::encoding E>
pretext(E) -> pretext<E>;
template <best::string_type S>
pretext(const S&) -> pretext<best::encoding_type<S>>;

template <typename E>
class text<E>::rune_iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr text rest() const { return text_; }

  using BestIterArrow = void;

 private:
  friend text;
  friend best::iter<rune_iter_impl>;
  friend best::iter<rune_iter_impl&>;

  constexpr explicit rune_iter_impl(text text) : text_(std::move(text)) {}
  constexpr best::option<best::rune> next();
  constexpr best::size_hint size_hint() const;
  // constexpr size_t count() && { return BEST_MOVE(iter_).count(); }

  text text_;
};

template <typename E>
class text<E>::rune_index_iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr pretext rest() const { return iter_->rest(); }

  using BestIterArrow = void;

 private:
  friend pretext;
  friend best::iter<rune_index_iter_impl>;
  friend best::iter<rune_index_iter_impl&>;

  constexpr explicit rune_index_iter_impl(text text) : iter_(text.runes()) {}
  constexpr best::option<best::row<size_t, best::rune>> next();
  constexpr best::size_hint size_hint() const { return iter_.size_hint(); }
  constexpr size_t count() && { return BEST_MOVE(iter_).count(); }

  rune_iter iter_;
  size_t idx_ = 0;
};

template <typename E>
class pretext<E>::rune_iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr pretext rest() const { return iter_->rest(); }

  using BestIterArrow = void;

 private:
  friend pretext;
  friend best::iter<rune_iter_impl>;
  friend best::iter<rune_iter_impl&>;

  constexpr explicit rune_iter_impl(rune_try_iter iter) : iter_(iter) {}
  constexpr best::option<best::rune> next() {
    return iter_.next().map(
        [](auto r) { return r.ok().value_or(rune::Replacement); });
  }
  constexpr best::size_hint size_hint() const { return iter_.size_hint(); }
  constexpr size_t count() && { return BEST_MOVE(iter_).count(); }

  rune_try_iter iter_;
};

template <typename E>
class pretext<E>::rune_index_iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr pretext rest() const { return iter_->rest(); }

  using BestIterArrow = void;

 private:
  friend pretext;
  friend best::iter<rune_index_iter_impl>;
  friend best::iter<rune_index_iter_impl&>;

  constexpr explicit rune_index_iter_impl(rune_try_iter iter)
      : iter_(iter), size_(iter->rest().size()) {}
  constexpr best::option<best::row<size_t, best::rune>> next();
  constexpr best::size_hint size_hint() const { return iter_.size_hint(); }
  constexpr size_t count() && { return BEST_MOVE(iter_).count(); }

  rune_try_iter iter_;
  size_t size_;
};

template <typename E>
class pretext<E>::rune_try_iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr pretext rest() const { return text_; }

  using BestIterArrow = void;

 private:
  friend pretext;
  friend best::iter<rune_try_iter_impl>;
  friend best::iter<rune_try_iter_impl&>;

  constexpr explicit rune_try_iter_impl(pretext text) : text_(text) {}
  constexpr best::option<best::result<best::rune, best::encoding_error>> next();
  constexpr best::size_hint size_hint() const;
  // constexpr size_t count() && {
  //   TODO: implement this one we implement a count() extension for encodings.
  // }

  pretext text_;
};

template <typename E>
template <typename P>
class pretext<E>::split_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr pretext rest() const { return text_; }

  using BestIterArrow = void;

 private:
  friend pretext;
  friend best::iter<split_impl>;
  friend best::iter<split_impl&>;

  constexpr explicit split_impl(auto&& pat, pretext text)
      : pat_(best::in_place, BEST_FWD(pat)), text_(text) {}

  constexpr best::option<pretext> next() {
    if (done_) return best::none;
    if (auto found = text_.split_once(*pat_)) {
      text_ = found->second();
      return found->first();
    }
    done_ = true;
    auto rest = text_;
    text_.span_ = {};
    return rest;
  }

  constexpr best::size_hint size_hint() const {
    if (done_) return {0, 0};
    return {1, text_.size() + 1};
  }

  [[no_unique_address]] best::object<P> pat_;
  pretext text_;
  bool done_ = false;
};

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
namespace str_internal {
/// `str_internal::splits()`
///
/// Finds the split points for some needle. This is a low-level implementation
/// detail of the various search functions. They are designed to minimize the
/// number of templates that must be instantiated when `str::contains()` or a
/// similar high-level search function is called.

template <typename N, typename H>
constexpr std::array<size_t, 2> splits(best::pretext<N> haystack,
                                       best::pretext<H> needle) {
  if (needle.is_empty()) {
    return {0, haystack.size()};
  }

  if constexpr (best::byte_comparable<code<H>> &&
                haystack.About.is_self_syncing &&
                best::same_encoding_code<decltype(haystack),
                                         decltype(needle)>()) {
    auto found = haystack.as_codes().find(needle.as_codes());
    if (!found) return {-1, -1};
    return {{*found, *found + needle.size()}};
  }

  auto runes = haystack.try_runes();
  auto needle_suf = needle.runes();
  auto first = *needle_suf.next();

  while (!runes->rest().is_empty()) {
    // We need to know what index we were at before we find `first` in
    // runes, since that's the end of the prefix half.
    size_t before = 0;
    while (auto next = runes.next()) {
      if (!next->ok()) return {-1, -1};
      if (*next->ok() == first) break;
      before = haystack.size() - runes->rest().size();
    }

    // Check if we found what we're looking for.
    // TODO: Avoid strip_prefix here, since that instantiates option<pretext>.
    if (auto suf = runes->rest().strip_prefix(needle_suf->rest())) {
      return {before, haystack.size() - suf->size()};
    }
  }

  return {-1, -1};
}

template <typename E>
constexpr std::array<size_t, 2> splits(best::pretext<E> haystack,
                                       best::rune needle) {
  code<E> buf[E::About.max_codes_per_rune];
  auto encoded = needle.encode(buf, haystack.enc());
  return str_internal::splits(haystack,
                              best::pretext<E>(*encoded.ok(), haystack.enc()));
}

template <typename E>
constexpr std::array<size_t, 2> splits(best::pretext<E> haystack,
                                       best::callable<bool(rune)> auto&& pred) {
  size_t before = 0;
  auto runes = haystack.try_runes();
  while (auto next = runes.next()) {
    if (!*next) break;
    if (best::call(pred, **next)) {
      return {{before, haystack.size() - runes->rest().size()}};
    }
    before = haystack.size() - runes->rest().size();
  }
  return {-1, -1};
}
}  // namespace str_internal

template <typename E>
constexpr best::option<text<E>> text<E>::from(pretext data) {
  if (!rune::validate(data.as_codes(), data.enc())) {
    return best::none;
  }

  return text(unsafe("validated just above"), data);
}

template <typename E>
constexpr best::row<text<E>, pretext<E>> text<E>::from_partial(pretext data) {
  size_t split_at = 0;
  auto runes = data.runes();
  for (auto r : runes) {
    if (r->err()) break;
    split_at = data.size() - runes->rest();
  }

  unsafe u("the loop above did an implicit bounds check for us");
  return {
      text(unsafe("this is as far as we verified"),
           data.at(u, {.end = split_at})),
      data.at(u, {.start = split_at}),
  };
}

template <typename E>
constexpr bool text<E>::is_rune_boundary(size_t idx) const {
  return rune::is_boundary(*this, idx, enc());
}

template <typename E>
constexpr text<E> text<E>::operator[](best::bounds::with_location range) const {
  // First, perform a bounds check.
  auto chunk = text_[range];

  auto at_boundary = is_rune_boundary(range.start) &&
                     is_rune_boundary(range.start + chunk.size());

  if (!at_boundary) {
    crash_internal::crash(
        {"string slice operation sliced through the middle of a character: "
         "{.start = %zu, .end = %zu}",
         range.where},
        range.start, range.start + chunk.size());
  }

  return text{unsafe("see boundary check above"), chunk};
}

template <typename E>
constexpr option<text<E>> text<E>::at(best::bounds range) const {
  auto chunk = text_.at(range);
  BEST_GUARD(chunk);

  auto at_boundary = is_rune_boundary(range.start) &&
                     is_rune_boundary(range.start + chunk->size());
  if (!at_boundary) return best::none;

  return text{unsafe("see boundary check above"), *chunk};
}

template <typename E>
constexpr best::option<size_t> text<E>::find(best::rune needle) const {
  auto [a, b] = str_internal::splits(best::pretext(*this), needle);
  if (a == -1) return best::none;
  return a;
}
template <typename E>
constexpr best::option<size_t> text<E>::find(
    const best::string_type auto& needle) const {
  auto [a, b] =
      str_internal::splits(best::pretext(*this), best::pretext(needle));
  if (a == -1) return best::none;
  return a;
}
template <typename E>
constexpr best::option<size_t> text<E>::find(
    best::callable<bool(rune)> auto&& pred) const {
  auto [a, b] = str_internal::splits(best::pretext(*this), BEST_FWD(pred));
  if (a == -1) return best::none;
  return a;
}

template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_once(
    best::rune needle) const {
  auto [a, b] = str_internal::splits(best::pretext(*this), needle);
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}

template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_once(
    const best::string_type auto& needle) const {
  auto [a, b] =
      str_internal::splits(best::pretext(*this), best::pretext(needle));
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}
template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_once(
    best::callable<bool(rune)> auto&& pred) const {
  auto [a, b] = str_internal::splits(best::pretext(*this), BEST_FWD(pred));
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}

template <typename E>
constexpr auto text<E>::split(best::rune needle) const {
  return text_.split(needle).map([](auto pre) {
    return text{unsafe("valid because text_ was already valid"), pre};
  });
}
template <typename E>
constexpr auto text<E>::split(const best::string_type auto& needle) const {
  return text_.split(needle).map([](auto pre) {
    return text{unsafe("valid because text_ was already valid"), pre};
  });
}
template <typename E>
constexpr auto text<E>::split(best::callable<bool(rune)> auto&& pred) const {
  return text_.split(BEST_MOVE(pred)).map([](auto pre) {
    return text{unsafe("valid because text_ was already valid"), pre};
  });
}

template <typename E>
constexpr best::option<best::rune> text<E>::rune_iter_impl::next() {
  if (text_.is_empty()) return best::none;
  auto span = text_.as_codes();
  rune next = rune::decode(&span, text_.enc())
                  .ok()
                  .value(unsafe("text_ is well-encoded"));
  text_ = text(unsafe("text_ is well-encoded"), {span, text_.enc()});
  return option(next);
}
template <typename E>
constexpr best::option<best::row<size_t, best::rune>>
text<E>::rune_index_iter_impl::next() {
  size_t cur_len = iter_->rest().size();
  auto next = iter_.next();
  BEST_GUARD(next);

  size_t idx = idx_;
  idx_ += cur_len - iter_->rest().size();
  return {{idx, *next}};
}
template <typename E>
constexpr best::option<best::row<size_t, best::rune>>
pretext<E>::rune_index_iter_impl::next() {
  auto next = iter_.next();
  BEST_GUARD(next);

  return {{size_ - rest().size(), next->ok().value_or(rune::Replacement)}};
}
template <typename E>
constexpr best::option<best::result<best::rune, best::encoding_error>>
pretext<E>::rune_try_iter_impl::next() {
  if (text_.is_empty()) return best::none;

  auto next = rune::decode(&text_.span_, text_.enc());
  if (!next) {
    // If we encountered an error, we can skip over a single code only if this
    // is a self-syncing encoding; otherwise we need to stop here.
    if (About.is_self_syncing) {
      text_ = text_[{.start = 1}];
    } else {
      text_ = text_[{.end = 0}];
    }
  }
  return next;
}

template <typename E>
constexpr best::size_hint text<E>::rune_iter_impl::size_hint() const {
  auto codes = text_.size();
  // Regardless of encoding, there are no invalid runes, so we can assume we
  // will yield all codes.
  return {best::ceildiv(codes, About.max_codes_per_rune).wrap(), codes};
}
template <typename E>
constexpr best::size_hint pretext<E>::rune_try_iter_impl::size_hint() const {
  auto codes = text_.size();

  if (About.is_self_syncing) {
    // If this is a self-synching encoding, we will yield every code: in the
    // lower bound, every rune is maximally encoded; otherwise, each code is
    // a rune.
    return {best::ceildiv(codes, About.max_codes_per_rune).wrap(), codes};
  } else {
    // Non-self-syncing encodings will give up on the first error. We will
    // always yield at least one value if there are codes left.
    return {codes > 0, codes};
  }
}

template <typename E>
constexpr best::option<pretext<E>> pretext<E>::strip_prefix(
    best::rune r) const {
  // TODO add an extension point so that e.g. UTF-8 can avoid the decode
  // cost here.
  auto haystack = try_runes();
  if (haystack.next() == r) {
    return haystack->rest();
  }
  return best::none;
}

template <typename E>
constexpr best::option<pretext<E>> pretext<E>::strip_prefix(
    const best::string_type auto& s) const {
  if constexpr (best::is_pretext<best::as_auto<decltype(s)>>) {
    if constexpr (best::same_encoding_code<pretext, decltype(s)>() &&
                  About.is_self_syncing) {
      auto rest = span_.strip_prefix(s.as_codes());
      BEST_GUARD(rest);
      return pretext{*rest, enc()};
    }

    auto haystack = try_runes();
    auto needle = s.runes();

    while (auto r1 = needle.next()) {
      auto r2 = haystack.next();
      if (r2.is_empty() || r2->err() || *r1 != *r2->ok()) return best::none;
    }
    return haystack->rest();
  } else {
    return strip_prefix(best::pretext(s));
  }
}

template <typename E>
constexpr best::option<pretext<E>> pretext<E>::strip_prefix(
    best::callable<bool(rune)> auto&& pred) const {
  auto haystack = try_runes();
  if (best::call(BEST_FWD(pred), haystack.next())) {
    return haystack->rest();
  }
  return best::none;
}

template <typename E>
constexpr best::option<size_t> pretext<E>::find(best::rune needle) const {
  auto [a, b] = str_internal::splits(*this, needle);
  if (a == -1) return best::none;
  return a;
}
template <typename E>
constexpr best::option<size_t> pretext<E>::find(
    const best::string_type auto& needle) const {
  auto [a, b] = str_internal::splits(*this, best::pretext(needle));
  if (a == -1) return best::none;
  return a;
}
template <typename E>
constexpr best::option<size_t> pretext<E>::find(
    best::callable<bool(rune)> auto&& pred) const {
  auto [a, b] = str_internal::splits(*this, BEST_FWD(pred));
  if (a == -1) return best::none;
  return a;
}

template <typename E>
constexpr best::option<best::row<pretext<E>, pretext<E>>>
pretext<E>::split_once(best::rune needle) const {
  auto [a, b] = str_internal::splits(*this, needle);
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}

template <typename E>
constexpr best::option<best::row<pretext<E>, pretext<E>>>
pretext<E>::split_once(const best::string_type auto& needle) const {
  auto [a, b] = str_internal::splits(*this, best::pretext(needle));
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}
template <typename E>
constexpr best::option<best::row<pretext<E>, pretext<E>>>
pretext<E>::split_once(best::callable<bool(rune)> auto&& pred) const {
  auto [a, b] = str_internal::splits(*this, BEST_FWD(pred));
  if (a == -1) return best::none;
  best::unsafe u("splits() does a bounds-check for us");
  return {{at(u, {.end = a}), at(u, {.start = b})}};
}

template <typename E>
constexpr auto pretext<E>::split(best::rune needle) const {
  return split_iter<rune>(split_impl<rune>(needle, *this));
}
template <typename E>
constexpr auto pretext<E>::split(const best::string_type auto& needle) const {
  pretext text = needle;
  return split_iter<decltype(text)>(split_impl<decltype(text)>(text, *this));
}
template <typename E>
constexpr auto pretext<E>::split(best::callable<bool(rune)> auto&& pred) const {
  return split_iter<decltype(pred)>(
      split_impl<decltype(pred)>(BEST_FWD(pred), *this));
}

template <typename E>
constexpr bool pretext<E>::operator==(rune r) const {
  return strip_prefix(r).has_value(&pretext::is_empty);
}

template <typename E>
constexpr bool pretext<E>::operator==(const string_type auto& s) const {
  return strip_prefix(s).has_value(&pretext::is_empty);
}

template <typename E>
constexpr best::ord pretext<E>::operator<=>(rune r) const {
  if (is_empty()) return best::ord::less;

  auto iter = runes();
  if (auto result = *iter.next() <=> r; result != 0) {
    return result;
  }

  return 0 <=> iter->rest().size();
}

template <typename E>
constexpr best::ord pretext<E>::operator<=>(const string_type auto& str) const {
  if constexpr (best::is_pretext<best::as_auto<decltype(str)>>) {
    if constexpr (best::byte_comparable<code> && About.is_self_syncing &&
                  best::same_encoding_code<pretext, decltype(str)>()) {
      return span_ <=> str.as_codes();
    }

    auto lhs = runes();
    auto rhs = str.runes();
    while (true) {
      auto r1 = lhs.next();
      auto r2 = rhs.next();
      if (!r1 && !r2) return best::ord::equal;

      // Note: this comparison relies on best::none < best::option(r) for all
      // runes.
      if (auto result = r1 <=> r2; result != 0) return result;
    }
  } else {
    return *this <=> best::pretext(str);
  }
}

}  // namespace best

#endif  // BEST_TEXT_STR_H_
