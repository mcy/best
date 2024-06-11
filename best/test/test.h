#ifndef BEST_TEST_TEST_H_
#define BEST_TEST_TEST_H_

#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>

#include "best/log/location.h"
#include "best/meta/ops.h"
#include "best/strings/str.h"

//! The best unit testing library.

namespace best {
/// A unit test.
///
/// To create a unit test, create a namespace with an appropriate name, and
/// define a variable thusly:
///
///   best::test MyTest = [](auto& t) {
///     // Test code.
///   };
///
/// Within the body of the lambda, `t` (which is a reference to the best::test
/// itself) can be used to manipulate test state (such as to make assertions).
///
/// Currently, tests cannot be in anonymous namespaces, and the test binary must
/// be built with -rdynamic. This is a temporary limitation.
class test final {
 public:
  /// Creates and registers a new unit test.
  ///
  /// This should ONLY be used to create global variables!
  template <typename Body>
  test(Body body, best::location loc = best::here)
    requires best::callable<Body, void(test&)>
      : body_(std::move(body)), loc_(loc) {
    init();
  }

  /// Returns the name of this test.
  best::str name() { return name_; }

  /// Returns the location where this test was defined.
  best::location where() { return loc_; }

  /// Executes this test. Returns whether this test passed.
  bool run() {
    failed_ = false;
    best::call(body_, *this);
    return !failed_;
  }

  /// Marks this test as failed.
  void fail(best::str message = "", best::location loc = best::here) {
    std::cerr << "failed at " << loc << "\n";
    if (!message.is_empty()) {
      std::cerr << "  " << message << "\n";
    }
    failed_ = true;
  }

  /// Performs an assertion on `cond`.
  ///
  /// If false, marks this test as failed and prints the given message.
  /// Returns `cond`, to allow for the pattern
  ///
  /// ```
  /// if (!t.expect(...)) { return; }
  /// ```
  bool expect(bool cond, best::str message = "",
              best::location loc = best::here) {
    if (!cond) {
      std::cerr << "failed expect() at " << loc << "\n";
      if (!message.is_empty()) {
        std::cerr << "  " << message << "\n";
      }
      failed_ = true;
    }
    return cond;
  }

  /// Performs a comparison assertion.
  ///
  /// If false, marks this test as failed and prints the given message.
  /// Returns `cond`, to allow for the pattern
  ///
  /// ```
  /// if (!t.expect_eq(...)) { return; }
  /// ```
  template <typename A, typename B = A>
  bool expect_eq(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a == b, a, b, "expect_eq", message, loc);
  }
  template <typename A, typename B = A>
  bool expect_ne(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a != b, a, b, "expect_ne", message, loc);
  }
  template <typename A, typename B = A>
  bool expect_lt(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a < b, a, b, "expect_lt", message, loc);
  }
  template <typename A, typename B = A>
  bool expect_le(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a <= b, a, b, "expect_le", message, loc);
  }
  template <typename A, typename B = A>
  bool expect_gt(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a > b, a, b, "expect_gt", message, loc);
  }
  template <typename A, typename B = A>
  bool expect_ge(const A& a, const B& b, best::str message = "",
                 best::location loc = best::here) {
    return expect_cmp(a >= b, a, b, "expect_ge", message, loc);
  }

  /// Runs all registered unit tests.
  ///
  /// Linking in the test library will automatically cause this to be called
  /// by main(). If you define your own main(), you must call this manually.
  ///
  /// Returns whether all tests passed.
  static bool run_all(int argc, char** argv);

 private:
  void init();

  bool expect_cmp(bool cond, auto& a, auto& b, best::str func,
                  best::str message, best::location loc) {
    if (!cond) {
      std::cerr << "failed " << func << "() at " << loc << "\n"
                << "expected these values to be equal:\n"
                << "  " << print_any{a} << "\n"
                << "  " << print_any{b} << "\n";
      if (!message.is_empty()) {
        std::cerr << "  " << message << "\n";
      }
      failed_ = true;
    }
    return cond;
  }

  template <typename T>
  struct print_any {
    const T& r;
  };
  template <typename T>
  print_any(T) -> print_any<T>;
  template <typename Os, typename T>
  friend Os& operator<<(Os& os, print_any<T> p) {
    if constexpr (best::has_op<best::op::Shl, Os&, const T&>) {
      return os << p.r;
    }

    os << "unprintable value: ";
    const char* ptr = reinterpret_cast<const char*>(std::addressof(p.r));
    for (size_t i = 0; i < sizeof(T); ++i) {
      os << std::hex << std::setw(2) << std::setfill('0') << ptr[i];
    }
    return os;
  }

  std::function<void(test&)> body_;
  best::location loc_;

  best::str name_;
  bool failed_ = false;
};
}  // namespace best

#endif  // BEST_TEST_TEST_H_