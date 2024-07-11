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

#ifndef BEST_CLI_TOY_FLAGS_H_
#define BEST_CLI_TOY_FLAGS_H_

#include "best/cli/cli.h"

//! Flags for helping test the CLI library.

namespace best::cli_toy {
struct Subcommand {
  int sub_flag = 0;
  best::str arg;

  friend constexpr auto BestReflect(auto& m, Subcommand*) {
    using ::best::cli;
    return m.infer().with(&Subcommand::sub_flag,
                          cli::flag{
                              .letter = 's',
                              .arg = "INT",
                              .help = "a subcommand argument",
                          });
  }

  bool operator==(const Subcommand&) const = default;
};

struct Subgroup {
  int eks = 0, why = 0, zed = 0;

  int a_flag_with_a_freakishly_long_name = 0;

  friend constexpr auto BestReflect(auto& m, Subgroup*) {
    using ::best::cli;
    return m.infer()
        .with(&Subgroup::eks,
              cli::flag{
                  .letter = 'x',
                  .arg = "INT",
                  .help = "a group integer",
              })
        .with(&Subgroup::why,
              cli::flag{
                  .letter = 'y',
                  .arg = "INT",
                  .help = "another group integer",
              })
        .with(&Subgroup::zed,
              cli::flag{
                  .letter = 'z',
                  .arg = "INT",
                  .help = "a third group integer",
              })
        .with(&Subgroup::a_flag_with_a_freakishly_long_name,
              cli::flag{
                  .arg = "INT",
                  .help = "like, freakishly long man",
              });
  }

  bool operator==(const Subgroup&) const = default;
};

struct Toy {
  int foo = 0;
  best::vec<int> bar;
  best::option<int> baz;
  best::str name, addr;

  best::option<bool> flag1, flag2, flag3, flag4;

  Subcommand sub;
  Subcommand sub2;

  Subgroup group;
  Subgroup flattened;

  int undocumented = 0;

  best::str arg;
  best::vec<best::str> args;

  friend constexpr auto BestReflect(auto& m, Toy*) {
    using ::best::cli;
    return m.infer()
        .with(cli::app{
            .name = "toy",
            .authors = "mcyoung",
            .about = "this is a test binary for playing with all of\n"
                     "best::cli's features",
            .version = "1.0.0",
            .url = "https://mcyoung.xyz",
            .copyright_year = 2024,
            .license = "Apache-2.0",
        })
        .with(&Toy::foo,
              cli::flag{
                  .letter = 'f',
                  .arg = "INT",
                  .count = cli::Repeated,
                  .help = "an integer",
              })
        .with(&Toy::bar,
              cli::flag{
                  .arg = "INT",
                  .help = "repeated integer",
              })
        .with(&Toy::baz,
              cli::flag{
                  .help = "another integer",
              })

        .with(&Toy::name,
              cli::flag{
                  .vis = cli::Hidden,
                  .help = "your name",
              },
              cli::alias{"my-name"})
        .with(&Toy::addr,
              cli::flag{
                  .vis = cli::Hidden,
                  .help = "your address",
              },
              cli::alias{"my-address"})

        .with(&Toy::flag1,
              cli::flag{
                  .letter = 'a',
                  .help = "this is a flag\nnewline",
              })
        .with(&Toy::flag2,
              cli::flag{
                  .letter = 'b',
                  .count = cli::Repeated,
                  .help = "this is a flag\nnewline",
              })
        .with(&Toy::flag3,
              cli::flag{
                  .letter = 'c',
                  .help = "this is a flag\nnewline",
              },
              cli::alias{"flag3-alias"},
              cli::alias{"flag3-alias2", cli::Hidden})
        .with(&Toy::flag4,
              cli::flag{
                  .letter = 'd',
                  .help = "this is a flag\nnewline",
              })
        .with(&Toy::undocumented, cli::flag{})

        .with(
            &Toy::sub,
            cli::subcommand{
                .help = "a subcommand",
                .about = "longer help for the subcommand\nwith multiple lines",
            })
        .with(
            &Toy::sub2,
            cli::subcommand{
                .help = "identical in all ways to `sub`\nexcept for this help",
                .about = "longer help for the subcommand\nwith multiple lines",
            },
            cli::alias{"sub3"})

        .with(&Toy::group,
              cli::group{
                  .name = "subgroup",
                  .letter = 'X',
                  .help = "extra options behind the -X flag",
              })
        .with(&Toy::flattened, cli::group{});
  }

  bool operator==(const Toy&) const = default;
};
}  // namespace best::cli_toy
#endif  // BEST_CLI_TOY_FLAGS_H_
