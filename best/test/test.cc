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
    best::eprint(" {}", argv[i]);
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