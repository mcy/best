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

#ifndef BEST_BASE_HINT_H_
#define BEST_BASE_HINT_H_

#include <type_traits>

#include "best/base/port.h"

//! Optimization hints.
//!
//! This header provides functions and macros for instructing the compiler
//! on how to optimize code.

namespace best {
/// # `BEST_RELOCATABLE`
///
/// Marks a type as trivially relocatable.
///
/// This ensures that the type can be passed "in registers" when possible.
/// However, this is only valid when the type is semantically "trivially
/// relocatable", i.e., if moving from a value, and then destroying that value,
/// is a no-op, and the move operation itself is otherwise trivial.
#define BEST_RELOCATABLE [[clang::trivial_abi]]

/// # `BEST_INLINE_ALWAYS`, `BEST_INLINE_NEVER`
///
/// Marks a function with strong inlining hints.
///
/// When always inlined, the compiler will try its best to inline the function
/// at all optimization levels. When never inlined, the compiler will try to
/// avoid inlining it. The compiler is free to disregard either.
#define BEST_INLINE_ALWAYS [[gnu::always_inline]] inline
#define BEST_INLINE_NEVER [[gnu::noinline]]

/// # `BEST_INLINE_SYNTHETIC`
///
/// Like `BEST_INLINE_ALWAYS`, but the marked function will not appear in stack
/// traces.
#define BEST_INLINE_SYNTHETIC [[gnu::always_inline]] [[gnu::artificial]] inline

/// # `best::assume()`
///
/// Informs the compiler that something can be assumed to be true.
/// If `truth` is not true at runtime, undefined behavior.
BEST_INLINE_ALWAYS constexpr void assume(bool truth) {
#if BEST_HAS_BUILTIN(__builtin_assume)
  __builtin_assume(truth);
#else
  (void)truth;
#endif
}

/// # `best::unreachable()`
///
/// Immediately triggers undefined behavior.
///
/// Informs the compiler that this code will not be executed.
[[noreturn]] inline void unreachable() { __builtin_unreachable(); }

/// # `best::likely()`
///
/// Marks a value as likely to be true. This controls which side of a condition
/// the compiler will treat as "hot".
[[nodiscard]] BEST_INLINE_SYNTHETIC constexpr bool likely(bool truthy) {
#if BEST_HAS_BUILTIN(__builtin_expect)
  return __builtin_expect(truthy, true);
#else
  return truthy;
#endif
}

/// # `best::unlikely()`
///
/// Marks a value as likely to be false. This controls which side of a condition
/// the compiler will treat as "hot".
[[nodiscard]] BEST_INLINE_SYNTHETIC constexpr bool unlikely(bool falsey) {
#if BEST_HAS_BUILTIN(__builtin_expect)
  return __builtin_expect(falsey, false);
#else
  return falsey;
#endif
}

/// # `best::black_box()`
///
/// Hides a value from the compiler's optimizer.
[[nodiscard]] BEST_INLINE_SYNTHETIC constexpr auto&& black_box(auto&& value) {
  if (!std::is_constant_evaluated()) {
    asm volatile("" ::"m,r"(value));
  }
  return decltype(value)(value);
}
}  // namespace best

#endif  // BEST_BASE_HINT_H_
