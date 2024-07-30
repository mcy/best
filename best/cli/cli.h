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

#ifndef BEST_CLI_CLI_H_
#define BEST_CLI_CLI_H_

#include <cstdint>

#include "best/container/result.h"
#include "best/meta/init.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"

//! Command-line flags.
//!
//! This header provides functionality for parsing CLI flags from program
//! inputs. In `best`, CLI flags are defined as a reflectable struct with
//! usage, flag, parse, and validation information attached to its fields as
//! tags.
//!
//! This means that it's easy to construct flag structs independently of
//! actual flag parsing: they're "just" structs! This design pattern is heavily
//! inspired by some Rust CLI crates, such as clap, structopt, and argh.
//!
//! For an example flags struct, see `toy_flags.h` and `toy.cc`. Note that
//! the actual parsing functions live in `cli/parser.h`. This is so that this
//! header is comparatively light-weight to the parser header; this header
//! does not pull in reflection.

namespace best {
/// # `best::cli`
///
/// A flags struct parser. Currently this has no user-accessible functions, but
/// it does contain all of the types necessary for defining a flags struct.
class cli final {
 public:
  /// # `cli::visibility`
  ///
  /// The visibility of some flag or other element of a flags struct.
  enum visibility : uint8_t {
    /// The default. Shown in `--help`.
    Public,
    /// Hidden from `--help` but appears in `--help-hidden`.
    Hidden,
    /// Not shown in any help but active.
    Invisible,
    /// Not parsed and reported as an unknown flag. Useful for disabling flags
    /// with compile-time defines.
    Delete,
  };

  /// # `cli::count`
  ///
  /// The number of times a flag can appear on the command line.
  enum count : uint8_t {
    /// The default: can appear at most once.
    Optional,
    /// Must occur exactly once.
    Required,
    /// May occur any number of times.
    Repeated,
  };

  /// # `cli::app`
  ///
  /// Top-level reflection tag for a CLI struct. Completely optional.
  struct app final {
    /// The name of the program. May be any string.
    ///
    /// If not specified, usage information will use `argv[0]` instead.
    best::str name;

    /// The author(s) of the program.
    best::str authors;

    /// Help text to show when the user runs `--help subcommand`. If not
    /// specified, uses `help`.
    best::str about;

    /// The version of the program.
    best::str version;

    /// A website URL for the program.
    best::str url;

    /// A copyright year to show in help. This is only shown if `authors` is
    /// nonempty.
    best::option<uint32_t> copyright_year;

    /// A license name to show in help. Ideally this should be an SPDX
    /// identifier. This is only shown if `authors` is  nonempty.
    best::str license;
  };

  /// # `cli::alias`
  ///
  /// Adds an alias to a CLI item. A field may contain
  /// many copies of this tag.
  struct alias final {
    /// Same restrictions as the `name` field on the corresponding tag.
    best::str name;
    /// The visibility for this alias. If not specified, uses the visibility
    /// of the tag it is attached to.
    best::option<visibility> vis;
  };

  /// # `cli::flag`
  ///
  /// A reflection tag for specifying a CLI flag. A field may have at most one
  /// copy of this tag.
  struct flag final {
    /// The long name of this flag, e.g. "my-flag". Must not start or end with
    /// `-` or `_`, nor contain ' ', '.', '=', nor ASCII control characters
    ///
    /// If not specified, will default to the name of the corresponding field.
    /// Flag names are insensitive to which of `_` or `-` is used as an internal
    /// separator.
    ///
    /// For example, if set to "foo_bar", the flag may be set by either
    /// `--foo_bar` or `--foo-bar` on the command line.
    ///
    /// The flags `--help`, `--help-hidden`, `--version`, `-h`, and `-V` are
    /// reserved for `best::cli` framework.
    best::str name;

    /// An optional short name consisting of a single character. It provides
    /// additional, convenient behavior:
    ///
    /// - It can be invoked with a single `-` instead of a `--`: `-A` instead of
    ///   `--A`. For completeness, `best::cli` also parses `--A`.
    /// - They can also be passed compactly: `-abcd` is equivalent to passing
    ///   `-a -b -c -d`.
    best::rune letter;

    /// The visibility for this flag.
    visibility vis = Public;

    /// The name of the flag's argument. Optional. Will only appear in help
    /// messages.
    best::str arg;

    /// The count for this flag.
    best::option<count> count;

    /// Help text to show alongside the flag.
    best::str help;
  };

