#ifndef BEST_STRINGS_UTF_H_
#define BEST_STRINGS_UTF_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/container/span.h"
#include "best/strings/internal/utf.h"
#include "best/strings/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-8, UTF-16, UTF-32, and WTF-8 encodings.

namespace best {
/// # `best::utf8`
///
/// A `best::encoding` representing UTF-8.
struct utf8 final {
  using code = char;  // Not char8_t because the standard messed up.
  using state = utf8;
  using self_syncing = void;

  static constexpr size_t MaxCodesPerRune = 4;

  static constexpr best::option<rune> read_rune(utf8,
                                                best::span<const char>& input) {
    if (auto result = best::utf_internal::decode8(input)) {
      input = input[{.start = result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
  }

  static constexpr bool write_rune(utf8, best::span<char>& output, rune rune) {
    if (auto result = best::utf_internal::encode8(output, rune)) {
      output = output[{.start = *result}];
      return true;
    }
    return false;
  }

  constexpr bool operator==(const utf8&) const = default;
};

/// # `best::wtf8`
///
/// A best::encoding representing WTF-8 (Wobbly Transformation Format).
///
/// Its only difference with UTF-8 is that it allows decoded runes to be
/// unpaired surrogates (in the range U+D800 to U+DFFF).
struct wtf8 final {
  using code = char;  // Not char8_t because the standard messed up.
  using state = utf8;
  using self_syncing = void;

  static constexpr size_t MaxCodesPerRune = 4;

  static constexpr best::option<rune> read_rune(wtf8,
                                                best::span<const char>& input) {
    if (auto result = best::utf_internal::decode8(input)) {
      input = input[{.start = result->first}];
      return rune::from_int_allow_surrogates(result->second);
    }
    return best::none;
  }

  static constexpr bool write_rune(wtf8, best::span<char>& output, rune rune) {
    return utf8::write_rune({}, output, rune);
  }

  constexpr bool operator==(const wtf8&) const = default;
};

/// # `best::utf16`
///
/// A best::encoding representing UTF-16.
struct utf16 final {
  using code = char16_t;
  using state = utf16;
  using self_syncing = void;

  static constexpr size_t MaxCodesPerRune = 2;

  static constexpr best::option<rune> read_rune(
      utf16, best::span<const char16_t>& input) {
    if (auto result = best::utf_internal::decode16(input)) {
      input = input[{.start = result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
  }

  static constexpr bool write_rune(utf16, best::span<char16_t>& output,
                                   rune rune) {
    if (auto result = best::utf_internal::encode16(output, rune)) {
      output = output[{.start = *result}];
      return true;
    }
    return false;
  }

  bool operator==(const utf16&) const = default;
};

/// # `best::utf32`
///
/// A best::encoding representing UTF-32.
struct utf32 final {
  using code = char32_t;
  using state = utf32;
  using self_syncing = void;

  static constexpr size_t MaxCodesPerRune = 1;

  static constexpr best::option<rune> read_rune(
      utf32, best::span<const char32_t>& input) {
    if (auto next = input.take_first(1)) {
      return rune::from_int((*next)[0]);
    }
    return best::none;
  }

  static constexpr bool write_rune(utf32, best::span<char32_t>& output,
                                   rune rune) {
    if (auto next = output.take_first(1)) {
      (*next)[0] = rune;
      return true;
    }
    return false;
  }

  constexpr bool operator==(const utf32&) const = default;
};

constexpr const utf8& BestEncoding(auto, const std::string&) {
  return best::val<utf8{}>::value;
}
constexpr const utf8& BestEncoding(auto, const std::string_view&) {
  return best::val<utf8{}>::value;
}
}  // namespace best

#endif  // BEST_STRINGS_UTF_H_