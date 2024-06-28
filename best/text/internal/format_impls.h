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

#ifndef BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_
#define BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_

#include <cstddef>
#include <type_traits>

#include "best/meta/reflect.h"
#include "best/text/rune.h"
#include "best/text/str.h"
#include "best/text/utf16.h"

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

void BestFmt(auto& fmt, char8_t value) { fmt.format(*rune::from_int(value)); }
template <typename T>
constexpr void BestFmtQuery(auto& query, char8_t* range) {
  query.requires_debug = false;
  query.supports_width = true;
}

void BestFmt(auto& fmt, char16_t value) {
  fmt.format(*rune::from_int_allow_surrogates(value));
}
template <typename T>
constexpr void BestFmtQuery(auto& query, char16_t* range) {
  query.requires_debug = false;
  query.supports_width = true;
}

void BestFmt(auto& fmt, char32_t value) {
  if (auto r = rune::from_int_allow_surrogates(value)) {
    fmt.format(*r);
  } else if (fmt.current_spec().debug) {
    fmt.format("'<U+{:X}>'", unsigned(value));
  } else {
    fmt.format("<U+{:X}>", unsigned(value));
  }
}
template <typename T>
constexpr void BestFmtQuery(auto& query, char32_t* range) {
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
      if (auto s = best::dependent<best::str16, T>::from_nul(value)) {
        fmt.format(*s);
        return;
      }
    }

    if constexpr (best::same<T, const char32_t>) {
      if (auto s = best::dependent<best::str32, T>::from_nul(value)) {
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

void BestFmt(auto& fmt, const best::string_type auto& s) {
  // Taken liberally from Rust's implementation of Formatter::pad().
  if constexpr (best::is_pretext<decltype(s)>) {
    auto str = s;
    if (fmt.current_spec().method == 'q' || fmt.current_spec().debug) {
      // Quoted string.
      fmt.write('"');
      for (rune r : str.runes()) {
        fmt.format("{}", r.escaped());
      }
      fmt.write('"');
      return;
    }

    const auto& spec = fmt.current_spec();
    if (spec.width == 0 && !spec.prec) {
      // Fast path.
      fmt.write(str);
      return;
    }

    if (auto prec = spec.prec) {
      size_t max = *prec;
      auto runes = str.runes();
      for (auto r : runes) {
        (void)r;
        if (--max == 0) break;
      }
      str = str[{.end = str.size() - runes->rest().size()}];
    }

    if (spec.width == 0) {
      // No need to pad here!
      fmt.write(str);
      return;
    }

    // Otherwise, we need to figure out the number of characters and potentially
    // write some padding.
    size_t runes = str.runes().count();
    if (runes >= spec.width) {
      // No need to pad here either!
      fmt.write(str);
      return;
    }

    auto fill = fmt.current_spec().fill;
    auto [pre, post] =
        fmt.current_spec().compute_padding(runes, fmt.current_spec().Left);
    for (size_t i = 0; i < pre; ++i) fmt.write(fill);
    fmt.write(str);
    for (size_t i = 0; i < post; ++i) fmt.write(fill);
  } else {
    BestFmt(fmt, best::pretext(s));
  }
}

constexpr void BestFmtQuery(auto& query, best::string_type auto*) {
  query.requires_debug = false;
  query.supports_width = true;
  query.supports_prec = true;
  query.uses_method = [](auto r) { return r == 'q'; };
}

// These are *very* common instantiations that we can cheapen by making them
// extern templates. The corresponding explicit instantiation lives in
// format.cc.
extern template void BestFmt(best::formatter&, const char*);
extern template void BestFmt(best::formatter&, const char16_t*);
extern template void BestFmt(best::formatter&, const char32_t*);
extern template void BestFmt(best::formatter&, const best::pretext<utf8>&);
extern template void BestFmt(best::formatter&, const best::text<utf8>&);
extern template void BestFmt(best::formatter&, const best::pretext<utf16>&);
extern template void BestFmt(best::formatter&, const best::text<utf16>&);
extern template void BestFmt(best::formatter&, const best::pretext<wtf8>&);
extern template void BestFmt(best::formatter&, const best::text<wtf8>&);
extern template void BestFmt(best::formatter&, char);
extern template void BestFmt(best::formatter&, signed char);
extern template void BestFmt(best::formatter&, signed short);
extern template void BestFmt(best::formatter&, signed int);
extern template void BestFmt(best::formatter&, signed long);
extern template void BestFmt(best::formatter&, signed long long);
extern template void BestFmt(best::formatter&, unsigned char);
extern template void BestFmt(best::formatter&, unsigned short);
extern template void BestFmt(best::formatter&, unsigned int);
extern template void BestFmt(best::formatter&, unsigned long);
extern template void BestFmt(best::formatter&, unsigned long long);

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

void BestFmt(auto& fmt, const best::is_reflected_struct auto& value)
  requires(!requires { fmt.format(*std::begin(value)); })
{
  auto refl = best::reflect<decltype(value)>;
  auto rec = fmt.record(refl.name());
  refl.apply(
      [&](auto... field) { (rec.field(field.name(), value->*field), ...); });
}

void BestFmt(auto& fmt, const best::is_reflected_enum auto& value) {
  auto refl = best::reflect<decltype(value)>;
  refl.match(
      value,  //
      [&](auto val) { fmt.format("{}::{}", refl.name(), val.name()); },
      [&] {
        using U = std::underlying_type_t<best::as_auto<decltype(value)>>;
        fmt.format("{}({})", refl.name(), U(value));
      });
}

namespace format_internal {
using mark_as_used = void;
}  // namespace format_internal
}  // namespace best

#endif  // BEST_TEXT_INTERNAL_FORMAT_IMPLS_H_
