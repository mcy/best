#ifndef BEST_TEXT_STR_H_
#define BEST_TEXT_STR_H_

#include <cstddef>

#include "best/container/span.h"
#include "best/memory/bytes.h"
#include "best/text/encoding.h"
#include "best/text/rune.h"
#include "best/text/utf.h"

//! Unicode strings.
//!
//! best::text is a Unicode string, i.e., an text sequence of best::runes.
//! It is essentially std::basic_string_view with a nicer API (compare with
//! best::span).
//!
//! best::str, best::str16, and best::str32 are type aliases corresponding to
//! the UTF-8/16/32 specializations of the above.

namespace best {
template <typename>
class text;

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
#define BEST_IS_VALID_LITERAL(literal_, enc_)                              \
  BEST_ENABLE_IF_CONSTEXPR(literal_)                                       \
  BEST_ENABLE_IF(                                                          \
      rune::validate(best::span(literal_, std::size(literal_) - 1), enc_), \
      "string literal must satisfy rune::validate() for the chosen encoding")

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

  /// # `text::About`
  ///
  /// Metadata about this strings's encoding.
  static constexpr best::encoding_about About = E::About;

  /// # `text::text()`
  ///
  /// Creates a new, empty string with the given encoding.
  constexpr explicit text(encoding enc = {})
      : text(in_place, best::span(&empty, 0), std::move(enc)) {}

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
      : text(in_place, span(lit, n - 1), std::move(enc)) {}
  template <size_t n>
  constexpr text(const code (&lit)[n]) BEST_IS_VALID_LITERAL(lit, encoding{})
      : text(in_place, span(lit, n - 1), encoding{}) {}

  /// # `text::text(unsafe)`
  ///
  /// Creates a new string from some other `best::string_type`.
  ///
  /// Crashes if the string is not correctly text::
  constexpr explicit text(unsafe, best::span<const code> data,
                          encoding enc = {})
      : text(in_place, data, std::move(enc)) {}

  /// # `text::from()`
  ///
  /// Creates a new string by parsing it from a span of potentially invalid
  /// characters.
  constexpr static best::option<text> from(best::span<const code> data,
                                           encoding enc = {});
  constexpr static best::option<text> from(const string_type auto& that) {
    return from(span(std::data(that), std::size(that)),
                best::encoding_of(that));
  }

  /// # `text::from_partial()`
  ///
  /// Creates a new string by decoding the longest valid prefix of `data`.
  /// Returns the valid prefix, and the rest of `data`.
  constexpr static best::row<text, best::span<const best::code<E>>>
  from_partial(best::span<const code> data, encoding enc = {});

  /// # `text::from_nul()`
  ///
  /// Creates a new string by parsing it from a NUL-terminated string. It must
  /// end in `code{0}`. If `data == `nullptr`, returns an empty string.
  constexpr static best::option<text> from_nul(const code* data,
                                               encoding enc = {}) {
    return from(best::span<const code>::from_nul(data), std::move(enc));
  }

  /// # `text::data()`
  ///
  /// Returns the string's data pointer.
  /// This value is never null.
  constexpr const code* data() const { return span_.data(); }

  /// # `text::is_empty()`
  ///
  /// Checks whether the string is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// # `text::size()`
  ///
  /// Returns the size of the string, in code units.
  constexpr size_t size() const { return span_.size(); }

  /// # `text::enc()`
  ///
  /// Returns the underlying text encoding.
  constexpr const encoding& enc() const { return enc_; }

