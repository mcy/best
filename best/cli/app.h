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

#include "best/container/result.h"
#include "best/container/span.h"
#include "best/meta/init.h"
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
/// # `best::argv`
///
/// Whether a type is an "argv", something that can be parsed from the
/// command-like arguments passed to the program by the operating system.
///
/// To define a new argument type, implement the `BestFromArgv()` FTADLE:
///
/// ```
/// friend best::result<void, E> BestFromArgv(auto& argv, MyArgs& args) {
///   // Impl here.
/// }
/// ```
///`argv` will  be a `best::iter` that yields `best::pretext<wtf8>`. `args` is
/// a freshly constructed `MyArgs`, which should be filled in by the function.
///
/// If parsing fails, the returned `best::result` will be unwrapped and shown
/// to the user. NOTE: This is a temporary limitation, eventually there will be
/// a cleaner interface once the flags interface is built.
template <typename T>
concept is_argv = requires(best::ftadle& ftadle, T& argv) {
  requires best::constructible<best::as_auto<T>>;
  { BestFromArgv(ftadle, argv) } -> best::is_result;
};

/// # `best::app`
///
/// A CLI application. This is `best`'s answer to C++'s `main()` function.
/// Instead of defining a function named `main()`, you define an app at
/// namespace scope in your main file.
///
/// ```
/// best::app MyApp = +[](best::vec<best::strbuf> args) {
///   // Your code here!
/// };
/// ```
///
/// So, what can go in `args`?
class app final {
 public:
  /// # `app::app()`
  ///
  /// An app can be constructed from three possible function signatures. The
  /// function must either take no arguments, or an argument that satisfies
  /// `best::is_argv`, and may return one of three types.
  ///
  /// - `void`, in which case returning will exit with code 0.
  /// - `int`, in which case returning will exit with the returned code.
  /// - A `best::result,` in which case the result is unwrapped on return.
  app(void (*main)(), best::location = best::here);
  app(int (*main)(), best::location = best::here);
  template <typename T, typename E>
  app(best::result<T, E> (*main)(), best::location = best::here);
  template <best::is_argv Args>
  app(void (*main)(Args), best::location = best::here);
  template <best::is_argv Args>
  app(int (*main)(Args), best::location = best::here);
  template <best::is_argv Args, typename T, typename E>
  app(best::result<T, E> (*main)(Args), best::location = best::here);
  app(auto lambda, best::location loc = best::here) : app(+lambda, loc) {}

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

  void install();
  template <best::is_argv Args>
  static Args parse();

  uintptr_t main_;
  void (*shim_)(uintptr_t);
  best::location loc_;
};

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <best::is_argv Args>
Args app::parse() {
  Args args;
  *BestFromArgv(argv(), args);
  return args;
}

inline app::app(void (*main)(), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<void (*)()>(vp);
        main();
        std::exit(0);
      }},
      loc_(loc) {
  install();
}

inline app::app(int (*main)(), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<int (*)()>(vp);
        std::exit(main());
      }},
      loc_(loc) {
  install();
}

template <typename T, typename E>
app::app(best::result<T, E> (*main)(), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<best::result<T, E> (*)()>(vp);
        *main();
        std::exit(0);
      }},
      loc_(loc) {
  install();
}

template <best::is_argv Args>
app::app(void (*main)(Args), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<void (*)(Args)>(vp);
        main(parse<Args>());
        std::exit(0);
      }},
      loc_(loc) {
  install();
}

template <best::is_argv Args>
app::app(int (*main)(Args), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<int (*)(Args)>(vp);
        std::exit(main(parse<Args>()));
      }},
      loc_(loc) {
  install();
}

template <best::is_argv Args, typename T, typename E>
app::app(best::result<T, E> (*main)(Args), best::location loc)
    : main_(reinterpret_cast<uintptr_t>(main)),
      shim_{+[](uintptr_t vp) {
        auto main = reinterpret_cast<best::result<T, E> (*)(Args)>(vp);
        *main(parse<Args>());
        std::exit(0);
      }},
      loc_(loc) {
  install();
}
}  // namespace best

#endif  // BEST_CLI_APP_H_
