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

#ifndef BEST_FUNC_FNREF2_H_
#define BEST_FUNC_FNREF2_H_

#include "best/func/internal/fn.h"

//! Dynamic function references.
//!
//! A `best::fn` is an interface for a lambda or other callable. It is similar
//! to a `std::function`, but how it is allocated depends on what pointer type
//! it is used with. It is useful for passing arbitrary callables into
//! non-templates.

namespace best {
/// # `best::fn<R(...) const>`
///
/// A `best::interface` with a single `operator()` interface function. Combined
/// with `best::ptr` or `best::box`, this allows passing type-erased lambdas
/// into functions.
///
/// The type of `Signature` must be some function type, such as `int()` or
/// `void(int, int) const`. The `const` will indicate whether this function
/// reference can be constructed from const references or not (in other words,
/// whether or not this is a "mutable" function reference).
template <typename Signature>
using fn =
  best::traits_internal::tame<Signature>::template apply<fn_internal::fn>;

template <typename Signature>
using fnptr = best::ptr<best::dyn<best::fn<Signature>>>;
}  // namespace best

#endif  // BEST_FUNC_FNREF_H_
