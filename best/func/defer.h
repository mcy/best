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

#ifndef BEST_FUNC_DEFER_H_
#define BEST_FUNC_DEFER_H_

#include "best/base/tags.h"
#include "best/func/call.h"

//! Deferred execution.
//!
//! `best::defer` is a mechanism for deferring execution of some closure until
//! scope exit.

namespace best {
/// # `best::defer`
///
/// Defers execution of a closure until scope exit.
///
/// ```
/// best::defer d_ = []{ ... };
/// ```
template <typename Guard, typename Cb>
class [[nodiscard(
  "a best::defer must be assigned to variable to have any effect")]] defer
  final {
 public:
  BEST_CTAD_GUARD_("best::defer", Guard);

  /// # `defer::defer()`
  ///
  /// Constructs a new tap by wrapping a callback.
  constexpr defer(Cb&& cb) : cb_(BEST_FWD(cb)) {}
  constexpr defer(best::bind_t, Cb&& cb) : cb_(BEST_FWD(cb)) {}

  /// # `defer::defer()`
  ///
  /// Runs the defer at scope exit.
  constexpr ~defer() { run(); }

  /// # `defer::cancel()`
  ///
  /// Inhibits execution of the defer.
  constexpr void cancel() { cancelled_ = true; }

  /// # `defer::run()`
  ///
  /// Forcibly runs the defer. Further calls to run() will have no effect.
  constexpr void run() {
    if (!cancelled_) {
      best::call(cb_);
      cancel();
    }
  }

 private:
  Cb cb_;
  bool cancelled_ = false;
};

template <typename Cb>
defer(Cb&&) -> defer<tags_internal_do_not_use::ctad_guard, best::as_auto<Cb>>;
template <typename Cb>
defer(best::bind_t, Cb&&) -> defer<tags_internal_do_not_use::ctad_guard, Cb&&>;
}  // namespace best

#endif  // BEST_FUNC_DEFER_H_
