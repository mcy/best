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

#ifndef BEST_TEST_TEST_H_
#define BEST_TEST_TEST_H_

#include "best/cli/cli.h"
#include "best/log/location.h"
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
  test(auto body, best::location loc = best::here) : body_(body), loc_(loc) {
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
  template <best::formattable... Args>
  void fail(best::format_template<Args...> message = "", const Args&... args) {
    best::eprintln("failed at {:?}", message.where());
    if (!message.as_str().is_empty()) {
      best::eprint("=> ");
      best::eprintln(message, args...);
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
  template <best::formattable... Args>
  bool expect(bool cond, best::format_template<Args...> message = "",
              const Args&... args) {
    if (!cond) {
      best::eprintln("failed expect() at {:?}", message.where());
      if (!message.as_str().is_empty()) {
        best::eprint("=> ");
        best::eprintln(message, args...);
      }
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
  template <typename A, best::equatable<A> B = A, best::formattable... Args>
  bool expect_eq(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a == b, a, b, "expect_eq", "equal", message, args...);
  }
  template <typename A, best::equatable<A> B = A, best::formattable... Args>
  bool expect_ne(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a != b, a, b, "expect_ne", "unequal", message, args...);
  }
  template <typename A, best::comparable<A> B = A, best::formattable... Args>
  bool expect_lt(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a < b, a, b, "expect_lt", "`<`", message, args...);
  }
  template <typename A, best::comparable<A> B = A, best::formattable... Args>
  bool expect_le(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a <= b, a, b, "expect_le", "`<=`", message, args...);
  }
  template <typename A, best::comparable<A> B = A, best::formattable... Args>
  bool expect_gt(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a > b, a, b, "expect_gt", "`>`", message, args...);
  }
  template <typename A, best::comparable<A> B = A, best::formattable... Args>
  bool expect_ge(const A& a, const B& b,
                 best::format_template<Args...> message = "",
                 const Args&... args) {
    return expect_cmp(a >= b, a, b, "expect_ge", "`>=`", message, args...);
  }

  /// # `test::flags`
  ///
  /// Flags necessary for calling `run_all()`. These are the flags passed to a
  /// test binary.
  struct flags;

  /// # `test::run_all()`.
  ///
  /// Runs all registered unit tests.
  ///
  /// Linking in the test library will automatically cause this to be called
  /// by the built-in `best::app`. If you define your own main(), you must call
  /// this manually. This function expects to be called from a `best::app`.
  ///
  /// Returns whether all tests passed.
  static bool run_all(const flags&);

 private:
  void init();

  bool expect_cmp(bool cond, auto& a, auto& b, best::str func, best::str cmp,
                  const auto& message, const auto&... args) {
    if (!cond) {
      best::eprintln(
          "failed {}() at {:?}\nexpected these values to be {}:\n  {:?}\n  "
          "{:?}",
          func, message.where(), cmp, best::make_formattable(a),
          best::make_formattable(b));
      failed_ = true;
      if (!message.as_str().is_empty()) {
        best::eprint("=> ");
        best::eprintln(message, args...);
      }
    }
    return cond;
  }

  void (*body_)(test&);
  best::location loc_;

  best::str name_;
  bool failed_ = false;
};

struct test::flags final {
  best::vec<best::strbuf> skip;
  best::vec<best::strbuf> filters;

  constexpr friend auto BestReflect(auto& m, flags*) {
    return m.infer()
        .with(best::cli::app{.about = "a best unit test binary"})
        .with(&flags::skip,
              best::cli::flag{
                  .arg = "FILTER",
                  .help = "Skip tests whose names contain FILTER",
              })
        .with(&flags::filters,
              best::cli::positional{
                  .name = "FILTERS",
                  .help = "Include only tests whose names contain FILTER",
              });
  }
};
}  // namespace best

#endif  // BEST_TEST_TEST_H_
