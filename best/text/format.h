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

#ifndef BEST_TEXT_FORMAT_H_
#define BEST_TEXT_FORMAT_H_

#include <cstddef>
#include <cstdio>

#include "best/container/option.h"
#include "best/text/internal/format_parser.h"
#include "best/text/rune.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"

//! String formatting.
//!
//! The `best::formatter` is a `fmtlib`-style formatter for structured printing,
//! strongly inspired by Rust's `fmt`-like API.
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
//!
//! ## Format Template Syntax
//!
//! A `best::format_template<...>` is a verified formatting template that can
//! be passed to `best::format` etc. It is constructible from a `constexpr`
//! string literal that includes format specifiers (wrapped in `{}`) that
//! represent values to interpolate in from the other arguments to
//! `best::format`.
//!
//! The syntax is very similar to Rust's but with some differences. The full
//! grammar is this:
//!
//! ```
//! '{'[num][':'['!'][[fill]align]['#']['0'][width]['.' prec][method]['?']]'}'
//! ```
//!
//! * `num`: Which value from the argument list to use. If omitted, this drawn
//!   from a counter incremented for each `{}` specifier without a number.
//!
//! * `!`: Forward from above. This will use whatever formatting style was
//!   requested by the caller of `BestFmt()`. This is useful for container
//!   types. Defaults to `?` with no method.
//!
//! * `align`: One of `<`, `^`, `>`. Sets the fill and alignment to justify with
//!   when the output would be smaller than `width`.
//!
//! * `#`: Sets the "alternative mode" for printing. `BestFmt` implementations
//!   can interpret this however they wish. For integers, it adds a radix
//!   prefix, for `formatter::block`s it makes them multiline, etc.
//!
//! * `0`: Sets "sign aware zero padding". Incompatible with `align`. This is
//!   the behavior of e.g. `%08x` in `printf()`.
//!
//! * `width`: Sets the minimum with that should be printed.
//!
//! * `prec`: Sets the prevision to print with. This affects the number of
//!   digits to print for floating point numbers, and the maximum length to
//!   print from a string.
//!
//! * `method`: A single ASCII letter. This is an open-ended extension point for
//!   types to provide custom formatting directives. Integers use this for
//!   setting what base they print, e.g. `{:x}`, `{:X}`, `{:b}`, and `{:o}`.
//!   Strings can be quoted with `{:q}`, and pointer-like types can have their
//!   address printed with `{:p}`. It ius
//!
//! * `?`: Whether this is "debug printing". Debug printing has fewer guarantees
//!   on output. By default, all `BestFmt()` implementations only allow debug
//!   printing.
//!
//! To configure which of these options your type supports, implement the
//! `BestFmtQuery()` FTADLE, as directed in `best::format_spec::query`.
//!
//! ## Using the formatter.
//!
//! The main entry point is `best::format()`. You call it with a format template
//! string and some arguments to interpolate. The syntax of the template is
//! checked at compile time, as is whether the supplied arguments are compatible
//! with it.
//!
//! `best::println()` and friends can be used to write directly to
//! stderr/stdout.

namespace best {
/// # `best::formattable`
///
/// Whether a type can be formatted.
template <typename T>
concept formattable =
  best::is_void<T> ||
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

  /// Whether the `#` flag was passed. This usually
  bool alt = false;
  /// Whether the `?` flag was passed.
  bool debug = false;
  /// Whether the `0` flag was passed.
  bool sign_aware_padding = false;
  /// If set, don't override any currently set specs.
  bool pass_through = false;

  /// How to align the result within the padding.
  best::option<align> alignment;

  /// What to fill the padding with.
  best::rune fill = ' ';

  /// The minimum width this formatting operation should try to produce.
  uint32_t width = 0;

  /// The maximum precision this formatting operation should use.
  best::option<uint32_t> prec;

  /// The method to format with, represented as a single rune.
  best::option<best::rune> method;

