#ifndef BEST_TEXT_UTF_H_
#define BEST_TEXT_UTF_H_

#include <cstddef>

#include "best/container/option.h"
#include "best/container/span.h"
#include "best/text/internal/utf.h"
#include "best/text/rune.h"

//! Encodings for the "Unicode Transformation Formats".
//!
//! This header defines the UTF-8, UTF-16, UTF-32, and WTF-8 encodings.

namespace best {
/// # `best::utf8`
///
/// A `best::encoding` representing UTF-8.
struct utf8 final {
  using code = char;  // Not char8_t because the standard messed up.
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 4,
      .is_self_syncing = true,
      .is_lexicographic = true,
  };

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return input.size() == idx || input.at(idx).has_value([](char c) {
      return best::leading_ones(c) != 1;
    });
  }

  static constexpr bool encode(best::span<char>* output, rune rune) {
    if (auto result = best::utf_internal::encode8(*output, rune)) {
      *output = (*output)[{.start = *result}];
      return true;
    }
    return false;
  }

  static constexpr best::option<rune> decode(best::span<const char>* input) {
    if (auto result = best::utf_internal::decode8(*input)) {
      *input = (*input)[{.start = result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
  }

  static constexpr best::option<rune> undecode(best::span<const char>* input) {
    if (auto result = best::utf_internal::undecode8(*input)) {
      *input = (*input)[{.end = input->size() - result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
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
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 4,
      .is_self_syncing = true,
      .is_lexicographic = true,
  };

  static constexpr bool is_boundary(best::span<const char> input, size_t idx) {
    return utf8::is_boundary(input, idx);
  }

  static constexpr bool encode(best::span<char>* output, rune rune) {
    return utf8::encode(output, rune);
  }

  static constexpr best::option<rune> decode(best::span<const char>* input) {
    if (auto result = best::utf_internal::decode8(*input)) {
      *input = (*input)[{.start = result->first}];
      return rune::from_int_allow_surrogates(result->second);
    }
    return best::none;
  }

  static constexpr best::option<rune> undecode(best::span<const char>* input) {
    if (auto result = best::utf_internal::undecode8(*input)) {
      *input = (*input)[{.end = input->size() - result->first}];
      return rune::from_int_allow_surrogates(result->second);
    }
    return best::none;
  }

  constexpr bool operator==(const wtf8&) const = default;
};

/// # `best::utf16`
///
/// A best::encoding representing UTF-16.
struct utf16 final {
  using code = char16_t;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 2,
      .is_self_syncing = true,
  };

  static constexpr bool is_boundary(best::span<const char16_t> input,
                                    size_t idx) {
    return input.size() == idx ||
           input.at(idx)
               .then([](char16_t c) {
                 return rune::from_int_allow_surrogates(c);
               })
               .has_value([](rune r) { return !r.is_low_surrogate(); });
  }

  static constexpr bool encode(best::span<char16_t>* output, rune rune) {
    if (auto result = best::utf_internal::encode16(*output, rune)) {
      *output = (*output)[{.start = *result}];
      return true;
    }
    return false;
  }

  static constexpr best::option<rune> decode(
      best::span<const char16_t>* input) {
    if (auto result = best::utf_internal::decode16(*input)) {
      *input = (*input)[{.start = result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
  }

  static constexpr best::option<rune> undecode(
      best::span<const char16_t>* input) {
    if (auto result = best::utf_internal::undecode16(*input)) {
      *input = (*input)[{.end = input->size() - result->first}];
      return rune::from_int(result->second);
    }
    return best::none;
  }

  bool operator==(const utf16&) const = default;
};

/// # `best::utf32`
///
/// A best::encoding representing UTF-32.
struct utf32 final {
  using code = char32_t;
  static constexpr best::encoding_about About{
      .max_codes_per_rune = 1,
      .is_self_syncing = true,
      .is_lexicographic = true,
  };

  static constexpr bool is_boundary(best::span<const char32_t> input,
                                    size_t idx) {
    return idx <= input.size();
  }

  static constexpr bool encode(best::span<char32_t>* output, rune rune) {
    if (auto next = output->take_first(1)) {
      (*next)[0] = rune;
      return true;
    }
    return false;
  }

  static constexpr best::option<rune> decode(
      best::span<const char32_t>* input) {
    if (auto next = input->take_first(1)) {
      return rune::from_int((*next)[0]);
    }
    return best::none;
  }

  static constexpr best::option<rune> undecode(
      best::span<const char32_t>* input) {
    if (auto next = input->take_last(1)) {
      return rune::from_int((*next)[0]);
    }
    return best::none;
  }

  constexpr bool operator==(const utf32&) const = default;
};

constexpr const utf8& BestEncoding(auto, const std::string&) {
  return best::val<utf8{}>::value;
}
constexpr const utf8& BestEncoding(auto, const std::string_view&) {
  return best::val<utf8{}>::value;
}
template <size_t n>
constexpr const utf8& BestEncoding(auto, const char (&)[n]) {
  return best::val<utf8{}>::value;
}

constexpr const utf16& BestEncoding(auto, const std::u16string&) {
  return best::val<utf16{}>::value;
}
constexpr const utf16& BestEncoding(auto, const std::u16string_view&) {
  return best::val<utf16{}>::value;
}
template <size_t n>
constexpr const utf16& BestEncoding(auto, const char16_t (&)[n]) {
  return best::val<utf16{}>::value;
}

constexpr const utf32& BestEncoding(auto, const std::u32string&) {
  return best::val<utf32{}>::value;
}
constexpr const utf32& BestEncoding(auto, const std::u32string_view&) {
  return best::val<utf32{}>::value;
}
template <size_t n>
constexpr const utf32& BestEncoding(auto, const char32_t (&)[n]) {
  return best::val<utf32{}>::value;
}

}  // namespace best

#endif  // BEST_TEXT_UTF_H_