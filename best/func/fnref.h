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

#ifndef BEST_FUNC_FNREF_H_
#define BEST_FUNC_FNREF_H_

#include "best/func/internal/fnref.h"
#include "best/meta/internal/abominable.h"
#include "best/meta/taxonomy.h"

//! Dynamic function references.
//!
//! A `best::fnref` is a type-erased function owned by a caller. It is similar
//! to a `std::function`, but never allocates on the heap. It is useful for
//! passing arbitrary callables into non-templates.

namespace best {
/// `best::fnref<R(...) const>`
///
/// A function reference. This type is a function pointer with associated data
/// captured by reference. You can construct a `best::fnref` from a lambda or
/// a function pointer, or another `best::fnref`, or any other callable.
///
/// The type of `Signature` must be some function type, such as `int()` or
/// `void(int, int) const`. The `const` will indicate whether this function
/// reference can be constructed from const references or not (in other words,
/// whether or not this is a "mutable" function reference).
///
/// `best::fnref` is a somewhat dangerous type to return or place into a
/// container. It's not forbidden, but it's something to do carefully.
template <typename Signature>
class fnref final : best::abominable_internal::tame<Signature>::template apply<
                        fnref_internal::impl> {
 private:
  using impl_t = best::abominable_internal::tame<Signature>::template apply<
      fnref_internal::impl>;

 public:
  /// # `fnref::output`
  ///
  /// The output of this function.
  using output = impl_t::output;

  /// # `fnref::is_const`
  ///
  /// Whether or not this is a "const" function reference.
  static constexpr bool is_const = best::is_const_func<Signature>;
  static_assert(!best::is_ref_func<Signature>,
                "cannot use ref-qualified function types with best::fnref");

  /// # `fnref::fnref()`
  ///
  /// Constructs a new fnref. It may be constructed from a closure, a function
  /// pointer, or `nullptr`.
  using impl_t::impl_t;

  /// # `fnref()`
  ///
  /// Calls the function.
  using impl_t::operator();

  /// # `fnref::operator==`
  ///
  /// `fnref`s may be compared to `nullptr` (and no other pointer).
  using impl_t::operator==;
  using impl_t::operator bool;
};
}  // namespace best

#endif  // BEST_FUNC_FNREF_H_