  /// # `format_spec::compute_padding()`
  ///
  /// Computes the padding necessary if we were to respect the padding
  /// requirements, given a particular number of runes to write.
  best::row<size_t, size_t> compute_padding(size_t runes_to_write,
                                            align default_alignment) const;

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
      if constexpr (requires { BestFmtQuery(query, best::as_ptr<T>()); }) {
        BestFmtQuery(query, best::as_ptr<T>());
      }
      return query;
    }
    ();
  };

  constexpr bool operator==(const format_spec&) const = default;

 private:
  friend formatter;
  static const format_spec Default;
};
inline constexpr format_spec format_spec::Default{.debug = true};

/// # `best::format_template`
///
/// A format template. This is constructable from a string literal. The library
/// will check that that string is actually a valid formatting template for
/// a given list of argument types.
///
/// This type has two functions: `.as_str()`, which returns the actual string
/// within, and `.where()`, which returns the location at which this template
/// was constructed.
template <typename... Args>
using format_template = best::format_internal::templ<  //
  format_spec, best::dependent<Args, Args...>...>;

/// # `best::formatter`
///
/// The type passed into `BestFmt()`. Not directly user-constructable; instead,
/// it is constructed by functions like `best::format()` and passed to
/// user-defined FTADLEs.
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
  void format(const best::format_spec& spec, const best::formattable auto& arg);

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

  /// Prints indentation if we are at the start of a new line.
  void update_indent();

  struct vptr;
  void format_impl(best::str templ, vptr* vtable);

  best::strbuf* out_;
  best::option<const best::format_spec&> cur_spec_ = format_spec::Default;
  bool at_new_line_ = false;
  size_t indent_ = 0;
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
  block& entry(const best::formattable auto& value);
  block& entry(const best::format_spec& spec,
               const best::formattable auto& value);

  /// # `block::field()`
  ///
  /// Submits a single keyed entry to this block. This is a wrapper over
  /// `pair()` that always uses a default formatting spec (a `{}`) for the key.
  block& field(best::str name, const best::formattable auto& value);
  block& field(best::str name, const best::format_spec& v_spec,
               const best::formattable auto& value);

  /// # `block::pair()`
  ///
  /// Submits a single keyed entry to this block.
  block& pair(const best::formattable auto& key,
              const best::formattable auto& value);
  block& pair(const best::format_spec& k_spec,
              const best::formattable auto& key,
              const best::format_spec& v_spec,
              const best::formattable auto& value);

  /// # `block::finish()`
  ///
  /// Finishes this block; all further operations on this value are no-ops.
  /// This function is called automatically by the destructor.
  void finish();

  /// # `block::advise_size()`
  ///
  /// Advices this block that exactly `n` elements will be printed. This is
  /// used for some print output cleanup.
  ///
  /// Calling this after calling `entry()` or `field()` is unspecified.
  void advise_size(size_t n);

  ~block() { finish(); }
  block(const block&) = delete;
  block& operator=(const block&) = delete;
  block(block&&) = delete;
  block& operator=(block&&) = delete;

 private:
  friend formatter;
  struct config {
    best::str title;
    best::str open, close;
  };

  block(const config& config, formatter* fmt);

  /// Prints a comma if needed.
  void separator();

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
[[nodiscard(
  "best::format() returns a brand new string if not given a best::strbuf& to "
  "write to")]] best::strbuf
format(best::format_template<Args...> templ = "", const Args&... args);
template <best::formattable... Args>
void format(best::strbuf& out, best::format_template<Args...> templ,
            const Args&... args);

/// # `best::print()`, `best::println()`, `best::eprint()`, `best::eprintln()`
///
/// Executes a formatting operation and writes the result to stdout or stderr.
/// The `ln` functions will also print a newline.
template <best::formattable... Args>
void print(best::format_template<Args...> templ = "", const Args&... args);
template <best::formattable... Args>
void println(best::format_template<Args...> templ = "", const Args&... args);
template <best::formattable... Args>
void eprint(best::format_template<Args...> templ = "", const Args&... args);
template <best::formattable... Args>
void eprintln(best::format_template<Args...> templ = "", const Args&... args);

/// # `best::make_formattable()`
///
/// Makes any type formattable. If a formatting implementation is not found for
/// the type, it is printed as a hex string instead.
decltype(auto) make_formattable(const auto& value);
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

#include "best/text/internal/format_impls.h"

