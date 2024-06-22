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

#ifndef BEST_BASE_UNSAFE_H_
#define BEST_BASE_UNSAFE_H_

//! Unsafe operation tracking.
//!
//! This header provides `best::unsafe`, a tag type for specifying that an
//! operation has non-trivial preconditions. A function whose first argument is
//! `best::unsafe` is an unsafe function.

namespace best {
/// # `best::unsafe`
///
/// The unsafe tag type. This value is used for tagging functions that have
/// non-trivial preconditions. This is not exactly the same as `unsafe` in Rust;
/// many operations in `best` and C++ at large cannot have such a check.
/// Instead, it primarily exists for providing unsafe overloads of functions
/// that skip safety checks.
///
/// ```
/// int evil(best::unsafe, int);
///
/// int x = evil(best::unsafe("I checked the preconditions"), 42);
/// ```
struct unsafe final {
  /// # `unsafe::unsafe()`
  ///
  /// Constructs a new `unsafe`; the user must provide justification for doing
  /// so, usually in the form of a string literal.
  constexpr unsafe(auto&& why) {}

 private:
  unsafe() = default;
};
}  // namespace best

#endif  // BEST_BASE_UNSAFE_H_
