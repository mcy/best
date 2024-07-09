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
#include "best/cli/toy_flags.h"
#include "best/test/test.h"

namespace best::flag_parser_test {
using ::best::cli_toy::MyFlags;

best::test Flags = [](auto& t) {
  best::parse_flags<MyFlags>("test", {"--help"}).err()->print_and_exit();
};
}  // namespace best::flag_parser_test