// Silence a tedious clang-tidy warning.
namespace best::format_internal {
using mark_as_used2 = mark_as_used;
}  // namespace best::format_internal

namespace best {
void formatter::format(const best::format_spec& spec,
                       const best::formattable auto& arg) {
  if (spec.pass_through) {
    format(arg);
    return;
  }
  auto old = std::exchange(cur_spec_, spec);
  format(arg);
  cur_spec_ = old;
}

void formatter::write(const best::string_type auto& string) {
  if constexpr (best::is_pretext<decltype(string)>) {
    if (indent_ == 0) {
      out_->push_lossy(string);
      return;
    }

    size_t watermark = 0;
    for (auto [idx, r] : string.rune_indices()) {
      if (r != '\n') { continue; }
      if (idx != watermark + 1 && idx > 0) {
        update_indent();
        out_->push_lossy(string[{.start = watermark, .end = idx - 1}]);
      }

      watermark = idx;
      out_->push_lossy("\n");
      at_new_line_ = true;
    }

    if (watermark < string.size()) {
      update_indent();
      out_->push_lossy(string[{.start = watermark}]);
    }
  } else {
    write(best::pretext(string));
  }
}

// These are *very* common instantiations that we can cheapen by making them
// extern templates. The corresponding explicit instantiation lives in
// format.cc.
extern template void formatter::write(const best::pretext<utf8>&);
extern template void formatter::write(const best::text<utf8>&);
extern template void formatter::write(const best::pretext<wtf8>&);
extern template void formatter::write(const best::text<wtf8>&);
extern template void formatter::write(const best::pretext<utf16>&);
extern template void formatter::write(const best::text<utf16>&);

decltype(auto) make_formattable(const auto& value) {
  if constexpr (best::formattable<decltype(value)>) {
    return value;
  } else {
    return format_internal::unprintable{
      reinterpret_cast<const char*>(best::addr(value)), sizeof(value)};
  }
}

struct formatter::vptr final {
  const void* data;
  void (*fn)(formatter&, const format_spec&, const void*);

  template <typename Arg>
  static void erased(formatter& f, const format_spec& s, const void* v) {
    f.format(s, *reinterpret_cast<const Arg*>(v));
  }
};

template <best::formattable... Args>
void formatter::format(best::format_template<Args...> fmt, const Args&... arg) {
  vptr vtable[] = {{
    best::addr(arg),
    vptr::erased<Args>,
  }...};
  format_impl(fmt.as_str(), vtable);
}

formatter::block& formatter::block::entry(const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(value);
  return *this;
}
formatter::block& formatter::block::entry(const best::format_spec& spec,
                                          const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(spec, value);
  return *this;
}

formatter::block& formatter::block::field(best::str name,
                                          const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(format_spec{}, name);
  fmt_->write(": ");
  fmt_->format(value);
  return *this;
}
formatter::block& formatter::block::field(best::str name,
                                          const best::format_spec& v_spec,
                                          const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(format_spec{}, name);
  fmt_->write(": ");
  fmt_->format(v_spec, value);
  return *this;
}

formatter::block& formatter::block::pair(const best::formattable auto& key,
                                         const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(key);
  fmt_->write(": ");
  fmt_->format(value);
  return *this;
}
formatter::block& formatter::block::pair(const best::format_spec& k_spec,
                                         const best::formattable auto& key,
                                         const best::format_spec& v_spec,
                                         const best::formattable auto& value) {
  if (!fmt_) { return *this; }
  separator();
  fmt_->format(k_spec, key);
  fmt_->write(": ");
  fmt_->format(v_spec, value);
  return *this;
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

namespace result_internal {
// See the matching decl in result.h.
struct fmt final {
  template <typename T, typename E>
  BEST_INLINE_SYNTHETIC static void check_ok(const best::result<T, E>* result) {
    if (auto err = result->err()) {
      auto message = best::format("unwrapped a best::err({:?})",
                                  best::make_formattable(*err));
      best::crash_internal::crash("%.*s", message.size(), message.data());
    }
  }
};
}  // namespace result_internal
}  // namespace best

#endif  // BEST_TEXT_FORMAT_H_
