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

#include "best/cli/parser.h"

#include <unordered_set>

#include "best/cli/cli.h"
#include "best/log/wtf.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"

namespace best {
struct cli::impl {
  struct f {
    about about;
    const flag* tag;
    const argv_query* query;
    parser parse = nullptr;
    char short_name[4];  // Need to convert the short name to a string for
                         // putting into the lookup table.

    count get_count() const {
      return tag->count.value_or(query->default_count);
    }
  };

  struct s {
    about about;
    const subcommand* tag;
    cli* child;
  };

  struct g {
    about about;
    const group* tag;
    cli* child;
  };

  struct p {
    about about;
    const positional* tag;
    const argv_query* query;
    parser parse = nullptr;

    count get_count() const {
      return tag->count.value_or(query->default_count);
    }
  };

  app app;
  best::vec<f> flags;
  best::vec<s> subs;
  best::vec<g> groups;
  best::vec<p> args;

  /// An element of sorted. Used to look up flags during parsing.
  struct entry final {
    best::strbuf key;  // This does not include leading --.
    size_t idx;
    bool is_group = false, is_letter = false, is_alias = false, is_copy = false;
    visibility vis;
  };

  best::vec<entry> sorted_flags, sorted_subs;

  impl* parent = nullptr;
  const subcommand* parent_sub;
  const group* parent_group;

  /// Pointers to all required flags.
  std::unordered_map<const flag*, best::strbuf> required;

  best::option<const entry&> find_flag(best::pretext<wtf8> tok) const {
    return sorted_flags.as_span()
        .bisect(tok, &entry::key)
        .ok()
        .map([&](size_t idx) -> auto&& { return sorted_flags[idx]; });
  }

