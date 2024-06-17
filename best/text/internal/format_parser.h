#ifndef BEST_TEXT_INTERNAL_FORMAT_PARSER_H_
#define BEST_TEXT_INTERNAL_FORMAT_PARSER_H_

#include "best/base/fwd.h"
#include "best/math/conv.h"
#include "best/meta/ops.h"
#include "best/text/str.h"

namespace best::format_internal {
/// # `format_internal::visit_template()`
///
/// Parses a formatting template, calling the given callbacks to print a literal
/// chunk or to interpolate a variable, respectively.
///
/// Each callback can return a bool to signal error; the parser returns a
/// `false` if parsing, printing, or interpolation failed.
template <typename spec = best::format_spec>
constexpr bool visit_template(
    best::str templ, best::callable<bool(best::str)> auto&& print,
    best::callable<bool(size_t, const spec&)> auto&& interpolate) {
  size_t idx = 0;
  while (!templ.is_empty()) {
    // Skip to the next '{'.
    auto brace = templ.find([](rune r) { return r == '{' || r == '}'; });

    // Print everything up to the brace.
    if (brace != 0 &&
        !best::call(print, templ[{.end = brace.value_or(templ.size())}])) {
      return false;
    }
    if (!brace) return true;

    auto what = templ[{.start = *brace, .count = 1}];
    templ = templ[{.start = *brace + 1}];

    if (what == "}") {
      // If this is immediately followed by another }, it is a literal.
      if (templ.consume_prefix("}")) {
        if (!best::call(print, best::str("}"))) return false;
        continue;
      }
      // Any other result is an error.
      return false;
    }

    // If this is immediately followed by another {, it is a literal.
    if (templ.consume_prefix("{")) {
      if (!best::call(print, best::str("{"))) return false;
      continue;
    }

    // We are parsing this grammar:
    // '{'[idx][:[[fill]align]['#']['0'][width]['.' precision][method]['?']]'}'
    // align := '<' | '^' | '>'
    spec args{};

    // This is a fast-path for `{}`.
    if (templ.consume_prefix('}')) {
      if (!best::call(interpolate, idx++, args)) return false;
      continue;
    }

    auto parse_int = [](best::str& s) -> best::option<uint32_t> {
      // Parse digits for an explicit index.
      auto idx = s.find([](rune r) { return !r.is_ascii_digit(); });
      // We should find a non-digit rune, because otherwise this is an invalid
      // string.
      if (!idx) best::none;

      auto parsed = best::atoi<uint32_t>(s[{.end = *idx}]);
      if (!parsed) return best::none;
      s = s[{.start = *idx}];
      return *parsed;
    };

    size_t cur_idx;
    if (templ.consume_prefix(':')) {
      cur_idx = idx++;
    } else {
      // Parse digits for an explicit index.
      auto parsed = parse_int(templ);
      if (!parsed) return false;
      cur_idx = *parsed;

      // The next rune must be either ':' or '}'. If it's ':' we consume it.
      if (!templ.consume_prefix(':')) {
        if (!templ.consume_prefix('}')) return false;
        if (!best::call(interpolate, cur_idx, args)) return false;
        continue;
      }
    }
    if (templ.is_empty()) return false;

    best::option<typename spec::align> align;
    auto parse_align = [&](rune r) {
      switch (r) {
        case '<':
          align = spec::Left;
          return true;
        case '^':
          align = spec::Center;
          return true;
        case '>':
          align = spec::Right;
          return true;
        default:
          return false;
      }
    };

    // Next we need to parse the fill. Check if the rune immediately after the
    // first is is one of <, ^, >. If not, check the first rune. This is order
    // is required to correctly handle specs like `{:>>}`.
    auto [next, rest] = *templ.break_off();
    if (rest.consume_prefix(parse_align)) {
      args.fill = next;
      args.alignment = *align;
      templ = rest;
    } else if (templ.consume_prefix(parse_align)) {
      args.alignment = *align;
      templ = rest;
    }

    // Parse for '#' and '0'.
    if (templ.consume_prefix('#')) args.alt = true;
    if (templ.consume_prefix('0')) args.sign_aware_padding = true;

    // Now, try parsing a width.
    if (templ.starts_with(&rune::is_ascii_digit)) {
      auto parsed = parse_int(templ);
      // Width must be positive.
      if (parsed < 1u) return false;
      args.width = *parsed;
    } else {
      // Using the alignment or '0' flags requires specifying a width.
      if (align || args.sign_aware_padding) return false;
    }

    // And then a precision.
    if (templ.consume_prefix('.')) {
      auto parsed = parse_int(templ);
      if (!parsed) return false;
      args.prec = *parsed;
    }

    // Finally, we can parse the method. This should be any alphabetic ASCII
    // rune.
    if (templ.is_empty()) return false;
    auto [method, rest2] = *templ.break_off();
    if (method.is_ascii_alpha()) {
      args.method = method;
      templ = rest2;
    }

    // And optionally the debug flag.
    if (templ.consume_prefix('?')) args.debug = true;

    // We should hit a '}' here unconditionally. Then, interpolate.
    if (!templ.consume_prefix('}')) return false;
    if (!best::call(interpolate, cur_idx, args)) return false;
  }

  return true;
}

template <typename spec, typename... Args>
class templ final {
 public:
  static constexpr std::array<typename spec::query, sizeof...(Args)> Queries{
      spec::query::template of<Args>...};

  static constexpr bool validate(best::span<const char> templ) {
    auto str = unsafe::in([&](auto u) {
      return best::str(u, best::span(templ.data(), templ.size() - 1));
    });
    return format_internal::visit_template<spec>(
        str, [](auto) { return true; },
        [&](size_t n, const spec& s) {
          if (n > Queries.size()) return false;
          auto& q = Queries[n];

          if (q.requires_debug && !s.debug) return false;
          if (!q.supports_width && s.width > 0) return false;
          if (!q.supports_prec && s.prec) return false;
          if (s.method && !q.methods.contains(*s.method)) return false;
          return true;
        });
  }

 public:
  template <size_t n>
  constexpr templ(const char (&chars)[n]) BEST_IS_VALID_LITERAL(chars, utf8{})
      BEST_ENABLE_IF(validate(chars),
                     "invalid format string (better diagnostics NYI)") {
    unsafe::in(
        [&](auto u) { template_ = best::str(u, best::span(chars, n - 1)); });
  }

 private:
  friend best::formatter;
  best::str template_;
};

// Can't seem to use requires {} for this. Unclear if this is a Clang bug, or
// a limitation of how names are bound in requires-expressions.
template <typename T>
struct has_fmt {
  static auto check(const auto& r)
      -> decltype(BestFmt(std::declval<best::ftadle&>(), r), true);
  static void check(...);

  static constexpr bool value =
      !best::void_type<decltype(check(std::declval<const T&>()))>;
};
template <typename T>
struct has_fmt_query {
  static auto check(auto* r)
      -> decltype(BestFmtQuery(std::declval<best::ftadle&>(), r), true);
  static void check(...);

  static constexpr bool value =
      !best::void_type<decltype(check(std::declval<T*>()))>;
};
}  // namespace best::format_internal
#endif  // BEST_TEXT_INTERNAL_FORMAT_PARSER_H_