  /// # `text::as_codes()`
  ///
  /// Returns the span of code units that backs this string.
  constexpr best::span<const code> as_codes() const { return span_; }

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
    return {in_place, span_.at(u, range)};
  }

  /// # `text::starts_with()`
  ///
  /// Checks whether this string begins with the specifies substring or rune.
  constexpr bool starts_with(rune r) const {
    return trim_prefix(r).has_value();
  }
  constexpr bool starts_with(const string_type auto& s) const {
    return trim_prefix(s).has_value();
  }
  constexpr bool starts_with(best::callable<bool(rune)> auto&& p) const {
    return trim_prefix(p).has_value();
  }

  /// # `text::trim_prefix()`
  ///
  /// If this string starts with the given prefix, returns a copy of this string
  /// with that prefix removed.
  constexpr best::option<text> trim_prefix(rune) const;
  constexpr best::option<text> trim_prefix(const string_type auto&) const;
  constexpr best::option<text> trim_prefix(
      best::callable<bool(rune)> auto&&) const;

  /// # `text::consume_prefix()`
  ///
  /// If this string starts with the given prefix, returns `true` and updates
  /// this string to the result of `trim_prefix()`. Otherwise, returns `false`
  /// and leaves this string unchanged.
  constexpr bool consume_prefix(rune r) {
    auto suffix = trim_prefix(r);
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }
  constexpr bool consume_prefix(const string_type auto& s) {
    auto suffix = trim_prefix(s);
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }
  constexpr bool consume_prefix(best::callable<bool(rune)> auto&& p) {
    auto suffix = trim_prefix(BEST_FWD(p));
    if (suffix) *this = *suffix;
    return suffix.has_value();
  }

  /// # `text::contains()`
  ///
  /// Whether this string contains a particular substring or rune..
  constexpr bool contains(rune r) const { return !!find(r); }
  constexpr bool contains(const string_type auto& s) const { return !!find(s); }
  constexpr bool contains(best::callable<bool(rune)> auto&& s) const {
    return !!find(s);
  }

  /// # `text::find()`
  ///
  /// Finds the first occurrence of a substring or rune within this string.
  ///
  /// If any invalidly text characters are encountered during the search, in
  /// either the haystack or the needle, this function returns `best::none`.
  constexpr best::option<size_t> find(rune r) const {
    return split_on(r).map([](auto r) { return r.first().size(); });
  }
  constexpr best::option<size_t> find(const string_type auto& s) const {
    return split_on(s).map([](auto r) { return r.first().size(); });
  }
  constexpr best::option<size_t> find(
      best::callable<bool(rune)> auto&& p) const {
    return split_on(BEST_FWD(p)).map([](auto r) { return r.first().size(); });
  }

  /// # `text::split_at()`
  ///
  /// Splits this string into two on the given index. If the desired split point
  /// is out of bounds, returns `best::none`.
  constexpr best::option<best::row<text, text>> split_at(size_t n) const {
    auto prefix = at({.end = n});
    if (!prefix) return best::none;
    return {{*prefix, operator[]({.start = n})}};
  }

  /// # `text::split_on()`
  ///
  /// Splits this string into two on the first occurrence of the given substring
  /// or rune, or when the callback returns true. If the desired split point
  /// is not found, returns `best::none`.
  constexpr best::option<best::row<text, text>> split_on(rune) const;
  constexpr best::option<best::row<text, text>> split_on(
      const string_type auto&) const;
  constexpr best::option<best::row<text, text>> split_on(
      best::callable<bool(rune)> auto&& p) const;

  /// # `text::break_off()`
  ///
  /// Parses the first rune in this string and returns it and a substring with
  /// that rune removed. Returns `best::none` if the string is empty.
  constexpr best::option<best::row<rune, text>> break_off() const;

  /// # `text::rune_iter`, `text::runes()`.
  ///
  /// An iterator over the runes of a `best::text`.
  ///
  /// A `best::text` may point to invalidly-text data. If the encoding is
  /// self-synchronizing, the stream of Unicode characters is interpreted as
  /// replacing each invalid code unit with a Unicode replacement character
  /// (U+FFFD). If the encoding is not self-synchronizing, the stream is
  /// interpreted to end at that position, with a replacement character.
  constexpr rune::iter<E> runes() const { return rune::iter<E>(span_, enc_); }

  /// # `text::operator==`, `text::operator<=>`
  ///
  /// Strings can be compared regardless of encoding, and they may be compared
  /// with runes, too.
  constexpr bool operator==(rune) const;
  constexpr bool operator==(const string_type auto&) const;
  constexpr bool operator==(const text&) const = default;
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

  // Make this into a best::string_type.
  constexpr friend const encoding& BestEncoding(auto, const text& t) {
    return t.enc();
  }

 private:
  constexpr explicit text(best::in_place_t, best::span<const code> span,
                          encoding enc)
      : span_(span), enc_(std::move(enc)) {
    if (span_.data() == nullptr) span_ = {&empty, 0};
  }

  constexpr bool can_memeq(const auto& that) const {
    return !std::is_constant_evaluated() &&
           (best::equal(this, best::addr(that)) ||
            best::same_encoding(*this, that));
  }

  constexpr bool can_memcmp(const auto& that) const {
    return can_memeq(that) && best::byte_comparable<code> &&
           E::About.is_lexicographic;
  }

  constexpr bool can_memmem(const auto& that) const {
    return can_memeq(that) && best::byte_comparable<code> &&
           E::About.is_self_syncing;
  }

  best::span<const code> span_{&empty, 0};
  [[no_unique_address]] encoding enc_;

  static constexpr code empty{};
};
}  // namespace best

