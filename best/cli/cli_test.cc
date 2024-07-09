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

// #include "best/cli/flag_parser.h"

#include "best/cli/cli.h"

#include "best/cli/parser.h"
#include "best/test/test.h"

namespace best::flag_parser_test {

struct Subcommand {
  int sub_flag;
  best::str arg;

  friend constexpr auto BestReflect(auto& m, Subcommand*) {
    using ::best::cli;
    return m.infer()->*m.field(best::vals<&Subcommand::sub_flag>,
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

  friend constexpr auto BestReflect(auto& m, Subgroup*) {
    using ::best::cli;
    return m.infer()
            ->*m.field(best::vals<&Subgroup::eks>, cli::flag{
              .letter = 'x',
              .arg = "INT",
              .help = "a group integer",
            })
            ->*m.field(best::vals<&Subgroup::why>, cli::flag{
              .letter = 'y',
              .arg = "INT",
              .help = "another group integer",
            })
            ->*m.field(best::vals<&Subgroup::zed>, cli::flag{
              .letter = 'z',
              .arg = "INT",
              .help = "a third group integer",
            });
  }

  bool operator==(const Subgroup&) const = default;
};

struct MyFlags {
  int foo = 0;
  best::vec<int> bar;
  best::option<int> baz;
  best::str name, addr;

  bool flag1 = false;
  bool flag2 = false;
  bool flag3 = false;
  bool flag4 = false;

  Subcommand sub;
  Subcommand sub2;

  Subgroup group;
  Subgroup flattened;

  best::str arg;
  best::vec<best::str> args;

  friend constexpr auto BestReflect(auto& m, MyFlags*) {
    using ::best::cli;
    return m.infer()
            ->*m.field(best::vals<&MyFlags::foo>, cli::flag{
              .letter = 'f',
              .arg = "INT",
              .count = cli::Required,
              .help = "a required integer",
            })
            ->*m.field(best::vals<&MyFlags::bar>, cli::flag{
              .arg = "INT",
              .help = "repeated integer",
            })
            ->*m.field(best::vals<&MyFlags::baz>, cli::flag{
              .help = "an optional integer",
            })

            ->*m.field(best::vals<&MyFlags::name>, cli::flag{
              .vis = cli::Hidden,
              .help = "your name",
            }, cli::alias{"my-name"})
            ->*m.field(best::vals<&MyFlags::addr>, cli::flag{
              .vis = cli::Hidden,
              .help = "your address",
            }, cli::alias{"my-address"})
            
            ->*m.field(best::vals<&MyFlags::flag1>, cli::flag{
              .letter = 'a',
              .help = "this is a flag\nnewline",
            })
            ->*m.field(best::vals<&MyFlags::flag2>, cli::flag{
              .letter = 'b',
              .help = "this is a flag\nnewline",
            })
            ->*m.field(best::vals<&MyFlags::flag3>, cli::flag{
              .letter = 'c',
              .help = "this is a flag\nnewline",
            }, cli::alias{"flag3-alias"},
               cli::alias{"flag3-alias2", cli::Hidden})
            ->*m.field(best::vals<&MyFlags::flag4>, cli::flag{
              .letter = 'd',
              .help = "this is a flag\nnewline",
            })
            
            ->*m.field(best::vals<&MyFlags::sub>, cli::subcommand{
              .help = "a subcommand",
              .about = "longer help for the subcommand\nwith multiple lines",
            })
            ->*m.field(best::vals<&MyFlags::sub2>, cli::subcommand{
              .help = "identical in all ways to `sub`\nexceptt for this help",
              .about = "longer help for the subcommand\nwith multiple lines",
            }, cli::alias{"sub3"})
            
            ->*m.field(best::vals<&MyFlags::group>, cli::group{
              .name = "subgroup",
              .letter = 'X',
              .help = "extra options behind the -X flag",
            })
            ->*m.field(best::vals<&MyFlags::flattened>, cli::group{}, cli::alias{"sub3"});
  }

  bool operator==(const MyFlags&) const = default;
};

best::test Flags = [](auto& t) {
  best::parse_flags<MyFlags>("test", {"--help"}).err()->print_and_exit();
};
}  // namespace best::flag_parser_test
