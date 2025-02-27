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

#ifndef BEST_BASE_PORT_H_
#define BEST_BASE_PORT_H_

#include "best/base/macro.h"

//! Miscellaneous helper/portability macros.

namespace best {
/// # `best::is_debug()`
///
/// Returns whether this program should have debug assertions enabled.
inline constexpr bool is_debug() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

/// # `BEST_HAS_BUILTIN()`
///
/// Tests whether a particular GCC-like builtin is available.
#ifndef __has_builtin
#define BEST_HAS_BUILTIN(x_) 0
#else
#define BEST_HAS_BUILTIN(x_) __has_builtin(x_)
#endif

/// # BEST_HAS_ATTRIBUTE()`
///
/// Tests whether a particular GCC-like attribute is available.
#ifndef __has_attribute
#define BEST_HAS_ATTRIBUTE(x_) 0
#else
#define BEST_HAS_ATTRIBUTE(x_) __has_attribute(x_)
#endif

/// # BEST_HAS_FEATURE()`
///
/// Tests whether a particular GCC-like feature test is available.
#ifndef __has_feature
#define BEST_HAS_FEATURE(x_) 0
#else
#define BEST_HAS_FEATURE(x_) __has_feature(x_)
#endif

/// # BEST_HAS_INCLUDE()`
///
/// Tests whether a particular `#include` statement would definitely succeed.
#ifndef __has_include
#define BEST_HAS_INCLUDE(x_) 0
#else
#define BEST_HAS_INCLUDE(x_) __has_include(x_)
#endif

/// # `BEST_WEAK`
///
/// Marks a symbol as weak.
///
/// When the linker is resolving a symbol `X`, and multiple definitions of `X`
/// exist, it is undefined behavior (in practice, the linker either produces
/// an error or picks one arbitrarily). However, if all definitions of `X` are
/// weak except for one, that one non-weak definition will be chosen.
///
/// This can be used to implement symbol overrides.
#define BEST_WEAK [[gnu::weak]]

#if BEST_HAS_ATTRIBUTE(enable_if)
/// # `BEST_HAS_ENABLE_IF`
///
/// True if `BEST_ENABLE_IF` is available.
#define BEST_HAS_ENABLE_IF 1

/// # `BEST_ENABLE_IF()`
///
/// Marks a symbol as "conditionally enabled".
///
/// This is a GCC version of SFINAE that has one nice property: you can pass
/// function arguments into it and evaluate functions on them.
#define BEST_ENABLE_IF(expr_, why_) __attribute__((enable_if(expr_, why_)))

/// # `BEST_ENABLE_IF_CONSTEXPR`
///
/// Like `BEST_ENABLE_IF()`, but merely requires `expr_` to be constexpr.
#define BEST_ENABLE_IF_CONSTEXPR(expr_) \
  __attribute__((                       \
    enable_if(__builtin_constant_p(expr_), "expected a constexpr value")))

#else
#define BEST_HAS_ENABLE_IF 0
#define BEST_ENABLE_IF(...)
#define BEST_ENABLE_IF_CONSTEXPR(...)
#endif  // BEST_HAS_ATTRIBUTE(enable_if)

/// # `BEST_PUSH_GCC_DIAGNOSTIC()`
///
/// Pushes a new context for `BEST_IGNORE_GCC_DIAGNOSTIC()`.
#define BEST_PUSH_GCC_DIAGNOSTIC() BEST_PRAGMA(GCC diagnostic push)

/// # `BEST_POP_GCC_DIAGNOSTIC()`
///
/// Pops the current context for `BEST_IGNORE_GCC_DIAGNOSTIC()`. In particular,
/// this undoes any ignores since the last `BEST_PUSH_GCC_DIAGNOSTIC()`.
#define BEST_POP_GCC_DIAGNOSTIC() BEST_PRAGMA(GCC diagnostic push)

/// # `BEST_IGNORE_GCC_DIAGNOSTIC()`
///
/// Ignores a particular named diagnostic. The name of the diagnostic should
/// be a string literal of the form `"-Wmy-diagnostic"`.
#define BEST_IGNORE_GCC_DIAGNOSTIC(W_) BEST_PRAGMA(GCC diagnostic ignored W_)

/// # `BEST_LINK_NAME()`
///
/// Specifies the linker symbol of a particular function declaration, overriding
/// the usual mangling. This should be placed after the argument list.
#define BEST_LINK_NAME(sym_) asm(sym_)
}  // namespace best

// Avoid including the whole header if we can include just the one that
// gives us the delicious, baked-into-the-language types.
#if BEST_HAS_INCLUDE("__compare/ordering.h")
#include <__compare/ordering.h>
#else
#include <compare>
#endif

/// # `BEST_IS_WINDOWS`
///
/// True if this platform is Microsoft Windows.
#if defined(_WIN32) || defined(WIN32)
#define BEST_IS_WINDOWS 1
#else
#define BEST_IS_WINDOWS 0
#endif

/// # `BEST_IS_POSIX`
///
/// True if this platform is a POSIX-like. It is assumed that all non-Windows
/// targets are POSIX-like.
#define BEST_IS_POSIX !BEST_IS_WINDOWS

#endif  // BEST_BASE_PORT_H_
