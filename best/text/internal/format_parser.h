#ifndef BEST_TEXT_INTERNAL_FORMAT_PARSER_H_
#define BEST_TEXT_INTERNAL_FORMAT_PARSER_H_

#include "best/base/fwd.h"
#include "best/meta/ops.h"
#include "best/text/str.h"

namespace best::format_internal {
constexpr size_t find(const char* data, size_t len, auto cb) {
  for (size_t i = 0; i < len; ++i) {
    if (cb(data[i])) return i;
  }
  return -1;
}

constexpr bool consume_prefix(const char*& data, size_t& len, char needle) {
  if (len != 0 && data[0] == needle) {
    ++data;
    --len;
    return true;
  }
  return false;
}

constexpr bool consume_prefix(const char*& data, size_t& len, auto cb) {
  if (len != 0 && cb(data[0])) {
    ++data;
    --len;
    return true;
  }
  return false;
}

/// # `format_internal::visit_template()`
///
/// Parses a formatting template, calling the given callbacks to print a literal
/// chunk or to interpolate a variable, respectively.
///
/// Each callback can return a bool to signal error; the parser returns a
/// `false` if parsing, printing, or interpolation failed.
template <typename spec = best::format_spec, typename Print,
          typename Interpolate>
constexpr bool visit_template(const char* data, size_t len, Print print,
                              Interpolate interpolate) {
  constexpr bool HavePrint = !best::same<Print, std::nullptr_t>;

  // As nice as best::str is, it's extremely slow in the constexpr evaluator.
  // As such, we need to avoid expensive operations like "bounds checks" and
  // "rune decoding".
  //
  // We don't need a lot of operations available, thankfully.

  size_t idx = 0;
  while (len > 0) {
    // Skip to the next '{'.
    auto brace = find(data, len, [](char r) { return r == '{' || r == '}'; });

    // Print everything up to the brace.
    if constexpr (HavePrint) {
      if (brace != 0) {
        auto bytes = best::span(data, brace == -1 ? len : brace);
        // The caller is assumed to have given us an actually UTF-8 string, not
        // just a random latin1 string.
        auto to_print = best::str(unsafe("templ<> checks this for us"), bytes);
        if (!best::call(print, to_print)) return false;
      }
    }
    if (brace == -1) return true;

    auto what = data[brace];
    data += brace + 1;
    len -= brace + 1;

    if (what == '}') {
      // If this is immediately followed by another }, it is a literal.
      if (consume_prefix(data, len, '}')) {
        if constexpr (HavePrint) {
          if (!best::call(print, best::str("}"))) return false;
        }
        continue;
      }
      // Any other result is an error.
      return false;
    }

    // If this is immediately followed by another {, it is a literal.
    if (consume_prefix(data, len, '{')) {
      if constexpr (HavePrint) {
        if (!best::call(print, best::str("{"))) return false;
      }
      continue;
    }

    // We are parsing this grammar:
    // '{'[idx][:[[fill]align]['#']['0'][width]['.' precision][method]['?']]'}'
    // align := '<' | '^' | '>'
    spec args{};

    // This is a fast-path for `{}`.
    if (consume_prefix(data, len, '}')) {
      if (!best::call(interpolate, idx++, args)) return false;
      continue;
    }

    // This is a fast-path for `{:?}`
    if (len >= 3 && data[0] == ':' && data[1] == '?' && data[2] == '}') {
      data += 3;
      len -= 3;
      args.debug = true;
      if (!best::call(interpolate, idx++, args)) return false;
      continue;
    }

    // This is a fast-path for `{:!}`
    if (len >= 3 && data[0] == ':' && data[1] == '!' && data[2] == '}') {
      data += 3;
      len -= 3;
      args.pass_through = true;
      args.debug = true;
      if (!best::call(interpolate, idx++, args)) return false;
      continue;
    }

    auto atoi = [&]() -> uint32_t {
      // Parse digits for an explicit index.
      auto count = find(data, len, [](char c) { return c < '0' || c > '9'; });
      // We should find a non-digit rune, because otherwise this is an invalid
      // string.
      if (count == -1) -1;

      uint32_t result = 0;
      for (size_t i = 0; i < count; ++i) {
        result *= 10;
        result += data[i] - '0';
      }
      data += count;
      len -= count;
      return result;
    };

    size_t cur_idx;
    if (consume_prefix(data, len, ':')) {
      cur_idx = idx++;
    } else {
      // Parse digits for an explicit index.
      cur_idx = atoi();
      if (cur_idx == -1) return false;

      // The next rune must be either ':' or '}'. If it's ':' we consume it.
      if (!consume_prefix(data, len, ':')) {
        if (!consume_prefix(data, len, '}')) return false;
        if (!best::call(interpolate, cur_idx, args)) return false;
        continue;
      }
    }
    if (len == 0) return false;

    if (consume_prefix(data, len, '!')) {
      args.pass_through = true;
      args.debug = true;
      if (!best::call(interpolate, idx++, args)) return false;
      continue;
    }

    bool have_align = false;
    auto parse_align = [&](char r) {
      switch (r) {
        case '<':
          args.alignment = spec::Left;
          have_align = true;
          return true;
        case '^':
          args.alignment = spec::Center;
          have_align = true;
          return true;
        case '>':
          args.alignment = spec::Right;
          have_align = true;
          return true;
        default:
          return false;
      }
    };

    // Next we need to parse the fill. Check if the rune immediately after the
    // first is is one of <, ^, >. If not, check the first rune. This is order
    // is required to correctly handle specs like `{:>>}`

    ++data, --len;  // Skip a byte.
    if (consume_prefix(data, len, parse_align)) {
      args.fill = *rune::from_int(data[-2]);
    } else {
      --data, ++len;
      // Undo the skip from the previous operation.
      consume_prefix(data, len, parse_align);
    }

    // Parse for '#' and '0'.
    if (consume_prefix(data, len, '#')) args.alt = true;
    if (consume_prefix(data, len, '0')) {
      // Cannot specify '0' with an explicit fill+alignment.
      if (have_align) return false;
      args.sign_aware_padding = true;
    }
    if (len == 0) return false;

    // Now, try parsing a width.
    if (data[0] >= '0' && data[0] <= '9') {
      args.width = atoi();
      // Width must be positive.
      if (args.width == 0 || args.width == -1) return false;
    } else {
      // Using the alignment or '0' flags requires specifying a width.
      if (have_align || args.sign_aware_padding) return false;
    }

    // And then a precision.
    if (consume_prefix(data, len, '.')) {
      args.prec = atoi();
      if (args.prec == -1) return false;
    }
    if (len == 0) return false;

    // Finally, we can parse the method. This should be any alphabetic ASCII
    // rune.
    if ((data[0] >= 'a' && data[0] <= 'z') ||
        (data[0] >= 'A' && data[0] <= 'Z')) {
      args.method = *rune::from_int(data[0]);
      ++data, --len;  // Skip a byte.
    }

    // And optionally the debug flag.
    if (consume_prefix(data, len, '?')) args.debug = true;

    // We should hit a '}' here unconditionally. Then, interpolate.
    if (!consume_prefix(data, len, '}')) return false;
    if (!best::call(interpolate, cur_idx, args)) return false;
  }

  return true;
}

template <typename spec, typename... Args>
class templ final {
 private:
  static constexpr std::array<typename spec::query, sizeof...(Args)> Queries{
      spec::query::template of<Args>...};

