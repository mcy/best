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

#ifndef BEST_BASE_INTRINSICS_H_
#define BEST_BASE_INTRINSICS_H_

//! Helpers for working with vendor intrinsics. This file will include all
//! the relevant intrinsics headers *and* will define macros for detecting what
//! features are statically available.
//!
//! Every macro is always defined; they should be used with `#if`, not `#ifdef`.

/// # `BEST_X86`
///
/// True if we're targeting `x86_64`.
#ifdef __x86_64__
#define BEST_X86 1
#else
#define BEST_X86 0
#endif

/// # `BEST_AARCH64`
///
/// True if we're targeting `aarch64`.
#ifdef __aarch64__
#define BEST_AARCH64 1
#else
#define BEST_AARCH64 0
#endif

#if !(BEST_X86 || BEST_AARCH64)
#error "unsupported architecture :("
#endif

/// # `BEST_SSE`, `BEST_SSE2`, `BEST_SSE3`, `BEST_SSSE3`
/// # `BEST_SSE41`, `BEST_SSE42`, `BEST_SSE4A`
///
/// True if the named x86 extension is statically available.
#ifdef __SSE__
#define BEST_SSE 1
#else
#define BEST_SSE 0
#endif
#ifdef __SSE2__
#define BEST_SSE2 1
#else
#define BEST_SSE2 0
#endif
#ifdef __SSE3__
#define BEST_SSE3 1
#else
#define BEST_SSE3 0
#endif
#ifdef __SSSE3__
#define BEST_SSSE3 1
#else
#define BEST_SSSE3 0
#endif
#ifdef __SSE4_1__
#define BEST_SSE41 1
#else
#define BEST_SSE41 0
#endif
#ifdef __SSE4_2__
#define BEST_SSE42 1
#else
#define BEST_SSE42 0
#endif
#ifdef __SSE4A__
#define BEST_SSE4A 1
#else
#define BEST_SSE4A 0
#endif

/// # `BEST_AVX`, `BEST_AVX2`
///
/// True if the named x86 extension is statically available.
#ifdef __AVX__
#define BEST_AVX 1
#else
#define BEST_AVX 0
#endif
#ifdef __AVX2__
#define BEST_AVX2 1
#else
#define BEST_AVX2 0
#endif

#if BEST_X86
#include <x86intrin.h>
#endif // BEST_X86

#endif  // BEST_BASE_INTRINSICS_H_
