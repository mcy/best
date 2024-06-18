#ifndef BEST_TEXT_FORMAT_H_
#define BEST_TEXT_FORMAT_H_

#include <cstddef>
#include <cstdio>

#include "best/container/option.h"
#include "best/meta/concepts.h"
#include "best/text/internal/format_parser.h"
#include "best/text/rune.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"

//! String formatting.
//!
//! The `best::formatter` is a `fmtlib`-style formatter for structured printing,
//! with a Rust `fmt`-like API.
//!
//! Values can make themselves formattable by implementing the `BestFmt()`
//! FTADLE, which should look like this:
//!
//! ```
//! friend void BestFmt(auto& fmt, const MyType& self);
//! ```
//!
//! This function is passed a reference to a `best::formatter` and to the
//! value being printed.

namespace best {
/// # `best::formattable`
///
/// Whether a type can be formatted.
template <typename T>
concept formattable =
    best::void_type<T> ||
    requires(best::formatter& fmt, const T& value) { BestFmt(fmt, value); };

/// # `best::format_spec`
///
/// Arguments to a particular formatting operation. This controls precisely
/// how a value should be formatted.
///
/// This corresponds to the information in a `{...}` specifier in a format
/// template.
struct format_spec final {
  /// # `format_spec::query`
  ///
  /// An alignment for a formatting operation that specifies a `width`.
  enum align { Left, Center, Right };

  /// Common formatting methods.
  static constexpr rune Hex = 'x';
  static constexpr rune UpperHex = 'X';
  static constexpr rune Octal = 'o';
  static constexpr rune Binary = 'b';
  static constexpr rune Pointer = 'p';

  /// Whether the `#` flag was passed. This usually
  bool alt = false;
  /// Whether the `?` flag was passed.
  bool debug = false;
  /// Whether the `0` flag was passed.
  bool sign_aware_padding = false;

  /// How to align the result within the padding.
  align alignment = Right;

  /// What to fill the padding with.
  best::rune fill = ' ';

  /// The minimum width this formatting operation should try to produce.
  uint32_t width = 0;

  /// The maximum precision this formatting operation should use.
  best::option<uint32_t> prec;

  /// The method to format with, represented as a character.
  best::option<best::rune> method;

  /// # `format_spec::query`
  ///
  /// By default, the only format specifier a type supports is `{:?}`. To enable
  /// other specifiers, it must define the `BestFmtQuery()` FTADLE, which is
  /// passed a value of this type:
  ///
  /// ```
  /// constexpr friend void BestFmtQuery(auto& query, MyType*);
  /// ```
  struct query final {
    /// Set this to `false` so that specifiers not ending in `?` can be used
    /// with your type.
    bool requires_debug = true;
    /// Set this to `true` so that width-related syntax can be used.
    bool supports_width = false;
    /// Set this to `true` so that precision-related syntax can be used.
    bool supports_prec = false;
    /// Set this to a non-null function to enable custom methods for this type.
    /// For example, integers set this to
    ///
    /// ```
    /// [](rune r) { return best::str("boxX").contains(r); }
    /// ```
    bool (*uses_method)(best::rune) = nullptr;

    /// # `query::of<T>`
    ///
    /// Computes the query of a particular type.
    template <typename T>
    static constexpr auto of = []<typename q = query> {
      q query;
      if constexpr (format_internal::has_fmt_query<T>::value) {
        BestFmtQuery(query, best::as_ptr<T>());
      }
      return query;
    }
    ();
  };

  constexpr bool operator==(const format_spec&) const = default;

  friend auto& operator<<(auto& os, const format_spec& spec) {
    return os << "{" << spec.alt << ", " << spec.debug << ", "
              << spec.sign_aware_padding << ", " << spec.alignment << ", "
              << spec.fill << ", " << spec.width << ", " << spec.prec << ", "
              << spec.method << "}";
  }
};

/// # `best::format_template`
///
/// A format template. This is constructable from a string literal. The library
/// will check that that string is actually a valid formatting template for
/// a given list of argument types.
template <typename... Args>
using format_template =
    best::format_internal::templ<format_spec,
                                 best::dependent<Args, Args...>...>;

class formatter final {
 public:
  class block;
  friend block;

  /// # `formatter::config`
  ///
  /// Options for initializing a formatting operation.
  struct config final {
    /// The string used for indenting blocks.
    best::str indent = "  ";
  };

  formatter(const formatter&) = delete;
  formatter& operator=(const formatter&) = delete;
  formatter(formatter&&) = delete;
  formatter& operator=(formatter&&) = delete;

  /// # `formatter::write()`
  ///
  /// Writes character data directly to the output. Non-UTF-8 strings are
  /// transcoded as-needed.
  void write(rune r);
  void write(const best::string_type auto& string);