  /// # `cli::positional`
  ///
  /// A reflection tag for specifying a CLI positional argument. A field may
  /// only contain one copy of this tag. This is the implicit tag for
  /// `best::is_from_argv` types.
  struct positional final {
    /// The name of this argument. Optional. Will only appear in help
    /// messages.
    best::str name;

    /// The count for this positional.
    best::option<count> count;

    /// Help text to show alongside the positional.
    best::str help;
  };

  /// # `cli::subcommand`
  ///
  /// A reflection tag for specifying a CLI subcommand. A field may only
  /// contain one copy of this tag.
  ///
  /// A subcommand is a field in a flag struct that is itself another flag
  /// struct. When the name of a subcommand is encountered, the flags parser
  /// switches to parsing this struct and does not return to the parent.
  struct subcommand final {
    /// The name of this subcommand e.g. "my-sub". Must not start or end with
    /// `-` or `_`, nor contain ' ', '.', '=', nor ASCII control characters
    ///
    /// If not specified, will default to the name of the corresponding field.
    /// Subcommand names are insensitive to which of `_` or `-` is used as an
    /// internal separator.
    ///
    /// For example, if set to "foo_bar", the subcommand may be set by either
    /// `foo_bar` or `foo-bar` on the command line.
    best::str name;

    /// The visibility for this subcommand.
    visibility vis = Public;

    /// Help text to show alongside the subcommand.
    best::str help;

    /// Help text to show when the user runs `--help subcommand`. If not
    /// specified, uses `help`.
    best::str about;
  };

  /// # `cli::group`
  ///
  /// A reflection tag for specifying a flag group. A field may only
  /// contain one copy of this tag.
  ///
  /// A flag group is a field in a flag struct that is itself another flag
  /// struct. It is intended as a way to group flags, potentially under some
  /// namespace.
  ///
  /// Groups may contain flags and subcommands, but not positionals.
  ///
  /// This is the implicit tag for CLI struct types.
  struct group final {
    /// The name of this group, e.g. "my-component". Must not start or end
    /// with `-` or `_`, nor contain ' ', '.', '=', nor ASCII control characters
    ///
    /// If not specified, will default to the name of the corresponding field.
    /// Subcommand names are insensitive to which of `_` or `-` is used as an
    /// internal separator.
    ///
    /// Flags and subcommands within this group will be made available under
    /// this name, separated by a dot. For example, if this name is "foo" and it
    /// contains a field "bar", it is specified as `--foo.bar`.
    ///
    /// `--foo.help` will show help information for the group's flags only, if
    /// the group visibility is `Hidden` or higher.
    ///
    /// If not specified, this effectively "flattens" the group.
    best::str name;

    /// The short name of this group. Must be any single rune. May not be `-`,
    /// `_`, ' ', '.', '=', nor any ASCII control character. It is incompatible
    /// with `name`.
    ///
    /// If present, all flags in the group are prefixed with this rune. For
    /// example, if it is 'X', and it contains a flag named `foo`, it is
    /// accessible as `-Xfoo` or `-X foo`.
    ///
    /// `-Xhelp` (and `-X help`) will show help information for the group's
    /// flags only, if the group visibility is `Hidden` or higher.
    best::rune letter;

    /// The visibility for this group. Affects all flags behind this group.
    visibility vis = Public;

    /// Help text to show alongside the group.
    best::str help;
  };

  /// # `cli::error`
  ///
  /// An error from parsing flags. This need not be fatal; "errors" are produced
  /// by operations that request usage or version information. The caller is
  /// responsible for printing this error's message to the user (stdout if
  /// non-fatal, stderr otherwise) and then terminating the program with an
  /// appropriate exit code.
  ///
  /// Note that `best::app` will do all this for you.
  class error final {
   public:
    /// # `error::error()`
    ///
    /// Constructs a new error.
    explicit error(best::strbuf message, bool is_fatal)
      : message_(BEST_MOVE(message)), is_fatal_(is_fatal) {}

    /// # `error::message()`
    ///
    /// Returns the error message.
    best::str message() const { return message_; }

    /// # `error::is_fatal()`
    ///
    /// Returns whether this is a fatal message, as defined in the class
    /// documentation.
    bool is_fatal() const { return is_fatal_; }

    /// # `error::print_and_exit()`
    ///
    /// Prints this error to the appropriate output stream and exits the
    /// program.
    [[noreturn]] void print_and_exit(int bad_exit = 128) const;

