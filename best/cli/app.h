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

#ifndef BEST_CLI_APP_H_
#define BEST_CLI_APP_H_

#include <cstdlib>

#include "best/cli/parser.h"
#include "best/container/result.h"
#include "best/container/span.h"
#include "best/log/location.h"
#include "best/text/format.h"
#include "best/text/str.h"

//! Command-line applications.
//!
//! This header provides a replacement for `main()` that automatically does many
//! of the things a modern language does before invoking the user's main. `best`
//! needs to perform one-time initialization, and since we need to replace the
//! `main` function, we use it as an opportunity to provide automatic CLI
//! processing, too.

namespace best {
/// # `best::app`
///
/// A CLI application. This is `best`'s answer to C++'s `main()` function.
/// Instead of defining a function named `main()`, you define an app at
/// namespace scope in your main file.
///
/// ```
/// best::app MyApp = [](MyFlags& args) {
///   // Your code here!
/// };
/// ```
///
/// So, what can go in `args`? It's a flags struct! See `cli.h` for more
/// information on how to define one. Alternatively, you can simply take zero
/// arguments, and interpret `best::app::argv()` yourself.
///
/// The function must return either `void`, `int`, or a result whose error type
/// is printable.
class app final {
 public:
  /// # `app::app()`
  ///
  /// An app can be constructed from three possible function signatures. The
  /// function must either take no arguments, or an argument that can be parsed
  /// with `best::parse_flags()`, and may return one of three types.
  ///
  /// - `void`, in which case returning will exit with code 0.
  /// - `int`, in which case returning will exit with the returned code.
  /// - A `best::result,` in which case the result is unwrapped on return.
  app(void (*main)(), best::location = best::here);
  app(int (*main)(), best::location = best::here);
  template <typename T, typename E>
  app(best::result<T, E> (*main)(), best::location = best::here);
  template <typename Args>
  app(void (*main)(Args&), best::location = best::here);
  template <typename Args>
  app(int (*main)(Args&), best::location = best::here);
  template <typename Args, typename T, typename E>
  app(best::result<T, E> (*main)(Args&), best::location = best::here);
  app(auto lambda, best::location loc = best::here) : app(+lambda, loc) {}

  /// # `app::exe()`
  ///
  /// Returns the executable name passed to the program on startup by the
  /// operating system.
  static best::pretext<wtf8> exe();

  /// # `app::argv()`
  ///
  /// Returns the arguments passed to the program on startup by operating
  /// system.
  static best::span<const best::pretext<wtf8>> argv();

  /// # `app::start()`
  ///
  /// Starts the app. This function should be called in `main()`. By default,
  /// linking in `app.cc` will generate a `main()` function that does this for
  /// you.
  ///
  /// This function is not re-entrant, and will crash if called more than once
  /// in the lifetime of the program.
  [[noreturn]] static void start(int argc, char** argv);

 private:
  using mark_format_header_as_used = best::formatter;

  app(auto main, best::location loc, int (*shim)(uintptr_t))
      : main_(reinterpret_cast<uintptr_t>(main)), shim_(shim), loc_(loc) {
    install();
  }

  void install();

  uintptr_t main_;
  int (*shim_)(uintptr_t);
  best::location loc_;
};

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
inline app::app(void (*main)(), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto main = reinterpret_cast<void (*)()>(vp);
        main();
        return 0;
      }) {}

inline app::app(int (*main)(), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto main = reinterpret_cast<int (*)()>(vp);
        return main();
      }) {}

template <typename T, typename E>
app::app(best::result<T, E> (*main)(), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto main = reinterpret_cast<best::result<T, E> (*)()>(vp);
        (void)*main();
        return 0;
      }) {}

template <typename Args>
app::app(void (*main)(Args&), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto args = best::parse_flags<Args>(exe(), argv());
        if (!args) args.err()->print_and_exit();

        auto main = reinterpret_cast<void (*)(Args&)>(vp);
        main(*args);
        return 0;
      }) {}

template <typename Args>
app::app(int (*main)(Args&), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto args = best::parse_flags<Args>(exe(), argv());
        if (!args) args.err()->print_and_exit();

        auto main = reinterpret_cast<int (*)(Args&)>(vp);
        return main(*args);
      }) {}

template <typename Args, typename T, typename E>
app::app(best::result<T, E> (*main)(Args&), best::location loc)
    : app(main, loc, [](uintptr_t vp) {
        auto args = best::parse_flags<Args>(exe(), argv());
        if (!args) args.err()->print_and_exit();

        auto main = reinterpret_cast<best::result<T, E> (*)(Args&)>(vp);
        (void)*main(*args);
        return 0;
      }) {}
}  // namespace best

#endif  // BEST_CLI_APP_H_