  /// # `formatter::format()`
  ///
  /// Writes a formatted value directly, potentially overriding the current
  /// specifier configuration.
  void format(const best::formattable auto& arg) { BestFmt(*this, arg); }
  void format(const best::format_spec& spec,
              const best::formattable auto& arg) {
    auto old = std::exchange(cur_spec_, spec);
    format(arg);
    cur_spec_ = old;
  }

  /// # `formatter::format(template, ...)`
  ///
  /// Expands a formatting template and appends the result to this formatter.
  template <best::formattable... Args>
  void format(best::format_template<Args...>, const Args&...);

  /// # `formatter::current_spec()`
  ///
  /// Returns the specification for the value currently being printed.
  const best::format_spec& current_spec() const { return *cur_spec_; }

  /// # `formatter::current_spec()`
  ///
  /// Returns the `config` value for the current operation.
  const best::formatter::config& current_config() const { return config_; }

  /// # `formatter::list()`, `formatter::tuple()`, `formatter::record()`
  ///
  /// Returns a formatter block for printing structured data. Each of these
  /// respectively wraps the block in either `[]`, `()`, or `{}`. You may
  /// specify an optional title for the block (such as a type name).
  block list(best::str title = "");
  block tuple(best::str title = "");
  block record(best::str title = "");

 private:
  explicit formatter(best::strbuf* out) : out_(out) {}

  template <best::formattable... Args>
  friend void format(best::strbuf&, best::format_template<Args...>,
                     const Args&...);

  void update_indent() {
    if (!std::exchange(at_new_line_, false)) return;
    for (size_t i = 0; i < indent_; ++i) {
      out_->push_lossy(config_.indent);
    }
  }

  best::strbuf* out_;
  best::option<const best::format_spec&> cur_spec_;
  bool at_new_line_ = false;
  size_t indent_ = 0;
  size_t bytes_written = 0;
  config config_;
};

/// # `formatter::block`
///
/// A helper for printing structured data, such as lists, tuples, maps, and
/// structs.
class formatter::block final {
 public:
  /// # `block::entry()`
  ///
  /// Submits a single unkeyed entry to this block.
  void entry(const best::formattable auto& value) {
    if (!fmt_) return;
    separator();
    fmt_->format(value);
  }
  void entry(const best::format_spec& spec,
             const best::formattable auto& value) {
    if (!fmt_) return;
    separator();
    fmt_->format(spec, value);
  }

  /// # `block::field()`
  ///
  /// Submits a single keyed entry to this block.
  void field(const best::formattable auto& key,
             const best::formattable auto& value) {
    if (!fmt_) return;
    separator();
    fmt_->format(key);
    fmt_->write(": ");
    fmt_->format(value);
  }
  void field(const best::format_spec& k_spec, const best::formattable auto& key,
             const best::format_spec& v_spec,
             const best::formattable auto& value) {
    if (!fmt_) return;
    separator();
    fmt_->format(k_spec, key);
    fmt_->write(": ");
    fmt_->format(v_spec, value);
  }

  /// # `block::finish()`
  ///
  /// Finishes this block; all further operations on this value are no-ops.
  /// This function is called automatically by the destructor.
  void finish() {
    if (!fmt_) return;
    if (uses_indent_ && entries_ > 0) {
      fmt_->write("\n");
    }
    if (uses_indent_) --fmt_->indent_;
    fmt_->write(config_.close);
  }

  ~block() { finish(); }
  block(const block&) = delete;
  block& operator=(const block&) = delete;
  block(block&&) = delete;
  block& operator=(block&&) = delete;

 private:
  struct config {
    best::str title;
    best::str open, close;
  };

  friend formatter;
  block(const config& config, formatter* fmt) : config_(config), fmt_(fmt) {
    if (!config_.title.is_empty()) {
      fmt_->write(config_.title);
      fmt_->write(' ');
    }
    fmt_->write(config.open);

    uses_indent_ = fmt_->current_spec().alt;
    if (uses_indent_) ++fmt_->indent_;
  }

  void separator() {
    if (uses_indent_) {
      entries_ == 0 ? fmt_->write("\n") : fmt_->write(",\n");
    } else if (entries_ != 0) {
      fmt_->write(", ");
    }
  }

  size_t entries_ = 0;
  bool uses_indent_ = false;
  config config_;
  formatter* fmt_;
};

/// # `best::format()`
///
/// Executes a formatting operation and returns the result as a string, or
/// append it to an existing string.
template <best::formattable... Args>
best::strbuf format(best::format_template<Args...> templ, const Args&... args);
template <best::formattable... Args>
void format(best::strbuf& out, best::format_template<Args...> templ,
            const Args&... args);

/// # `best::print()`, `best::println()`, `best::eprint()`, `best::eprintln()`
///
/// Executes a formatting operation and writes the result to stdout or stderr.
/// The `ln` functions will also print a newline.
template <best::formattable... Args>
void print(best::format_template<Args...> templ, const Args&... args);
template <best::formattable... Args>
void println(best::format_template<Args...> templ, const Args&... args);
template <best::formattable... Args>
void eprint(best::format_template<Args...> templ, const Args&... args);
template <best::formattable... Args>
void eprintln(best::format_template<Args...> templ, const Args&... args);

/// # `best::make_formattable()`
///
/// Makes any type formattable. If a formatting implementation is not found for
/// the type, it is printed as a hex string instead.
template <typename T>
struct make_formattable {
  const T& value;