  static constexpr bool validate(best::span<const char> templ) {
    return format_internal::visit_template<spec>(
        templ.data(), templ.size() - 1, nullptr, [&](size_t n, const spec& s) {
          if (n > Queries.size()) return false;
          auto& q = Queries[n];

          if (q.requires_debug && !s.debug) return false;
          if (!q.supports_width && s.width > 0) return false;
          if (!q.supports_prec && s.prec) return false;
          if (s.method && !q.uses_method(*s.method)) return false;
          return true;
        });
  }

 public:
  template <size_t n>
  constexpr templ(const char (&chars)[n], best::location loc = best::here)
      BEST_IS_VALID_LITERAL(chars, utf8{})
          BEST_ENABLE_IF(validate(chars),
                         "invalid format string (better diagnostics NYI)")
      : template_(unsafe("checked by BEST_IS_VALID_LITERAL()"),
                  best::span(chars, n - 1)),
        loc_(loc) {}

  constexpr best::str as_str() const { return template_; }
  constexpr best::location where() const { return loc_; }

 private:
  best::str template_;
  best::location loc_;
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
      !best::void_type<decltype(check(best::as_ptr<T>()))>;
};

struct unprintable final {
  const char* bytes_;
  size_t size_;
};

void BestFmt(auto& fmt, const unprintable& x) {
  fmt.format("unprintable {}-byte value: `", x.size_);
  for (size_t i = 0; i < x.size_; ++i) {
    fmt.format("{:02x}", uint8_t(x.bytes_[i]));
  }
  fmt.write("`");
}
template <typename T>
constexpr void BestFmtQuery(auto& query, unprintable*) {
  query = query.template of<T>;
}
}  // namespace best::format_internal
#endif  // BEST_TEXT_INTERNAL_FORMAT_PARSER_H_