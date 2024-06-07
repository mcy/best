#include "best/test/test.h"

#include <dlfcn.h>

#include <cstdlib>
#include <iostream>
#include <vector>

namespace best {
namespace {
std::vector<best::test*> all_tests;

best::str symbol_name(const void* ptr, best::location loc) {
  ::Dl_info di;
  if (::dladdr(ptr, &di)) {
    if (di.dli_sname == nullptr) {
      std::cerr
          << "could not parse symbol name for test at " << loc
          << "; you may need to pass -rdynamic as part of your link options\n";
      std::exit(128);
    }
    return best::str(di.dli_sname);
  } else {
    std::cerr << "could not parse symbol name for test at " << loc
              << "; it might not be a global variable?\n";
    std::exit(128);
  }
}

inline constexpr best::str Reset = "\N{ESCAPE}[0m";
inline constexpr best::str Bold = "\N{ESCAPE}[1m";
inline constexpr best::str Red = "\N{ESCAPE}[31m";
}  // namespace

void test::init() {
  all_tests.push_back(this);
  name_ = symbol_name(this, where());
}

bool test::run_all(int argc, char** argv) {
  std::cout << Bold << "testing:";
  for (int i = 0; i < argc; ++i) {
    std::cout << " " << argv[i];
  }
  std::cout << "\n";
  std::cout << "executing " << all_tests.size() << " test(s)\n\n";

  std::vector<best::test*> successes;
  std::vector<best::test*> failures;
  for (auto* test : all_tests) {
    std::cout << Bold << "[ TEST: " << test->name() << " ]\n" << Reset;
    if (!test->run()) {
      std::cout << Bold << Red << "[ FAIL: " << test->name() << " ]\n" << Reset;
      failures.push_back(test);
    } else {
      std::cout << Bold << "[ OK: " << test->name() << " ]\n" << Reset;
      successes.push_back(test);
    }
  }

  std::cout << "\n";
  std::cout << Bold << "[ RESULTS ]\n" << Reset;
  if (!successes.empty()) {
    std::cout << Bold << "passed " << successes.size() << " test(s) \n"
              << Reset;
    for (auto* test : successes) {
      std::cout << " * " << test->name() << "\n" << Reset;
    }
  }

  if (!failures.empty()) {
    std::cout << Bold << Red << "failed " << failures.size() << " test(s) \n"
              << Reset;
    for (auto* test : failures) {
      std::cout << Red << " * " << test->name() << "\n" << Reset;
    }
  }

  return failures.empty();
}
}  // namespace best

[[gnu::weak]] int main(int argc, char** argv) {
  return !::best::test::run_all(argc, argv);
}