  friend void BestFmt(auto& fmt, const make_formattable& x) {
    if constexpr (best::formattable<T>) {
      fmt.format(x.value);
      return;
    }

    const char* bytes = reinterpret_cast<const char*>(std::addressof(x.value));
    fmt.format("unprintable {}-byte value: `", sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i) {
      fmt.format("{:02x}", bytes[i]);
    }
    fmt.write("`");
  }
  constexpr friend void BestFmtQuery(auto& query, make_formattable*) {
    query = query.template of<T>;
  }
};
template <typename T>
make_formattable(const T&) -> make_formattable<T>;

// --- IMPLEMENTATION DETAILS BELOW ---

inline void formatter::write(rune r) {
  size_t prev = out_->size();
  if (r != '\n') update_indent();

  out_->push_lossy(r);
  bytes_written += out_->size() - prev;
}

void formatter::write(const best::string_type auto& string) {
  size_t prev = out_->size();
  if (indent_ == 0) {
    out_->push_lossy(string);
    bytes_written += out_->size() - prev;
  }

  rune::iter runes(string);
  const auto& enc = best::encoding_of(string);
  auto curr = runes.rest();
  for (rune r : runes) {
    if (r == '\n') {
      if (runes.rest().size() < curr.size()) {
        update_indent();
        out_->push_lossy(curr[{.end = runes.rest().size() - curr.size()}], enc);
        curr = runes.rest();
      }

      out_->push_lossy('\n');
      at_new_line_ = true;
      continue;
    }
  }
  if (!curr.is_empty()) {
    update_indent();
    out_->push_lossy(curr, enc);
  }
}

inline formatter::block formatter::list(best::str title) {
  return {{.title = title, .open = "[", .close = "]"}, this};
}
inline formatter::block formatter::tuple(best::str title) {
  return {{.title = title, .open = "(", .close = ")"}, this};
}
inline formatter::block formatter::record(best::str title) {
  return {{.title = title, .open = "{", .close = "}"}, this};
}

template <best::formattable... Args>
void formatter::format(best::format_template<Args...> fmt, const Args&... arg) {
  std::array<best::row<const void*,
                       void (*)(formatter&, const format_spec&, const void*)>,
             sizeof...(Args)>
      vtable = {{{
          std::addressof(arg),
          [](formatter& f, const format_spec& s, const void* v) {
            f.format(s, *reinterpret_cast<const Args*>(v));
          },
      }...}};

  format_internal::visit_template(
      fmt.template_,
      [&](best::str chunk) {
        out_->push(chunk);
        return true;
      },
      [&](size_t idx, const auto& spec) {
        vtable[idx][best::index<1>](*this, spec, vtable[idx][best::index<0>]);
        return true;
      });
}

template <best::formattable... Args>
void format(best::strbuf& out, best::format_template<Args...> templ,
            const Args&... args) {
  best::formatter fmt(&out);
  fmt.format(templ, args...);
}
template <best::formattable... Args>
best::strbuf format(best::format_template<Args...> templ, const Args&... args) {
  best::strbuf out;
  best::format(out, templ, args...);
  return out;
}

template <best::formattable... Args>
void print(best::format_template<Args...> templ, const Args&... args) {
  auto result = best::format(templ, args...);
  ::fwrite(result.data(), 1, result.size(), stdout);
}

template <best::formattable... Args>
void println(best::format_template<Args...> templ, const Args&... args) {
  auto result = best::format(templ, args...);
  result.push('\n');
  ::fwrite(result.data(), 1, result.size(), stdout);
}

template <best::formattable... Args>
void eprint(best::format_template<Args...> templ, const Args&... args) {
  auto result = best::format(templ, args...);
  ::fwrite(result.data(), 1, result.size(), stderr);
}

template <best::formattable... Args>
void eprintln(best::format_template<Args...> templ, const Args&... args) {
  auto result = best::format(templ, args...);
  result.push('\n');
  ::fwrite(result.data(), 1, result.size(), stderr);
}
}  // namespace best

#include "best/text/internal/format_impls.h"

// Silence a tedious clang-tidy warning.
namespace best::format_internal {
using mark_as_used2 = mark_as_used;
}  // namespace best::format_internal

#endif  // BEST_TEXT_FORMAT_H_