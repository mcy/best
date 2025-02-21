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

#ifndef BEST_BASE_HINT2_H_
#define BEST_BASE_HINT2_H_

#include "best/base/internal/macro.h"

//! Standard macros.
//!
//! This header provides macros for writing macros.

namespace best {
/// # `BEST_STRINGIFY()`
///
/// Stringifies a token string.
#define BEST_STRINGIFY(...) BEST_STRINGIFY_(__VA_ARGS__)
#define BEST_STRINGIFY_(...) #__VA_ARGS__

/// # `BEST_PRAGMA()`
///
/// Helper macro for generating more readable pragma definitions.
#define BEST_PRAGMA(...) _Pragma(BEST_STRINGIFY(__VA_ARGS__))

/// # `BEST_COUNT()`
///
/// Expands to the number of arguments passed to a macro, up to 63.
#define BEST_COUNT(...) \
  BEST_COUNT_(BEST_ADD_TRAILING_COMMA(__VA_ARGS__) BEST_COUNT_SEQ_)
#define BEST_COUNT_(...) BEST_COUNT_N_(__VA_ARGS__)

/// # `BEST_ADD_TRAILING_COMMA()`, `BEST_REMOVE_TRAILING_COMMA()`
///
/// Expands to its arguments, but with an optional trialing comma added or
/// removed.
#define BEST_ADD_TRAILING_COMMA(...) \
  BEST_REMOVE_TRAILING_COMMA(__VA_ARGS__) __VA_OPT__(, )
#define BEST_REMOVE_TRAILING_COMMA(...) \
  BEST_COUNT_(__VA_ARGS__, BEST_RTC_SEQ_)(__VA_ARGS__)

/// # `BEST_NTH()`
///
/// If `idx` is a decimal integer literal up to 63, expands to the `idx`th
/// argument that follows it.
#define BEST_NTH(idx_, ...) BEST_NTH_##idx_##_(__VA_ARGS__)

/// # `BEST_EXPAND()`
///
/// Expands to its arguments, but after forcing some large number of rescans.
/// Calls to `_REC` macros must be wrapped in a top-level call to `BEST_EXPAND`.
#define BEST_EXPAND(...) BEST_EXPAND_(__VA_ARGS__)

/// # `BEST_VARIADIC()`
///
/// Expands to a call of `MACRO_n_(...)`, where `n` is `BEST_COUNT(...)`.
#define BEST_VARIADIC(MACRO, ...) \
  BEST_EXPAND(BEST_VARIADIC_REC(MACRO, __VA_ARGS__))

/// # `BEST_VARIADIC_REC()`
///
/// Like `BEST_VARIADIC()`, but may be called recursively inside of a call to
/// `BEST_EXPAND()`.
#define BEST_VARIADIC_REC(MACRO, ...) BEST_VARIADIC_(MACRO, __VA_ARGS__)

/// # `BEST_MAP()`, `BEST_MAP_JOIN()`
///
/// Expands to `MACRO` applied to each argument passed to this macro. The `JOIN`
/// variant will intersperse each call with the given separator. The separator
/// may be wrapped in parentheses if it contains commas.
#define BEST_MAP(MACRO, ...) BEST_MAP_JOIN(MACRO, (), __VA_ARGS__)
#define BEST_MAP_JOIN(MACRO, sep_, ...) \
  BEST_VARIADIC(BEST_MAP, MACRO, sep_, __VA_ARGS__)

/// # `BEST_MAP_REC()`, `BEST_MAP_JOIN_REC()`
///
/// Like `BEST_MAP()`, but may be called recursively.
#define BEST_MAP_REC(MACRO, ...) BEST_MAP_JOIN_REC(MACRO, (), __VA_ARGS__)
#define BEST_MAP_JOIN_REC(MACRO, sep_, ...) \
  BEST_VARIADIC_REC(BEST_MAP, MACRO, sep_, __VA_ARGS__)

/// # `BEST_IMAP()`, `BEST_MAP_JOIN()`
///
/// Like `BEST_MAP()`, but each call to `MACRO` includes the index of the
/// argument as a decimal integer literal.
#define BEST_IMAP(MACRO, ...) BEST_IMAP_JOIN(MACRO, (), __VA_ARGS__)
#define BEST_IMAP_JOIN(MACRO, sep_, ...) \
  BEST_VARIADIC(BEST_IMAP, MACRO, sep_, __VA_ARGS__)

/// # `BEST_IMAP_REC()`, `BEST_IMAP_JOIN_REC()`
///
/// Like `BEST_IMAP()`, but may be called recursively.
#define BEST_IMAP_REC(MACRO, ...) BEST_IMAP_JOIN_REC(MACRO, (), __VA_ARGS__)
#define BEST_IMAP_JOIN_REC(MACRO, sep_, ...) \
  BEST_VARIADIC_REC(BEST_IMAP, MACRO, sep_, __VA_ARGS__)

/// # `BEST_PARENS()`
///
/// Expands to the result of wraping its argument(s) in parentheses. Useful for
/// delaying parsing of macro calls.
#define BEST_PARENS(...) (__VA_ARGS__)

/// # `BEST_REMOVE_PARENS()`
///
/// Expands to the result of removing the parentheses around an expression.
#define BEST_REMOVE_PARENS(...) BEST_REMOVE_PARENS_(__VA_ARGS__)

/// # `BEST_COMMA()`
///
/// Expands to a comma if passed more than zero arguments.
#define BEST_COMMA(...) __VA_OPT__(, )

}  // namespace best

#endif  // BEST_BASE_HINT_H_
