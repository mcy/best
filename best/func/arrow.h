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

#ifndef BEST_FUNC_ARROW_H_
#define BEST_FUNC_ARROW_H_

#include "best/meta/traits/ptrs.h"

//! Arrows, helpers for implementing exotic `operator->`s.
//!
//! When C++ sees `a->b`, and `a` is not a raw pointer, it replaces it with
//! `a.operator->()->b`. It does so recursively until it obtains a raw pointer.
//!
//! Suppose we wanted to return some kind of non-pointer view from `operator->`;
//! for example, we want to inject all of the functions of some type `T` into
//! our type `U`, findable by `->`. But it doesn't contain a `T` that we can
//! return a pointer to!
//!
//! An obviously incorrect implementation of `operator->` in this case would be
//!
//! ```
//! T* operator->() {
//!   T view = ...;
//!   return &view;
//! }
//! ```
//!
//! When the `operator->` returns, it returns a pointer to an object whose
//! lifetime is over. Oops! However, we can instead return a `best::arrow`:
//!
//! ```
//! best::arrow<T> operator->() {
//!   return T{...};
//! }
//! ```
//!
//! Then, when C++ goes to desugar `a->b`, it will expand to
//! `a.operator->()->b`, calling `U`'s `operator->`; then it will expand again
//! to `a.operator->().operator->()->b`, calling `best::arrow::operator->()`.
//!
//! Because we returned an actual `T`, it is not destroyed until the end of the
//! full expression, so the pointer that the final `->b` offsets will be valid.

namespace best {
/// # `best::deref_arrow()`
///
/// Runs an `operator->` deref sequence to completion, simulating the result
/// of doing `x->y`, without the final "real" `->y` part.
template <typename A>
auto* deref_arrow(A&& ptr) requires (best::is_raw_ptr<best::as_auto<A>> ||
                                     requires {
                                       { deref_arrow(ptr.operator->()) };
                                     })
{
  if constexpr (best::is_raw_ptr<best::as_auto<A>>) {
    return ptr;
  } else {
    return deref_arrow(BEST_MOVE(ptr).operator->());
  }
}

/// # `best::can_arrow<A>`
///
/// Returns whether `A` provides a valid `operator->`.
template <typename A>
concept can_arrow = requires(A a) {
  { deref_arrow(a) };
};

/// # `best::arrow<T>`
///
/// A wrapper over a `T` that implements a `operator->` that returns a pointer
/// to that value. If `T` itself provides `operator->`, it calls that instead.
template <typename T>
class arrow final {
 public:
  constexpr arrow(auto&& arg) : value_(BEST_FWD(arg)) {}
  constexpr auto operator->() && {
    if constexpr (requires {
                    { value_.operator->() };
                  }) {
      return value_;
    } else {
      return best::addr(value_);
    }
  }

  constexpr decltype(auto) operator()(auto&&... args)
    requires requires(T value) {
      { value(BEST_FWD(args)...) };
    }
  {
    return value_(BEST_FWD(args)...);
  }

  arrow() = delete;
  constexpr arrow(const arrow&) = default;
  constexpr arrow& operator=(const arrow&) = default;
  constexpr arrow(arrow&&) = default;
  constexpr arrow& operator=(arrow&&) = default;

 private:
  T value_;
};

template <typename T>
arrow(T&&) -> arrow<best::as_auto<T>>;
}  // namespace best

#endif  // BEST_FUNC_ARROW_H_