    friend void BestFmt(auto& fmt, const error& e) {
      fmt.record("cli::error")
        .field("message", e.message())
        .field("is_fatal", e.is_fatal());
    }

   private:
    best::strbuf message_;
    bool is_fatal_ = false;
  };

  cli(const cli&) = delete;
  cli& operator=(const cli&) = delete;
  cli(cli&&);
  cli& operator=(cli&&);

  ~cli();

 private:
  static constexpr positional DefaultTag = {};

  // The parsing context.
  struct context;
  /// A parsing function for a particular field.
  using parser = best::result<void, error> (*)(context&, best::pretext<wtf8>);

  /// A type-erased field-parsing entries.
  struct about final {
    best::vec<best::row<best::strbuf, visibility>> names;
    size_t depth;

    const best::type_names *strukt, *type;
    best::str field;
  };

  explicit cli(best::option<const app&>);

  void add(about, const flag&, const argv_query&, parser);
  void add(about, const subcommand&, cli&);
  void add(about, const group&, cli&);
  void add(about, const positional&, const argv_query&, parser);

  /// Called by the constructor to convert type information (reflection) into
  /// runtime information that init() can handle.
  template <auto downcast>
  void type_erase_field(auto field);

  best::strbuf usage(best::pretext<wtf8> exe, bool hidden) const;

  /// Does all runtime initialization, after we've type erased everything.
  void init();
  /// Prefixes every flag in this CLI.
  void push_prefix(best::str);

  /// Constructs a cli for Flags.
  template <typename Flags, auto downcast>
  static cli build();
  template <typename Flags, auto downcast>
  static cli global_for;

  /// Executes a parse operation.
  best::result<void, error> parse(
    void* flags, best::pretext<wtf8> exe,
    best::span<const best::pretext<wtf8>> argv) const;

  template <typename Flags>
  friend const cli& cli_for();

  template <typename Flags>
  friend best::result<Flags, error> parse_flags(
    best::pretext<best::wtf8> exe,
    best::span<const best::pretext<best::wtf8>> argv);

  struct impl;
  std::unique_ptr<impl> impl_;
};
template <typename Flags, auto downcast>
cli cli::global_for = cli::build<Flags, downcast>();

/// # `best::is_from_argv`.
///
/// A type that can be parsed from a CLI argument. It is any
/// default-constructible type that implements the `BestFromArgv` FTADLE:
///
/// ```
/// friend result<void, MyError> BestFromArgv(auto raw, MyType& arg) {
///   // Parse code here.
/// }
/// ```
///
/// `raw` will be a `best::pretext<best::wtf8>`. `arg` will be a freshly
/// default-constructed value of type `MyType`. You must declare `raw` as
/// `auto`.
///
/// May be called multiple times, as is the case with an `is_tail` positional.
template <typename T>
concept is_from_argv = requires(const ftadle& f, T& value) {
  requires best::constructible<T>;
  requires requires {
    { BestFromArgv(f, value) } -> best::is_result;
  } || requires {
    { BestFromArgv(f, value) } -> best::is_void;
  };
};

/// # `best::argv_query`
///
/// By default, flag values in a struct require arguments, e.g. `--foo bar`.
/// This struct allows configuring that, and other parser behavior, for specific
/// types.
///
/// Those types should provide a FTADLE of the following signature, which is
/// passed a mutable reference of `best::argv_query` that can be used to
/// configure the type's behavior.
///
/// ```
/// constexpr friend void BestFmtQuery(auto& query, MyType*);
/// ```
struct argv_query final {
  /// Set this to `false` so that this type can be called as an argument-less
  /// flag (`bool` is the classic example). `BestFromArgv()` will still be
  /// called, with an empty argument.
  ///
  /// It *may* still be called with an actual argument, to support e.g. `--foo`
  /// and `--foo=no` and so on.
  bool wants_arg = true;

  /// Sets the default count for a specific type. For example, `best::vec`
  /// overrides this to `cli::Repeated`.
  cli::count default_count = cli::Optional;

  /// # `argv_query::of<T>`
  ///
  /// Computes the query of a particular type.
  template <best::is_from_argv T>
  static constexpr auto of = []<typename q = argv_query> {
    q query;
    if constexpr (requires {
                    BestFromArgvQuery(query, best::as_raw_ptr<T>());
                  }) {
      BestFromArgvQuery(query, best::as_raw_ptr<T>());
    }
    return query;
  }
  ();
};
}  // namespace best

#endif  // BEST_CLI_CLI_H_