/******************************************************************************/

///////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! /////////////////////

/******************************************************************************/

namespace best {
template <typename E>
constexpr best::option<text<E>> text<E>::from(best::span<const code> data,
                                              encoding enc) {
  if (!rune::validate(data, enc)) {
    return best::none;
  }

  return text(best::in_place, data, std::move(enc));
}

template <typename E>
constexpr best::row<text<E>, best::span<const code<E>>> text<E>::from_partial(
    best::span<const code> data, encoding enc) {
  auto orig = data;
  while (!data.is_empty()) {
    if (!rune::decode(&data, enc)) {
      return {text{in_place, orig[{.end = orig.size() - data.size()}],
                   std::move(enc)},
              data};
    }
  }
  return {text{in_place, orig, std::move(enc)}, {}};
}

template <typename E>
constexpr bool text<E>::is_rune_boundary(size_t idx) const {
  return rune::is_boundary(*this, idx, enc());
}

template <typename E>
constexpr text<E> text<E>::operator[](best::bounds::with_location range) const {
  // First, perform a bounds check.
  auto chunk = span_[range];

  auto at_boundary = is_rune_boundary(range.start) &&
                     is_rune_boundary(range.start + chunk.size());

  if (!at_boundary) {
    crash_internal::crash(
        {"string slice operation sliced through the middle of a character: "
         "{.start = %zu, .end = %zu}",
         range.where},
        range.start, range.start + chunk.size());
  }

  return text{in_place, chunk, enc()};
}

template <typename E>
constexpr option<text<E>> text<E>::at(best::bounds range) const {
  auto chunk = span_.at(range);
  if (!chunk) return best::none;

  auto at_boundary = is_rune_boundary(range.start) &&
                     is_rune_boundary(range.start + chunk->size());
  if (!at_boundary) return best::none;

  return text{in_place, *chunk, enc()};
}

template <typename E>
constexpr option<text<E>> text<E>::trim_prefix(rune r) const {
  return trim_prefix([=](rune r2) { return r == r2; });
}

template <typename E>
constexpr option<text<E>> text<E>::trim_prefix(
    const string_type auto& str) const {
  rune::iter needle(str);
  rune::iter haystack(*this);

  if constexpr (best::same_encoding_code<text, decltype(str)>()) {
    if (can_memeq(str)) {
      auto that = needle.rest();
      auto prefix = span_.at({.end = that.size()});
      if (!prefix) return best::none;
      if (!best::equate_bytes(*prefix, that)) return none;
      return operator[]({.start = that.size()});
    }
    return best::none;
  }

  while (auto r1 = needle.next()) {
    if (r1 != haystack.next()) return best::none;
  }
  return text{in_place, haystack.rest(), enc()};
}

template <typename E>
constexpr option<text<E>> text<E>::trim_prefix(
    best::callable<bool(rune)> auto&& r) const {
  return break_off().then([&](auto x) -> option<text> {
    if (best::call(BEST_FWD(r), x.first())) return x.second();
    return best::none;
  });
}

template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_on(
    rune r1) const {
  if (can_memmem(*this)) {
    code buf[About.max_codes_per_rune];
    auto that = r1.encode(buf, enc());
    if (!that) return best::none;

    auto idx = best::search_bytes(span_, *that);
    if (!idx) return best::none;
    return {{
        text(in_place, span_[{.end = *idx}], enc()),
        text(in_place, span_[{.start = *idx + that->size()}], enc()),
    }};
  }

  return split_on([=](rune r2) { return r1 == r2; });
}

template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_on(
    const string_type auto& str) const {
  rune::iter haystack_start(*this);
  rune::iter needle_start(str);

  if constexpr (best::same_encoding_code<text, decltype(str)>()) {
    if (can_memmem(str)) {
      best::span that = needle_start.rest();
      auto idx = best::search_bytes(span_, that);
      if (!idx) return best::none;
      return {{
          text(in_place, span_[{.end = *idx}], enc()),
          text(in_place, span_[{.start = *idx + that.size()}], enc()),
      }};
    }
  }

  auto haystack = haystack_start;
  auto needle = needle_start;
  while (auto n = needle.next()) {
    if (haystack.next() == n) continue;

    if (!haystack_start.next()) return best::none;
    haystack = haystack_start;
    needle = needle_start;
  }

  size_t start = size() - haystack_start.rest().size();
  size_t end = size() - haystack.rest().size();

  return {{
      text(in_place, span_[{.end = start}], enc()),
      text(in_place, span_[{.start = end}], enc()),
  }};
}

template <typename E>
constexpr best::option<best::row<text<E>, text<E>>> text<E>::split_on(
    best::callable<bool(rune)> auto&& pred) const {
  best::rune::iter iter(*this);
  size_t prev = 0;
  while (auto next = iter.next()) {
    if (!best::call(BEST_FWD(pred), *next)) {
      prev = size() - iter.rest().size();
      continue;
    }

    return {{
        text(in_place, span_[{.end = prev}], enc()),
        text(in_place, iter.rest(), enc()),
    }};
  }

  return best::none;
}

template <typename E>
constexpr bool text<E>::operator==(rune r) const {
  if (is_empty()) return false;
  auto [r2, rest] = *break_off();
  return rest.is_empty() && r == r2;
}

template <typename E>
constexpr bool text<E>::operator==(const string_type auto& s) const {
  return trim_prefix(s).has_value(&text::is_empty);
}

template <typename E>
constexpr best::ord text<E>::operator<=>(rune r) const {
  if (is_empty()) return best::ord::less;

  auto [r2, rest] = *break_off();
  if (auto result = r <=> r2; result != 0) {
    return result;
  }
  return 0 <=> rest;
}

template <typename E>
constexpr best::ord text<E>::operator<=>(const string_type auto& str) const {
  rune::iter a(*this);
  rune::iter b(str);

  if constexpr (best::same_encoding_code<text, decltype(str)>()) {
    if (can_memcmp(str)) return a.rest() <=> b.rest();
  }

  while (true) {
    auto r1 = a.next();
    auto r2 = b.next();
    if (r1.is_empty() && r2.is_empty()) {
      return best::ord::equal;
    }

    if (auto result = r1 <=> r2; result != 0) {
      return result;
    }
  }
}

template <typename E>
constexpr best::option<best::row<rune, text<E>>> text<E>::break_off() const {
  if (is_empty()) return best::none;

  auto suffix = *this;
  auto rune = rune::decode(&suffix.span_, enc());
  return {{*rune, suffix}};
}
}  // namespace best

#endif  // BEST_TEXT_STR_H_