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

#ifndef BEST_FUNC_TAP_H_
#define BEST_FUNC_TAP_H_

#include "best/base/tags.h"
#include "best/func/call.h"
#include "best/meta/taxonomy.h"

//! Taps, `best`'s "functional pipeline" syntax.
//!
//! A tap is a type that wraps a function to make it callable as a postfix
//! operation with `->*`.
//!
//! ```
//! best::tap foo = [](int x) { return x + x; };
//! int z = 5->*foo;
//! ```
//!
//! Common taps are good for returning out of functions or making into
//! constants, so you might write
//!
//! ```
//! inline constexpr best::tap MyTap = [](auto x) { ... };
//!
//! something->*MyTap->*MyOtherTap(xyz);
//! ```
//!
//! ## Language Bugs
//!
//! Because `->*` has lower precedence than `.` and `->`, there are some
//! limitations. In particular, you can't do `foo->*bar().baz()`, where `bar()`
//! is a function that returns a tap. You must either parenthesize the tap
//! expression, `(foo->*bar()).baz()`; or save it to a temporary. you may,
//! however, chain tap expressions as you might expect.
//!
//! `->*` also binds stronger than `operator()` and `operator[]`. However, the
//! vanilla tap, `best::tap`, overloads `operator()` and `operator[]` to give
//! the illusion otherwise: `foo->*my_tap(bar)` will be parsed as
//! `operator->*(foo, my_tap.operator()(bar))`: using `()` or `[]` on a tap will
//! "bind" those arguments, returning a new tap that "does what you expect",
//! such that `foo->*my_tap(bar)` is almost `(foo->*my_tap)(bar)`.
//!
//! Another quirk is that the following code will not compile:
//!
//! ```
//! MyType{}->*best::tap([](auto&) { ... });
//! ```
//!
//! This is because this gets rewritten as an ordinary function call, so unlike
//! a call through `.` or `->`, it will not convert the LHS into a non-const
//! lvalue. In general, you want your taps to take `auto&&`.

namespace best {
/// # `best::tap`
///
/// The basic tap: takes a function and `->*` calls it on the tapped value.
///
/// ```
/// best::tap foo = [](int x) { return x + x; };
/// int z = 5->*foo;
/// ```
template <typename Guard, typename Cb>
class [[nodiscard(
  "a best::tap must be called using ->* to have any effect")]] tap final {
 public:
  BEST_CTAD_GUARD_("best::tap", Guard);

  /// # `tap::tap()`
  ///
  /// Constructs a new tap by wrapping a callback.
  constexpr tap(Cb&& cb) : cb_(BEST_FWD(cb)) {}
  constexpr tap(best::bind_t, Cb&& cb) : cb_(BEST_FWD(cb)) {}

  /// # `tap::callback()`
  ///
  /// Gets a reference to the wrapped callback.
  constexpr const Cb& callback() const& { return cb_; }
  constexpr Cb& callback() & { return cb_; }
  constexpr const Cb&& callback() const&& { return BEST_MOVE(cb_); }
  constexpr Cb&& callback() && { return BEST_MOVE(cb_); }

  /// # `tap()`, `tap[]`
  ///
  /// Binds arguments for this tap. This makes it possible to use this tap as
  /// a function call: `foo->*my_tap(bar)`, assuming the tap returns a callable.
  /// (Respectively for `foo->*my_tap[bar]`).
  ///
  /// This works around a quirk of C++'s operator precedence order.
  constexpr auto operator()(auto&&...) const&;
  constexpr auto operator()(auto&&...) &;
  constexpr auto operator()(auto&&...) const&&;
  constexpr auto operator()(auto&&...) &&;
  constexpr auto operator[](auto&&) const&;
  constexpr auto operator[](auto&&) &;
  constexpr auto operator[](auto&&) const&&;
  constexpr auto operator[](auto&&) &&;

  /// # `arg->*tap`
  ///
  /// "Taps" a value with this tap. This calls the LHS of `->*` with the stored
  /// callback.
  constexpr friend decltype(auto) operator->*(auto&& arg, const tap& tap) {
    return best::call(tap.callback(), BEST_FWD(arg));
  }
  constexpr friend decltype(auto) operator->*(auto&& arg, tap& tap) {
    return best::call(tap.callback(), BEST_FWD(arg));
  }
  constexpr friend decltype(auto) operator->*(auto&& arg, const tap&& tap) {
    return best::call(BEST_MOVE(tap).callback(), BEST_FWD(arg));
  }
  constexpr friend decltype(auto) operator->*(auto&& arg, tap&& tap) {
    return best::call(BEST_MOVE(tap).callback(), BEST_FWD(arg));
  }

 private:
  Cb cb_;
};

/// # `best::inspect()`
///
/// Creates an "inspecting tap", i.e., a tap that discards the return value of
/// the passed closure and returns its argument.
///
/// ```
/// foo(my_vec->*best::inspect([](auto& v) { v.push(42); }));
/// ```
constexpr auto inspect(auto&&);
constexpr auto inspect(best::bind_t, auto&&);
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator()(auto&&... args) const& {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return best::call((BEST_FWD(arg))->**this, BEST_FWD(args)...);
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator()(auto&&... args) & {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return best::call((BEST_FWD(arg))->**this, BEST_FWD(args)...);
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator()(auto&&... args) const&& {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return best::call((BEST_FWD(arg))->*BEST_MOVE(*this), BEST_FWD(args)...);
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator()(auto&&... args) && {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return best::call((BEST_FWD(arg))->*BEST_MOVE(*this), BEST_FWD(args)...);
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator[](auto&& arg) const& {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return ((BEST_FWD(arg))->**this)[BEST_FWD(arg)];
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator[](auto&& arg) & {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return ((BEST_FWD(arg))->**this)[BEST_FWD(arg)];
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator[](auto&& arg) const&& {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return ((BEST_FWD(arg))->*BEST_MOVE(*this))[BEST_FWD(arg)];
  });
}
template <typename Guard, typename Cb>
constexpr auto tap<Guard, Cb>::operator[](auto&& arg) && {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    return ((BEST_FWD(arg))->*BEST_MOVE(*this))[BEST_FWD(arg)];
  });
}

constexpr auto inspect(auto&& cb) {
  return best::tap([cb = BEST_FWD(cb)](auto&& arg) -> decltype(auto) {
    (void)best::call(cb, BEST_FWD(arg));
    BEST_FWD(arg);
  });
}
constexpr auto inspect(best::bind_t, auto&& cb) {
  return best::tap([&](auto&& arg) -> decltype(auto) {
    (void)best::call(BEST_FWD(cb), BEST_FWD(arg));
    BEST_FWD(arg);
  });
}
}  // namespace best

#endif  // BEST_FUNC_TAP_H_
