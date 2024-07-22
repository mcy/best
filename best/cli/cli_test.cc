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

#include "best/cli/cli.h"

#include "best/cli/parser.h"
#include "best/cli/toy_flags.h"
#include "best/test/test.h"

namespace best::parser_test {
using ::best::cli_toy::Toy;
constexpr best::str Exe = "cli_test";

template <typename Flags = Toy>
void expect_ok(best::test& t, best::span<const best::pretext<wtf8>> args,
               const Flags& expect, best::location loc = best::here) {
  auto parsed = best::parse_flags<Flags>(Exe, args);
  if (!parsed) {
    t.fail({"parse of {:?} failed with message {:?}", loc}, args,
           parsed.err()->message());
    return;
  }
  t.expect_eq(*parsed, expect, {"args: {:?}", loc}, args);
}

template <typename Flags = Toy>
void expect_usage(best::test& t, best::span<const best::pretext<wtf8>> args,
                  best::str expect, best::location loc = best::here) {
  auto parsed = best::parse_flags<Flags>(Exe, args);
  if (parsed) {
    t.fail({"parse of {:?} succeeded: {:#?}", loc}, args, *parsed);
    return;
  }
  if (parsed.err()->is_fatal()) {
    t.fail({"parse of {:?} failed unexpectedly: {:?}", loc}, args,
           parsed.err()->message());
    return;
  }
  t.expect_eq(parsed.err()->message(), expect, {"args: {:?}", loc}, args);
}

template <typename Flags = Toy>
void expect_fail(best::test& t, best::span<const best::pretext<wtf8>> args,
                 best::str expect, best::location loc = best::here) {
  auto parsed = best::parse_flags<Flags>(Exe, args);
  if (parsed) {
    t.fail({"parse of {:?} succeeded: {:#?}", loc}, args, *parsed);
    return;
  }
  if (!parsed.err()->is_fatal()) {
    t.fail({"parse of {:?} printed usage unexpectedly: {:?}", loc}, args,
           parsed.err()->message());
    return;
  }
  t.expect_eq(parsed.err()->message(), expect, {"args: {:?}", loc}, args);
}

best::test TopLevelFlags = [](auto& t) {
  expect_ok(t, {}, {});

  expect_ok(t, {"--foo", "42"}, {.foo = 42});
  expect_ok(t, {"--foo", "0x42"}, {.foo = 66});
  expect_ok(t, {"--foo=42"}, {.foo = 42});
  expect_ok(t, {"-f", "42"}, {.foo = 42});
  expect_ok(t, {"--f", "42"}, {.foo = 42});
  expect_ok(t, {"-f=42"}, {.foo = 42});
  expect_ok(t, {"--foo", "42", "--foo=0x42"}, {.foo = 66});
  expect_fail(t, {"--foo"}, "cli_test: fatal: expected argument after --foo");
  expect_fail(t, {"--foo", "bar"},
              "cli_test: fatal: could not parse argument for --foo: invalid "
              "integer: \"bar\"");
  expect_fail(t, {"--foo=bar"},
              "cli_test: fatal: could not parse argument for --foo: invalid "
              "integer: \"bar\"");

  expect_ok(t, {"--bar", "42"}, {.bar = {42}});
  expect_ok(t, {"--bar", "42", "--bar=0x42"}, {.bar = {42, 66}});

  expect_ok(t, {"--baz", "42"}, {.baz = 42});
  expect_fail(t, {"--baz=42", "--baz=42"},
              "cli_test: fatal: flag --baz appeared more than once");

  expect_ok(t, {"--name", "solomon", "--addr=cambridge"},
            {
              .name = "solomon",
              .addr = "cambridge",
            });
  expect_ok(t, {"--my-name", "solomon", "--my-address=cambridge"},
            {
              .name = "solomon",
              .addr = "cambridge",
            });
  expect_ok(t, {"--my-name", "🧶🐈‍⬛", "--my-address==="},
            {
              .name = "🧶🐈‍⬛",
              .addr = "==",
            });

  expect_ok(t, {"-a"}, {.flag1 = true});
  expect_ok(t, {"-acb"}, {.flag1 = true, .flag2 = true, .flag3 = true});
  expect_ok(t, {"-cbf", "42"}, {.foo = 42, .flag2 = true, .flag3 = true});
  expect_ok(t, {"-cbf=42"}, {.foo = 42, .flag2 = true, .flag3 = true});

  expect_fail(t, {"-a", "-a"},
              "cli_test: fatal: flag -a appeared more than once");
  expect_fail(t, {"-aa"}, "cli_test: fatal: flag -a appeared more than once");
  expect_ok(t, {"-bb"}, {.flag2 = true});
  expect_ok(t, {"-b", "-b"}, {.flag2 = true});

  expect_ok(t, {"--flag2=true"}, {.flag2 = true});
  expect_ok(t, {"--flag2=t"}, {.flag2 = true});
  expect_ok(t, {"--flag2=yes"}, {.flag2 = true});
  expect_ok(t, {"--flag2=y"}, {.flag2 = true});
  expect_ok(t, {"--flag2=on"}, {.flag2 = true});
  expect_ok(t, {"--flag2=1"}, {.flag2 = true});
  expect_ok(t, {"--flag2=0x1"}, {.flag2 = true});
  expect_ok(t, {"--flag2=0b001"}, {.flag2 = true});

  expect_ok(t, {"--flag2=false"}, {.flag2 = false});
  expect_ok(t, {"--flag2=f"}, {.flag2 = false});
  expect_ok(t, {"--flag2=no"}, {.flag2 = false});
  expect_ok(t, {"--flag2=n"}, {.flag2 = false});
  expect_ok(t, {"--flag2=off"}, {.flag2 = false});
  expect_ok(t, {"--flag2=0"}, {.flag2 = false});
  expect_ok(t, {"--flag2=0x0"}, {.flag2 = false});
  expect_ok(t, {"--flag2=0b00"}, {.flag2 = false});

  expect_ok(
    t,
    {"-x", "5", "--why", "6", "-z=7", "--a-flag-with-a-freakishly-long-name=8"},
    {
      .flattened =
        {.eks = 5, .why = 6, .zed = 7, .a_flag_with_a_freakishly_long_name = 8},
    });
  expect_fail(
    t, {"--flattened.eks"},
    "cli_test: fatal: unknown flag \"--flattened.eks\"\ncli_test: you can "
    "use `--` if you meant to pass this as a positional argument");
};

best::test Group = [](auto& t) {
  expect_ok(t, {"-X", "eks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-X", "eks=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-Xeks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-Xeks=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-X", "x", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-X", "x=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-Xx", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"-Xx=5"}, {.group = {.eks = 5}});

  expect_ok(t, {"--X", "eks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--X", "eks=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--Xeks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--Xeks=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--X", "x", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--X", "x=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--Xx", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--Xx=5"}, {.group = {.eks = 5}});

  expect_ok(t, {"--subgroup", "x", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup", "x=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup.x", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup.x=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup", "eks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup", "eks=5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup.eks", "5"}, {.group = {.eks = 5}});
  expect_ok(t, {"--subgroup.eks=5"}, {.group = {.eks = 5}});
};

best::test Sub = [](auto& t) {
  expect_ok(t, {"sub", "-s", "42", "foo"},
            {.sub = {.sub_flag = 42, .arg = "foo"}});
};

best::test Help = [](auto& t) {
  best::str usage =
    R"help(Usage: cli_test -Xabcdfhxyz [OPTIONS] [sub|sub2|sub3] [ARG1] [ARG2]...
this is a test binary for playing with all of
best::cli's features

# Subcommands
      sub . . . . . . . . . . a subcommand
      sub2  . . . . . . . . . identical in all ways to `sub`
                              except for this help
      sub3  . . . . . . . . . identical in all ways to `sub`
                              except for this help

# Flags
  -a, --flag1 . . . . . . . . this is a flag
                              newline
      --a-flag-with-a-freakishly-long-name INT
                              like, freakishly long man
  -b, --flag2 . . . . . . . . this is a flag
                              newline
      --bar INT . . . . . . . repeated integer
      --baz ARG . . . . . . . another integer
  -c, --flag3,  . . . . . . . this is a flag
      --flag3-alias           newline
  -d, --flag4 . . . . . . . . this is a flag
                              newline
  -f, --foo INT . . . . . . . an integer
  -x, --eks INT . . . . . . . a group integer
  -y, --why INT . . . . . . . another group integer
  -z, --zed INT . . . . . . . a third group integer

  -X, --subgroup FLAG . . . . extra options behind the -X flag

  -h, --help  . . . . . . . . show usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";
  best::str hidden =
    R"help(Usage: cli_test -Xabcdfhxyz [OPTIONS] [sub|sub2|sub3] [ARG1] [ARG2]...
this is a test binary for playing with all of
best::cli's features

# Subcommands
      sub . . . . . . . . . . a subcommand
      sub2  . . . . . . . . . identical in all ways to `sub`
                              except for this help
      sub3  . . . . . . . . . identical in all ways to `sub`
                              except for this help

# Flags
  -a, --flag1 . . . . . . . . this is a flag
                              newline
      --a-flag-with-a-freakishly-long-name INT
                              like, freakishly long man
      --addr ARG, . . . . . . your address
      --my-address ARG        
  -b, --flag2 . . . . . . . . this is a flag
                              newline
      --bar INT . . . . . . . repeated integer
      --baz ARG . . . . . . . another integer
  -c, --flag3,  . . . . . . . this is a flag
      --flag3-alias,          newline
      --flag3-alias2          
  -d, --flag4 . . . . . . . . this is a flag
                              newline
  -f, --foo INT . . . . . . . an integer
      --name ARG, . . . . . . your name
      --my-name ARG           
      --undocumented ARG  . . <undocumented>
  -x, --eks INT . . . . . . . a group integer
  -y, --why INT . . . . . . . another group integer
  -z, --zed INT . . . . . . . a third group integer

  -X, --subgroup FLAG . . . . extra options behind the -X flag
      --subgroup.a-flag-with-a-freakishly-long-name INT
                              like, freakishly long man
      --subgroup.eks INT  . . a group integer
      --subgroup.why INT  . . another group integer
      --subgroup.zed INT  . . a third group integer

  -h, --help  . . . . . . . . show usage and exit
      --help-hidden . . . . . show extended usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";

  best::str group =
    R"help(Usage: cli_test -X [SUBOPTION] -Xabcdfhxyz [OPTIONS] [sub|sub2|sub3] [ARG1] [ARG2]...
extra options behind the -X flag

# Flags
      --a-flag-with-a-freakishly-long-name INT
                              like, freakishly long man
  -x, --eks INT . . . . . . . a group integer
  -y, --why INT . . . . . . . another group integer
  -z, --zed INT . . . . . . . a third group integer

  -h, --help  . . . . . . . . show usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";

  best::str group_hidden =
    R"help(Usage: cli_test -X [SUBOPTION] -Xabcdfhxyz [OPTIONS] [sub|sub2|sub3] [ARG1] [ARG2]...
extra options behind the -X flag

# Flags
      --a-flag-with-a-freakishly-long-name INT
                              like, freakishly long man
  -x, --eks INT . . . . . . . a group integer
  -y, --why INT . . . . . . . another group integer
  -z, --zed INT . . . . . . . a third group integer

  -h, --help  . . . . . . . . show usage and exit
      --help-hidden . . . . . show extended usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";

  best::str sub =
    R"help(Usage: cli_test sub -hs [OPTIONS] [ARG1]
longer help for the subcommand
with multiple lines

# Flags
  -s, --sub-flag INT  . . . . a subcommand argument

  -h, --help  . . . . . . . . show usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";

  best::str sub_hidden =
    R"help(Usage: cli_test sub -hs [OPTIONS] [ARG1]
longer help for the subcommand
with multiple lines

# Flags
  -s, --sub-flag INT  . . . . a subcommand argument

  -h, --help  . . . . . . . . show usage and exit
      --help-hidden . . . . . show extended usage and exit

Version: toy v1.0.0
Website: <https://mcyoung.xyz>

(c) 2024 mcyoung, licensed Apache-2.0
)help";

  expect_usage(t, {"--help"}, usage);
  expect_usage(t, {"-h"}, usage);
  expect_usage(t, {"-ah"}, usage);
  expect_usage(t, {"--help-hidden"}, hidden);

  expect_usage(t, {"-X", "h"}, group);
  expect_usage(t, {"-Xh"}, group);
  expect_usage(t, {"--subgroup", "h"}, group);
  expect_usage(t, {"--subgroup.h"}, group);

  expect_usage(t, {"-X", "help"}, group);
  expect_usage(t, {"-Xhelp"}, group);
  expect_usage(t, {"--subgroup", "help"}, group);
  expect_usage(t, {"--subgroup.help"}, group);

  expect_usage(t, {"-X", "help-hidden"}, group_hidden);
  expect_usage(t, {"-Xhelp-hidden"},
               group);  // A fun case: -Xh is a prefix so
                        // that's what gets parsed here.
                        // This is a nasty corner case that
                        // feels not worth fixing.
  expect_usage(t, {"--subgroup", "help-hidden"}, group_hidden);
  expect_usage(t, {"--subgroup.help-hidden"}, group_hidden);

  expect_usage(t, {"sub", "--help"}, sub);
  expect_usage(t, {"sub", "-h"}, sub);
  expect_usage(t, {"sub", "--help-hidden"}, sub_hidden);
};
}  // namespace best::parser_test
