#ifndef BEST_TEST_TEST_H_
#define BEST_TEST_TEST_H_

#include <functional>
#include <iostream>

#include "best/log/location.h"
#include "best/meta/ops.h"
#include "best/text/format.h"
#include "best/text/str.h"

//! The best unit testing library.

namespace best {
/// # `best::test`
///
/// A unit test.
///
/// To create a unit test, create a namespace with an appropriate name, and
/// define a variable thusly:
///
/// ```
/// best::test MyTest = [](auto& t) {
///   // Test code.
/// };
/// ```
///
/// Within the body of the lambda, `t` (which is a reference to the `best::test`
/// itself) can be used to manipulate test state (such as to make assertions).
///
/// Currently, tests cannot be in anonymous namespaces, and the test binary must
/// be built with `-rdynamic`. This is a temporary limitation.
class test final {
 public:
  /// # `test::test(body)`
  ///
  /// Creates and registers a new unit test.
  ///
  /// This should ONLY be used to create global variables!
  template <typename Body>
  test(Body body, best::location loc = best::here)
    requires best::callable<Body, void(test&)>
      : body_(std::move(body)), loc_(loc) {
    init();
  }

  /// # `test::name()`
  ///
  /// Returns the name of this test.
  best::str name() { return name_; }

  /// # `test::where()`
  ///
  /// Returns the location where this test was defined.
  best::location where() { return loc_; }

  /// # `test::run()`
  ///
  /// Executes this test. Returns whether this test passed.
  bool run() {
    failed_ = false;
    best::call(body_, *this);
    return !failed_;
  }

  /// # `test::fail()`
  ///
  /// Marks this test as failed.
  void fail(best::str message = "", best::location loc = best::here) {
    std::cerr << "failed at " << loc << "\n";
    if (!message.is_empty()) {
      std::cerr << "  " << message << "\n";
    }
    failed_ = true;
  }

  /// # `test::expect()`
  ///
  /// Performs an assertion on `cond`.
  ///
  /// If false, marks this test as failed and prints the given message.
  /// Returns `cond`, to allow for the pattern
  ///
  /// ```
  /// if (!t.expect(...)) { return; }
  /// ```
  bool expect(best::track_location<bool> cond) {
    if (!*cond) {
      best::eprintln("failed expect() at {:?}", cond.location());
      failed_ = true;
    }
    return cond;
  }
  template <best::formattable... Args>
  bool expect(best::track_location<bool> cond,
              best::format_template<Args...> message, const Args&... args) {
    if (!*cond) {
      best::eprintln("failed expect() at {:?}", cond.location());
      best::eprint("=> ");
      best::eprintln(message, args...);
      failed_ = true;
    }
    return cond;
  }

  /// # `test::expect_eq()` et. al.
  ///
  /// Performs a comparison assertion.
  ///
  /// If false, marks this test as failed and prints the given message.
  /// Returns `cond`, to allow for the pattern
  ///
  /// ```
  /// if (!t.expect_eq(...)) { return; }
  /// ```
  template <typename A, best::equatable<A> B = A>
  bool expect_eq(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a == b, a, b, "expect_eq", "equal", loc);
  }
  template <typename A, typename B = A>
  bool expect_ne(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a != b, a, b, "expect_ne", "unequal", loc);
  }
  template <typename A, typename B = A>
  bool expect_lt(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a < b, a, b, "expect_lt", "`<`", loc);
  }
  template <typename A, typename B = A>
  bool expect_le(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a <= b, a, b, "expect_le", "`<=`", loc);
  }
  template <typename A, typename B = A>
  bool expect_gt(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a > b, a, b, "expect_gt", "`>`", loc);
  }
  template <typename A, typename B = A>
  bool expect_ge(const A& a, const B& b, best::location loc = best::here) {
    return expect_cmp(a >= b, a, b, "expect_ge", "`>=`", loc);
  }

  /// # `test::run_all()`.
  ///
  /// Runs all registered unit tests.
  ///
  /// Linking in the test library will automatically cause this to be called
  /// by main(). If you define your own main(), you must call this manually.
  ///
  /// Returns whether all tests passed.
  static bool run_all(int argc, char** argv);

 private:
  void init();

  bool expect_cmp(bool cond, auto& a, auto& b, best::str func, best::str cmp,
                  best::location loc) {
    if (!cond) {
      best::eprintln(
          "failed {}() at {:?}\nexpected these values to be {}:\n=> {:?}\n=> "
          "{:?}",
          func, loc, cmp, best::make_formattable(a), best::make_formattable(b));
      failed_ = true;
    }
    return cond;
  }

  std::function<void(test&)> body_;
  best::location loc_;

  best::str name_;
  bool failed_ = false;
};
}  // namespace best

#endif  // BEST_TEST_TEST_H_