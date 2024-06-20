#ifndef BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_
#define BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_

#include <cstddef>

#include "best/text/rune.h"
#include "best/text/str.h"

//! Misc. formatting impls that don't have a clear other place to live.

namespace best {
void BestFmt(auto& fmt, bool value) {
  value ? fmt.write("true") : fmt.write("false");
}
template <typename T>
constexpr void BestFmtQuery(auto& query, bool* range) {
  query.requires_debug = false;
  query.supports_width = true;
}

void BestFmt(auto& fmt, std::byte value) { fmt.format(uint8_t(value)); }
constexpr void BestFmtQuery(auto& query, std::byte*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.uses_method = [](auto r) { return str("boxX").contains(r); };
}

template <typename T>
void BestFmt(auto& fmt, T* value) {
  // Custom handling for string literals.
  if (fmt.current_spec().method != 'p') {
    if (value == nullptr) {
      fmt.write("nullptr");
      return;
    }

    if constexpr (best::same<T, const char>) {
      if (auto s = best::str::from_nul(value)) {
        fmt.format(*s);
        return;
      }
    }

    if constexpr (best::same<T, const char16_t>) {
      if (auto s = best::str16::from_nul(value)) {
        fmt.format(*s);
        return;
      }
    }

    if constexpr (best::same<T, const char32_t>) {
      if (auto s = best::str32::from_nul(value)) {
        fmt.format(*s);
        return;
      }
    }
  }

  fmt.format("{:#x}", reinterpret_cast<uintptr_t>(value));
}

template <typename T>
constexpr void BestFmtQuery(auto& query, T** range) {
  query.requires_debug = false;
  query.uses_method = [](rune r) { return r == 'p'; };
}

template <typename A, typename B>
void BestFmt(auto& fmt, const best::row<A, B> value) {
  fmt.format(best::row<const A&, const B&>(value.first, value.second()));
}

void BestFmt(auto& fmt, integer auto value) {
  // Taken liberally from Rust's implementation of Formatter::pad_integral().

  // First, select the base and prefix.
  uint32_t base = 10;
  best::str prefix;
  bool uppercase = false;
  switch (fmt.current_spec().method.value_or()) {
    case 'b':
      base = 2;
      prefix = "0b";
      break;
    case 'o':
      base = 8;
      prefix = "0";  // In C++, the octal prefix is 0, not 0o
      break;
    case 'X':
      uppercase = true;
      [[fallthrough]];
    case 'x':
      base = 16;
      prefix = "0x";
      break;
  }

  bool negative = value < 0;
  if (negative) value = -value;

  // Construct the actual digits.
  char buf[128];
  size_t count = 0;
  do {
    rune r = *rune::from_digit(value % base, base);
    if (uppercase) r = r.to_ascii_upper();

    buf[128 - count++ - 1] = r;
    value /= base;
  } while (value != 0);

  best::str digits(unsafe("all characters are ascii"),
                   best::span(buf + 128 - count, count));

  size_t width = count;
  if (negative) ++width;
  if (fmt.current_spec().alt) width += prefix.size();

  auto write_prefix = [&] {
    if (negative) fmt.write('-');
    if (fmt.current_spec().alt) fmt.write(prefix);
  };

  size_t min_width = fmt.current_spec().width;
  if (min_width <= width) {
    write_prefix();
    fmt.write(digits);
  } else if (fmt.current_spec().sign_aware_padding) {
    write_prefix();

    auto padding = best::saturating_sub(min_width, width);
    for (size_t i = 0; i < padding; ++i) fmt.write('0');
    fmt.write(digits);
  } else {
    auto fill = fmt.current_spec().fill;
    auto [pre, post] =
        fmt.current_spec().compute_padding(width, fmt.current_spec().Right);

    for (size_t i = 0; i < pre; ++i) fmt.write(fill);
    write_prefix();
    fmt.write(digits);
    for (size_t i = 0; i < post; ++i) fmt.write(fill);
  }
}
constexpr void BestFmtQuery(auto& query, integer auto*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.uses_method = [](auto r) { return str("boxX").contains(r); };
}

// TODO: invent ranges/iterator traits.
template <typename R>
void BestFmt(auto& fmt, const R& range)
  requires(!best::string_type<R>) &&
          requires { fmt.format(*std::begin(range)); }
{
  // TODO: Printing for associative containers.
  auto list = fmt.list();
  for (const auto& value : range) {
    list.entry(value);
  }
}
template <typename R>
constexpr void BestFmtQuery(auto& query, R* range)
  requires(!best::string_type<R>) && requires { *std::begin(*range); }
{
  query = query.template of<decltype(*std::begin(*range))>;
  query.requires_debug = false;
}

namespace format_internal {
using mark_as_used = void;
}  // namespace format_internal
}  // namespace best

#endif  // BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_