  best::option<const entry&> find_sub(best::pretext<wtf8> tok) const {
    return sorted_subs.as_span()
        .bisect(tok, &entry::key)
        .ok()
        .map([&](size_t idx) -> auto&& { return sorted_subs[idx]; });
  }
};

cli::cli(best::option<const app&> app)
    : impl_(new impl{.app = app.value_or()}) {}

cli::cli(cli&&) = default;
cli& cli::operator=(cli&&) = default;
cli::~cli() = default;

namespace {
bool reserved_rune(rune r) {
  return r.is_ascii_control() || r == ' ' || r == '#' || r == '=';
}

cli::visibility merge(cli::visibility a, cli::visibility b) {
  return cli::visibility(best::max(int(a), int(b)));
}

bool visible(cli::visibility v, bool hidden) {
  return v == cli::Public || (hidden && v == cli::Hidden);
}

void normalize(best::strbuf& name, const auto& about) {
  if (name.is_empty()) {
    best::wtf("field {}::{} has an empty name", about.strukt->path(),
              about.field);
  }

  // TODO: also check the ends.
  if (name.starts_with('-') || name.starts_with('_') ||
      name.contains(&reserved_rune)) {
    best::wtf("field {}::{}'s name ({:?}) contains reserved runes",
              about.strukt->path(), about.field, name);
  }

  // Normalize all underscores to dashes. This does not violate UTF-8-ness.
  for (size_t i = 0; i < name.size(); ++i) {
    char& c = name.data()[i];
    if (c == '_') c = '-';
  }
}
}  // namespace

void cli::add(about about, const flag& tag, const argv_query& query,
              parser parser) {
  impl_->flags.push(std::move(about), &tag, &query, parser);
}
void cli::add(about about, const subcommand& tag, cli& child) {
  // Set up back-pointers for the benefit of usage information.
  child.impl_->parent = &*impl_;
  child.impl_->parent_sub = &tag;

  impl_->subs.push(std::move(about), &tag, &child);
}
void cli::add(about about, const group& tag, cli& child) {
  // Set up back-pointers for the benefit of usage information.
  child.impl_->parent = &*impl_;
  child.impl_->parent_group = &tag;

  impl_->groups.push(std::move(about), &tag, &child);
}
void cli::add(about about, const positional& tag, const argv_query& query,
              parser parser) {
  // TODO: validate that Optionals come after Required and there
  // is only one Repeated at the end.
  impl_->args.push(std::move(about), &tag, &query, parser);
}

void cli::init() {
  // First, compute the actual names of all the entries. Stick flags and groups
  // into the lookup tables.

  for (auto [idx, f] : impl_->flags.iter().enumerate()) {
    auto has_letter = f.tag->letter != '\0';
    for (auto [name_idx, pair] : f.about.names.iter().enumerate()) {
      auto& [name, vis] = pair;
      if (vis == Delete) continue;

      normalize(name, f.about);
      if (name == "help" || name == "help-hidden" || name == "version") {
        best::wtf("field {}::{}'s name ({:?}) is reserved and may not be used",
                  f.about.strukt->path(), f.about.field, name);
      }

      impl_->sorted_flags.push(impl::entry{
          .key = name,
          .idx = idx,
          .is_letter = name_idx == 0 && has_letter,
          .is_alias = name_idx > size_t(has_letter),
          .vis = vis,
      });
    }
  }

  for (auto [idx, s] : impl_->subs.iter().enumerate()) {
    for (auto& [name, vis] : s.about.names) {
      if (vis == Delete) continue;
      normalize(name, s.about);

      impl_->sorted_subs.push(impl::entry{.key = name, .idx = idx, .vis = vis});
    }
  }

  // Can't use a nice iterator here, due to potential iterator invalidation due
  // to `impl->groups.append()` below.
  size_t total_groups = impl_->groups.size();
  for (size_t idx = 0; idx < total_groups; ++idx) {
    auto* g = &impl_->groups[idx];

    // Flatten the group.
    size_t flag_offset = impl_->flags.size();
    size_t sub_offset = impl_->subs.size();
    size_t group_offset = impl_->groups.size();

    impl_->flags.append(g->child->impl_->flags);
    impl_->subs.append(g->child->impl_->subs);

    // This zaps `g`.
    impl_->groups.append(g->child->impl_->groups);
    g = &impl_->groups[idx];

    // Update the visibilities of the copied entries.
    auto update = [&](about& about) {
      for (auto& [x, vis] : about.names) {
        vis = merge(vis, g->tag->vis);
      }
    };
    for (auto& f : impl_->flags[{.start = flag_offset}]) {
      update(f.about);
    }
    for (auto& s : impl_->subs[{.start = sub_offset}]) {
      update(s.about);
    }
    for (auto& g : impl_->groups[{.start = group_offset}]) {
      update(g.about);
    }

    auto has_letter = g->tag->letter != '\0';
    for (auto [name_idx, pair] : g->about.names.iter().enumerate()) {
      auto& [name, vis] = pair;
      if (vis == Delete) continue;

      bool is_flatten = !has_letter && name.is_empty();
      if (!is_flatten) {
        normalize(name, g->about);
        if (name == "help" || name == "help-hidden" || name == "version") {
          best::wtf(
              "field {}::{}'s name ({:?}) is reserved and may not be used",
              g->about.strukt->path(), g->about.field, name);
        }

        impl_->sorted_flags.push(impl::entry{
            .key = name,
            .idx = idx,
            .is_group = true,
            .is_letter = name_idx == 0 && has_letter,
            .is_alias = name_idx > size_t(has_letter),
        });

        // Letter names for groups are parsed in a different way that does not
        // require generating keys for them other than the one above.
        if (name_idx == 0 && has_letter) continue;
      }

      auto copy_vis = merge(vis, is_flatten ? Public : Hidden);
      for (auto entry : g->child->impl_->sorted_flags) {
        if (!is_flatten && entry.is_letter) continue;
        if (!name.is_empty()) {
          entry.key = best::format("{}.{}", name, entry.key);
        }
        entry.idx += entry.is_group ? group_offset : flag_offset;
        entry.vis = merge(entry.vis, copy_vis);
        entry.is_copy = !is_flatten;
        impl_->sorted_flags.push(std::move(entry));
      }

      for (auto entry : g->child->impl_->sorted_subs) {
        if (!name.is_empty()) {
          entry.key = best::format("{}.{}", name, entry.key);
        }
        entry.idx += sub_offset;
        entry.vis = merge(entry.vis, copy_vis);
        entry.is_copy = !is_flatten;
        impl_->sorted_subs.push(std::move(entry));
      }
    }
  }

  // Pull out all of the required flags.
  for (const auto& f : impl_->flags) {
    if (!f.tag->count == Required) continue;
    auto name = f.tag->letter != '\0' ? f.about.names[1].first()
                                      : f.about.names[0].first();

    impl_->required.emplace(f.tag, name);
  }

  // Now, sort the flags so we can bisect through them later.
  impl_->sorted_flags.sort(&impl::entry::key);
  impl_->sorted_subs.sort(&impl::entry::key);

  // Check for duplicates.
  best::option<best::str> prev;
  for (auto& e : impl_->sorted_flags) {
    if (e.key == prev) {
      if (e.is_letter) {
        best::wtf("detected duplicate flag: -{}", e.key);
      } else {
        best::wtf("detected duplicate flag: --{}", e.key);
      }
    }
    prev = e.key.as_text();
  }

  prev.reset();
  for (auto& e : impl_->sorted_subs) {
    if (e.key == prev) {
      best::wtf("detected duplicate subcommand: {}", e.key);
    }
    prev = e.key.as_text();
  }

  // All done!
}

best::result<void, cli::error> cli::parse(
    void* args, best::pretext<wtf8> exe,
    best::span<const best::pretext<wtf8>> argv) const {
  context ctx = {
      .exe = exe.split('/').last().value_or(),
      .root = this,
      .sub = this,
      .cur = this,
      .next_positional = 0,
      .args = args,
  };

  bool done_with_flags = false;

  // Pointers to the entries of flags and such that we have seen. This enables
  // us to generate errors for e.g. flag duplication or missing flags.
  std::unordered_set<const flag*> seen;

  auto iter = argv.iter();
  while (auto next = iter.next()) {
    ctx.cur = ctx.sub;
    if (!done_with_flags) {
      bool is_flag = next->starts_with("-");
      bool is_letter = !next->starts_with("--");

      if (next == "--") {
        done_with_flags = true;
        goto again;
      }

      // Peel off the leading dash(es).
      best::pretext<wtf8> flag = *next;
      if (is_letter) {
        flag = flag[{.start = 1}];
      } else if (is_flag) {
        flag = flag[{.start = 2}];
      }

      best::option<pretext<wtf8>> arg;
      if (auto split = flag.split_once("=")) {
        flag = split->first();
        arg = split->second();
      }

      auto push_group = [&](size_t idx, bool update_arg =
                                            true) -> best::result<void, error> {
        // We need to parse a flag from the next argument, of the form e.g.
        // -C opt-level=3.

        if (update_arg) {
          // This type of flag cannot use a = argument.
          if (arg) {
            return best::err(
                best::format("{0}: fatal: unexpected argument after {1}",
                             ctx.exe, *next),
                /*is_fatal=*/true);
          }

          auto next_arg = iter.next();
          if (!next_arg) {
            return best::err(
                best::format("{0}: fatal: expected sub-flag after {1}", ctx.exe,
                             *next),
                /*is_fatal=*/true);
          }

          // Update flag and arg.
          flag = *next_arg;
          if (auto split = flag.split_once("=")) {
            flag = split->first();
            arg = split->second();
          }
        }

        // And nest.
        ctx.cur = ctx.sub->impl_->groups[idx].child;
        return best::ok();
      };

      if (is_letter) {
        // This may be a run of short flags, like -xvzf file, or a single short
        // flag group, like -Copt-level. To discover this, we need to peel off
        // one rune at a time until we are no longer seeing !wants_arg flags.

        auto runes = flag.runes();
        for (rune r : runes) {
          if (r == 'h') {
            return best::err(ctx.cur->usage(ctx.exe, false),
                             /*is_fatal=*/false);
          }

          auto tok = best::format("-{}", r);
          auto e = ctx.cur->impl_->find_flag(tok[{.start = 1}]);

          if (!e || !e->is_letter) break;
          if (e->is_group) {
            // This is a nesting of the form -Copt-level. Unlike the case below,
            // we do not need to update flag/arg if there remain runes to be
            // consumed.
            bool update = runes->rest().is_empty();
            BEST_GUARD(push_group(e->idx, update));
            if (!update) flag = runes->rest();
            continue;
          }

          const auto& f = ctx.cur->impl_->flags[e->idx];
          if (f.query->wants_arg) break;
          ctx.token = tok;

          auto [it, inserted] = seen.insert(f.tag);
          if (!inserted && f.tag->count != Repeated) {
            return best::err(
                best::format("{0}: fatal: flag {1} appeared more than once",
                             ctx.exe, tok),
                /*is_fatal=*/true);
          }

          // Need to be careful here: we might have a !want_arg flag that
          // still got passed an args by =, e.g. as in -xvz=true. In that
          // case, that needs to be passed as the argument, and we are *done*
          // with this argv element.
          if (runes->rest().is_empty()) {
            BEST_GUARD(f.parse(ctx, arg.value_or()));
            goto again;
          }

          BEST_GUARD(f.parse(ctx, ""));
          flag = runes->rest();
        }

        if (flag.is_empty()) goto again;
      }

      ctx.token = *next;
      while (is_flag) {  // This needs to be in a loop to handle nested group
                         // calls.
        if (flag == "help") {
          return best::err(ctx.cur->usage(ctx.exe, false),
                           /*is_fatal=*/false);
        } else if (flag == "help-hidden") {
          return best::err(ctx.cur->usage(ctx.exe, true),
                           /*is_fatal=*/false);
        }

        if (auto e = ctx.cur->impl_->find_flag(flag)) {
          if (e->is_group) {
            BEST_GUARD(push_group(e->idx));
            continue;
          }

          const auto& f = ctx.cur->impl_->flags[e->idx];

          auto [it, inserted] = seen.insert(f.tag);
          if (!inserted && f.tag->count != Repeated) {
            best::str dash = "-";
            if (!is_letter) dash = "--";

            return best::err(
                best::format("{0}: fatal: flag {1}{2} appeared more than once",
                             ctx.exe, dash, flag),
                /*is_fatal=*/true);
          }

          if (!arg && f.query->wants_arg) {
            arg = iter.next();
            if (!arg) {
              return best::err(
                  best::format("{0}: fatal: expected argument after {1}",
                               ctx.exe, *next),
                  /*is_fatal=*/true);
            }
          }

          BEST_GUARD(f.parse(ctx, arg.value_or()));
          goto again;
        }

        // Looks like a bad flag. Users need to pass `--`.
        return best::err(
            best::format("{0}: fatal: unknown flag {1:?}\n"
                         "{0}: you can use `--` if you meant to pass "
                         "this as a positional argument",
                         ctx.exe, *next),
            /*is_fatal=*/true);
      }
    }

    // Look for a relevant subcommand.
    if (auto e = ctx.cur->impl_->find_sub(*next)) {
      ctx.sub = ctx.cur->impl_->subs[e->idx].child;
      goto again;
    }

    // If not, this is definitely a positional.
    if (auto p = ctx.cur->impl_->args.at(ctx.next_positional)) {
      BEST_GUARD(p->parse(ctx, *next));
      if (p->get_count() != Repeated) {
        ++ctx.next_positional;
      }
      goto again;
    }

    return best::err(best::format("{0}: fatal: unexpected extra argument {1:?}",
                                  ctx.exe, *next),
                     /*is_fatal=*/true);

  again:;
  }

  for (const auto& [req, name] : impl_->required) {
    if (!seen.contains(req)) {
      return best::err(
          best::format("{0}: fatal: missing flag --{1}", ctx.exe, name),
          /*is_fatal=*/true);
    }
  }

  return best::ok();
}

namespace {
size_t width_of(best::str s) {
  // Technically not correct. Width for e.g. CJK, ZWJ is different.
  // TODO: crib https://crates.io/crates/unicode-width
  return s.runes().count();
}
}  // namespace

best::strbuf cli::usage(best::pretext<wtf8> exe, bool hidden) const {
  best::strbuf out;
  auto indent = [&](size_t n) {
    for (size_t i = 0; i < n; ++i) out.push(" ");
  };

  auto indent_dots = [&](size_t n) {
    for (size_t i = 0; i < n; ++i) {
      if (i == 0 || i == n - 1 || i % 2 != n % 2) {
        out.push(" ");
      } else {
        out.push(".");
      }
    }
  };

  best::format(out, "Usage: {}", exe);

  // This may be a subcommand/group! We need to trace all the way up.
  best::vec<best::strbuf> parents;
  auto* impl = impl_.get();
  while (impl->parent != nullptr) {
    if (impl->parent_sub) {
      parents.push(impl->parent_sub->name);
    } else if (impl->parent_group->letter != '\0') {
      parents.push(best::format("-{}", impl->parent_group->letter));
    } else if (!impl->parent_group->name.is_empty()) {
      parents.push(best::format("--{}", impl->parent_group->name));
    }

    impl = impl->parent;
  }

  if (!parents.is_empty()) {
    parents.reverse();
    for (auto sub : parents) {
      best::format(out, " {}", sub);
    }
  }
  const auto& app = impl->app;  // This is always the root.

  // We also need to dig our way out of any groups we're in.
  auto* cmd = impl_.get();
  while (cmd->parent_group) cmd = cmd->parent;

  // Append all of the letters, but only if we're inside of a subcommand, not a
  // group.
  if (impl_->parent_group != nullptr) {
    out.push(" [SUBOPTION]");
  }

  bool needs_dash = true;
  for (const auto& e : cmd->sorted_flags) {
    if (!e.is_letter || !visible(e.vis, hidden)) continue;
    if (std::exchange(needs_dash, false)) {
      out.push(" -");
    }
    out.push(e.key);
  }

  if (!cmd->sorted_flags.is_empty()) {
    out.push(" [OPTIONS]");
  }

  // Append all of the subcommands.
  bool first = true;
  for (const auto& s : cmd->sorted_subs) {
    if (s.is_alias) continue;
    if (std::exchange(first, false)) {
      out.push(" [");
    } else {
      out.push("|");
    }
    out.push(s.key);
  }
  if (!first) {
    out.push("]");
  }

  // Append all of the arguments.
  for (auto [idx, p] : cmd->args.iter().enumerate()) {
    if (best::str name = p.tag->name; !name.is_empty()) {
      switch (p.get_count()) {
        case Optional:
          best::format(out, " [{}]", name);
          break;
        case Required:
          best::format(out, " <{}>", name);
          break;
        case Repeated:
          best::format(out, " [{}]...", name);
          break;
      }
    } else {
      switch (p.get_count()) {
        case Optional:
          best::format(out, " [ARG{}]", idx + 1);
          break;
        case Required:
          best::format(out, " <ARG{}>", idx + 1);
          break;
        case Repeated:
          best::format(out, " [ARG{}]...", idx + 1);
          break;
      }
    }
  }

  out.push('\n');

  // Print the help!
  size_t start = out.size();
  if (auto* sub = impl_->parent_sub) {
    if (!sub->about.is_empty()) {
      out.push(sub->about);
    } else {
      out.push(sub->help);
    }
  } else if (auto* group = impl_->parent_group) {
    out.push(group->help);
  } else {
    out.push(app.about);
  }
  if (start < out.size()) {
    out.push("\n\n");
  }

  // This is how wide the column for a flag to be in-line with its
  // help is.
  constexpr size_t Width = 28;

  // Next, print all of the subcommands.
  first = true;
  for (const auto& e : impl_->sorted_subs) {
    if (!visible(e.vis, hidden) || e.is_alias) continue;

    if (std::exchange(first, false)) {
      out.push("# Subcommands\n");
    }

    indent(6);
    out.push(e.key);
    size_t extra = Width - width_of(e.key) - 6;
    if (extra <= Width) {
      indent_dots(extra + 2);
    } else {
      out.push("\n");
      indent(Width + 2);
    }

    bool first = true;
    for (auto line : impl_->subs[e.idx].tag->help.split("\n")) {
      if (!std::exchange(first, false)) indent(Width + 2);
      out.push(line);
      out.push("\n");
    }
  }
  if (!first) out.push('\n');

  out.push("# Flags\n");

  // Next, print all of the flags. The order we want to go in is:
  // 1. Ordinary flags, alphabetized by letter and name.
  // 2. Groups, starting with the top-level group flag and followed by its
  //    subflags.

  best::vec<const cli::impl::entry*> flags;

  // First, add the ordinary flags. We can detect these by looking for
  // flags that do not contain a `.` in their key.
  for (const auto& e : impl_->sorted_flags) {
    if (e.is_alias || e.is_group || e.is_copy) continue;

    const auto& f = impl_->flags[e.idx];
    bool has_letter = f.tag->letter != '\0';

    // This makes us prefer to alphabetize by letter when possible.
    if (has_letter && !e.is_letter) continue;
    flags.push(&e);
  }
  // Now add the groups and their children, which we sort by name rather than by
  // letter.
  for (const auto& e : impl_->sorted_flags) {
    if (e.is_alias || !(e.is_group || e.is_copy)) continue;

    bool has_letter = false;
    if (e.is_group) {
      const auto& g = impl_->groups[e.idx];
      has_letter = g.tag->letter != '\0';
    } else {
      const auto& f = impl_->flags[e.idx];
      has_letter = f.tag->letter != '\0';
    }

    // This makes us prefer to alphabetize by name.
    if (has_letter && e.is_letter) continue;
    flags.push(&e);
  }

  bool first_group = true;
  for (const auto* e : flags) {
    const cli::about* about;
    best::str help, arg;
    bool has_letter;

    if (e->is_group) {
      const auto& g = impl_->groups[e->idx];
      about = &g.about;
      help = g.tag->help;
      arg = "FLAG";
      has_letter = g.tag->letter != '\0';
    } else {
      const auto& f = impl_->flags[e->idx];
      about = &f.about;
      help = f.tag->help;
      has_letter = f.tag->letter != '\0';

      if (f.query->wants_arg) {
        arg = f.tag->arg;
        if (arg.is_empty()) arg = "ARG";
      }
    }

    if ((e->is_group || e->is_copy) && std::exchange(first_group, false)) {
      out.push("\n");
    }

    if (!visible(e->vis, hidden)) continue;

    size_t start = out.size();
    out.push("  ");
    if (auto [letter, vis] = about->names[0];
        has_letter && !e->is_copy && visible(vis, hidden)) {
      best::format(out, "-{}, ", letter);
    } else {
      indent(4);
    }

    best::str prefix = e->key[{
        .end = e->key.size() - e->key.split('.').last()->size(),
    }];
    // Chop off everything past the last `.` to make the prefix.

    auto names = about->names[{.start = has_letter ? 1 : 0}].iter();
    auto helps = help.split("\n");
    bool first = true;
    for (auto [name, vis] : names) {
      if (!visible(vis, hidden)) continue;

      bool is_first = std::exchange(first, false);
      if (!is_first) {
        start = out.size();
        indent(6);
      }

      bool needs_comma = false;
      for (auto [name, vis] : names->rest()) {
        needs_comma |= visible(vis, hidden);
      }

      best::format(out, "--{}{}", prefix, name);
      if (!arg.is_empty()) {
        out.push(" ");
        out.push(arg);
      }

      if (needs_comma) out.push(',');

      auto help = helps.next();

      size_t extra = Width - (out.size() - start);
      if (extra <= Width) {
        if (is_first) {
          indent_dots(extra + 2);
        } else {
          indent(extra + 2);
        }
        out.push(help.value_or());
      } else if (help) {
        out.push("\n");
        indent(Width + 2);
        out.push(*help);
      }
      out.push("\n");
    }
    for (auto help : helps) {
      indent(Width + 2);
      out.push(help);
      out.push("\n");
    }
  }

  out.push("\n");
  start = out.size();
  best::format(out, "  -h, --help");
  size_t extra = Width - (out.size() - start);
  indent_dots(extra + 2);
  out.push("show usage and exit\n");

  if (hidden) {
    start = out.size();
    best::format(out, "      --help-hidden");
    extra = Width - (out.size() - start);
    indent_dots(extra + 2);
    out.push("show extended usage and exit\n");
  }

  return out;
}
}  // namespace best
