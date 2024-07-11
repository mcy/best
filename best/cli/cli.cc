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

#include "best/text/format.h"

namespace best {

namespace {
inline constexpr best::str Reset = "\N{ESCAPE}[0m";
inline constexpr best::str Red = "\N{ESCAPE}[31m";
}  // namespace

void cli::error::print_and_exit(int bad_exit) const {
  if (is_fatal()) {
    best::eprintln("{}{}{}", Red, message(), Reset);
    std::exit(128);
  } else {
    best::println("{}", message());
    std::exit(0);
  }
}
}  // namespace best
