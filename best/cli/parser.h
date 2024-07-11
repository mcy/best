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

#ifndef BEST_CLI_PARSER_H_
#define BEST_CLI_PARSER_H_

#include "best/cli/cli.h"
#include "best/container/result.h"
#include "best/math/conv.h"
#include "best/memory/allocator.h"
#include "best/meta/names.h"
#include "best/meta/reflect.h"
#include "best/text/format.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"
#include "best/text/utf8.h"

//! Command-line flags.
//!

namespace best {
/// # `best::cli_flags()`
///
/// Constructs a `best::cli` for the given flag struct type and returns a
/// reference to it.
template <typename Flags>
const cli& cli_for() {
  return cli::global_for<Flags,
                         [](void* args) { return static_cast<Flags*>(args); }>;
}

/// # `best::parse_flags()`
///
/// Parses a flags struct using reflection. If parsing fails, returns an error;
/// otherwise, returns the parsed flags.
///
/// Note that the name of the current binary is assumed to *not* be in argv, and
/// is passed as a separate argument.
template <typename Flags>
best::result<Flags, best::cli::error> parse_flags(
    best::pretext<best::wtf8> exe,
    best::span<const best::pretext<best::wtf8>> argv) {
  Flags flags;
  BEST_GUARD(best::cli_for<Flags>().parse(&flags, exe, argv));
  return flags;
}
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename Flags, auto downcast>
cli cli::build() {
  // First, obtain the top-level CLI descriptor.
  auto tags = best::reflect<Flags>.tags(best::types<best::cli::app>);
  static_assert(tags.size() < 2,
                "found two or more best::cli_apps on flags struct");

  const cli::app* app = nullptr;
  if constexpr (!tags.is_empty()) {
    app = &tags.first();
  }

  best::cli cli(app);

  // Then, type-erase all the entries.
  best::reflect<Flags>.each(
      [&](auto field) { cli.type_erase_field<downcast>(field); });

  // Having that, draw the rest of the frickin' owl.
  cli.init();

  return cli;
}

struct cli::context final {
  best::pretext<wtf8> exe, token;
  const cli *root, *sub, *cur;
  const about* about;
  size_t next_positional;
  void* args;
};

template <auto downcast>
void cli::type_erase_field(auto f) {
  using Field = decltype(f);
  using Flags = Field::reflected;
  using Arg = Field::type;
  constexpr Field field;

  static constexpr auto aliases = field.tags(best::types<alias>);

  static constexpr auto flags = field.tags(best::types<flag>);
  static constexpr auto subs = field.tags(best::types<subcommand>);
  static constexpr auto groups = field.tags(best::types<group>);
  static constexpr auto poses = field.tags(best::types<positional>);

  static_assert(
      (flags.size() + subs.size() + groups.size() + poses.size()) <= 1,
      "cannot have more than one of best::cli_flag, best::cli_subcommand, or "
      "best::cli_group attached to a field");

  static constexpr auto Downcast = [](void* args) -> auto& {
    return downcast(args)->*Field{};
  };

  cli::about about{
      .strukt = &best::type_names::of<Flags>,
      .type = &best::type_names::of<Arg>,
      .field = field.name(),
  };

  if constexpr (!flags.is_empty()) {
    constexpr const cli::flag& flag = flags.first();
    if (flag.letter != '\0') {
      about.names.push(best::format("{}", flag.letter), flag.vis);
    }

    best::str name = flag.name;
    if (name.is_empty()) name = about.field;
    about.names.push(best::strbuf(name), flag.vis);
  } else if constexpr (!subs.is_empty()) {
    constexpr const cli::subcommand& sub = subs.first();

    best::str name = sub.name;
    if (name.is_empty()) name = about.field;
    about.names.push(best::strbuf(name), sub.vis);
  } else if constexpr (!groups.is_empty()) {
    constexpr const cli::group& group = groups.first();
    if (group.letter != '\0') {
      about.names.push(best::format("{}", group.letter), group.vis);
    }

    about.names.push(best::strbuf(group.name), group.vis);
  }

  aliases.each([&](auto alias) {
    static_assert(sizeof(alias) == 0 || poses.is_empty(),
                  "cannot apply cli::alias to a positional");
    about.names.push(best::strbuf(alias.name),
                     alias.vis.value_or(about.names[0].second()));
  });

  if constexpr (!flags.is_empty()) {
    static_assert(best::is_from_argv<Arg>,
                  "flag type must implement BestFromArgv()");

    constexpr const best::cli::flag& flag = flags.first();
    add(
        std::move(about), flag, argv_query::of<Arg>,
        +[](context& ctx,
            best::pretext<wtf8> argv) -> best::result<void, error> {
          auto result = BestFromArgv(argv, Downcast(ctx.args));
          if (result) return best::ok();

          return best::err(
              best::format("{}: fatal: could not parse argument for {}: {}",
                           ctx.exe, ctx.token, *result.err()),
              /*is_fatal=*/true);
        });
  } else if constexpr (!subs.is_empty()) {
    constexpr const best::cli::subcommand& sub = subs.first();
    add(std::move(about), sub, global_for<Arg, Downcast>);
  } else if constexpr (!groups.is_empty()) {
    constexpr const best::cli::group& group = groups.first();
    add(std::move(about), group, global_for<Arg, Downcast>);
  } else {
    static_assert(best::is_from_argv<Arg>,
                  "positional type must implement BestFromArgv()");

    const best::cli::positional* pos = &DefaultTag;
    if constexpr (!poses.is_empty()) {
      pos = &poses.first();
    }

    add(
        std::move(about), *pos, argv_query::of<Arg>,
        +[](context& ctx,
            best::pretext<wtf8> argv) -> best::result<void, error> {
          auto result = BestFromArgv(argv, Downcast(ctx.args));
          if (result) return best::ok();

          return best::err(
              best::format("{}: fatal: could not parse argument: {}", ctx.exe,
                           ctx.token, *result.err()),
              /*is_fatal=*/true);
        });
  }
}
// FromArgv implementations for common types.
result<void, best::strbuf> BestFromArgv(auto raw, bool& arg) {
  static constexpr std::array<best::str, 5> True = {"true", "t", "yes", "y",
                                                    "on"};
  static constexpr std::array<best::str, 5> False = {"false", "f", "no", "n",
                                                     "off"};

  // TODO: convert `raw` to lowercase first.

  if (raw.starts_with(&rune::is_ascii_digit)) {
    if (auto i = best::atoi_with_prefix<uint8_t>(raw); i == 0 || i == 1) {
      arg = i == 1;
      return best::ok();
    }
  }
  if (raw.is_empty() || best::span(True).contains(raw)) {
    arg = true;
    return best::ok();
  } else if (best::span(False).contains(raw)) {
    arg = false;
    return best::ok();
  }
  return best::format("invalid bool: {:?}", raw);
}
constexpr void BestFromArgvQuery(auto& query, bool*) {
  query.wants_arg = false;
}

result<void, best::strbuf> BestFromArgv(auto raw, best::integer auto& arg) {
  if (auto i = best::atoi_with_prefix<best::as_auto<decltype(arg)>>(raw)) {
    arg = *i;
    return best::ok();
  }

  return best::format("invalid integer: {:?}", raw);
}

result<void, best::strbuf> BestFromArgv(auto raw, best::rune& arg) {
  auto it = raw.try_runes();
  auto next = it.next();
  if (!next || !*next || !it->rest().is_empty()) {
    return best::format("invalid rune: {:?}", raw);
  }
  arg = **next;
  return best::ok();
}

template <typename E>
result<void, best::strbuf> BestFromArgv(auto raw, best::text<E>& arg)
  requires best::same<best::code<best::wtf8>, best::code<E>>
{
  if (auto valid = best::text<E>::from(raw)) {
    arg = *valid;
    return best::ok();
  }

  return best::format("string is not correctly encoded: {:?}", raw);
}

template <typename E, best::allocator A>
result<void, best::strbuf> BestFromArgv(auto raw, best::textbuf<E, A>& arg)
  requires best::constructible<A>
{
  if (auto valid = best::textbuf<E, A>::transcode(raw)) {
    arg = BEST_MOVE(*valid);
    return best::ok();
  }

  return best::format("string is not correctly encoded: {:?}", raw);
}

template <best::is_from_argv T>
auto BestFromArgv(auto raw, best::option<T>& arg)
    -> decltype(BestFromArgv(raw, arg.emplace())) {
  return BestFromArgv(raw, arg.emplace());
}
template <best::is_from_argv T>
constexpr auto BestFromArgvQuery(auto& query, best::option<T>*) {
  query.wants_arg = best::argv_query::of<T>.wants_arg;
}

template <best::is_from_argv T, size_t n, best::allocator A>
auto BestFromArgv(auto raw, best::vec<T, n, A>& arg)
    -> decltype(BestFromArgv(raw, arg.push())) {
  return BestFromArgv(raw, arg.push());
}
template <best::is_from_argv T, size_t n, best::allocator A>
constexpr auto BestFromArgvQuery(auto& query, best::vec<T, n, A>*) {
  query.wants_arg = best::argv_query::of<T>.wants_arg;
  query.default_count = cli::Repeated;
}
}  // namespace best

#endif  // BEST_CLI_PARSER_H_
