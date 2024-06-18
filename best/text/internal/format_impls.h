#ifndef BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_
#define BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_

#include <cstddef>

#include "best/text/rune.h"
#include "best/text/str.h"

namespace best {
void BestFmt(auto& fmt, bool value) {
  value ? fmt.write("true") : fmt.write("false");
}

void BestFmt(auto& fmt, std::byte value) { fmt.format(uint8_t(value)); }

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
void BestFmt(auto& fmt, const std::pair<A, B> value) {
  fmt.format(best::row<const A&, const B&>(value.first, value.second));
}

void BestFmt(auto& fmt, integer auto value) {
  // TODO: Implement padding.
  uint32_t base = 10;
  bool uppercase = false;
  switch (fmt.current_spec().method.value_or()) {
    case 'b':
      base = 2;
      break;
    case 'o':
      base = 8;
      break;
    case 'X':
      uppercase = true;
      [[fallthrough]];
    case 'x':
      base = 16;
      break;
  }

  if (value < 0) {
    fmt.write("-");
    value = -value;
  }

  if (fmt.current_spec().alt) {
    switch (base) {
      case 2:
        fmt.write("0b");
        break;
      case 8:
        fmt.write("0");  // In C++, the octal prefix is 0, not 0o
        break;
      case 16:
        fmt.write("0x");
        break;
    }
  }

  do {
    rune r = *rune::from_digit(value % base, base);
    if (uppercase) r = r.to_ascii_upper();

    fmt.write(r);
    value /= base;
  } while (value != 0);
}
constexpr void BestFmtQuery(auto& query, integer auto*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.uses_method = [](auto r) { return str("boxX").contains(r); };
}

void BestFmt(auto& fmt, const best::string_type auto& str) {
  // TODO: Implement padding.
  if (fmt.current_spec().method != 'q' && !fmt.current_spec().debug) {
    fmt.write(str);
    return;
  }

  // Quoted string.
  fmt.write('"');
  for (rune r : rune::iter(str)) {
    fmt.format("{}", r.escaped());
  }
  fmt.write('"');
}
constexpr void BestFmtQuery(auto& query, best::string_type auto*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.uses_method = [](rune r) { return r == 'q'; };
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