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

#include "best/test/test.h"

#include <dlfcn.h>

#include <cstdlib>

#include "best/container/vec.h"

namespace best {
namespace {
best::vec<best::test*> all_tests;

best::str symbol_name(const void* ptr, best::location loc) {
  ::Dl_info di;
  if (::dladdr(ptr, &di)) {
    if (di.dli_sname == nullptr) {
      best::eprintln(
          "fatal: could not parse symbol name for test at {:?}\nyou may need "
          "to pass `-rdynamic` as part of your link options\n",
          loc);
      std::exit(128);
    }
    return *best::str::from_nul(di.dli_sname);
  } else {
    best::eprintln(
        "fatal: could not parse symbol name for test at {:?}\nit might not be "
        "a global variable?",
        loc);
    std::exit(128);
  }
}

static_assert(best::rune::validate("\N{ESCAPE}[0m"));
inline constexpr best::str Reset = "\N{ESCAPE}[0m";
inline constexpr best::str Bold = "\N{ESCAPE}[1m";
inline constexpr best::str Red = "\N{ESCAPE}[31m";
}  // namespace

void test::init() {
  all_tests.push(this);
  name_ = symbol_name(this, where());
}

bool test::run_all(int argc, char** argv) {
  best::eprint("{}testing:", Bold);

  for (int i = 0; i < argc; ++i) {
    best::eprint(" {}", *str::from_nul(argv[i]));
  }
  best::eprintln();
  best::eprintln("executing {} test(s)\n", all_tests.size());

  best::vec<best::test*> successes;
  best::vec<best::test*> failures;
  for (auto* test : all_tests) {
    best::eprintln("{}[ TEST: {} ]{}", Bold, test->name(), Reset);
    if (!test->run()) {
      best::eprintln("{}{}[ FAIL: {} ]{}", Bold, Red, test->name(), Reset);
      failures.push(test);
    } else {
      best::eprintln("{}[ Ok: {} ]{}", Bold, test->name(), Reset);
      successes.push(test);
    }
  }

  best::eprintln();
  best::eprintln("{}[ RESULTS ]{}", Bold, Reset);
  if (!successes.is_empty()) {
    best::eprintln("{}passed {} test(s){}", Bold, successes.size(), Reset);
    for (auto* test : successes) best::eprintln(" * {}", test->name());
  }

  if (!failures.is_empty()) {
    best::eprintln("{}{}failed {} test(s){}", Bold, Red, successes.size(),
                   Reset);
    for (auto* test : failures)
      best::eprintln("{} * {}{}", Red, test->name(), Reset);
  }

  return failures.is_empty();
}
}  // namespace best

[[gnu::weak]] int main(int argc, char** argv) {
  return !::best::test::run_all(argc, argv);
}
