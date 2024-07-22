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

#ifndef BEST_BASE_NICHE_H_
#define BEST_BASE_NICHE_H_

#include <stddef.h>

#include "best/base/ord.h"
#include "best/meta/init.h"

//! Niche representations.
//!
//! A niche representation of a type `T` is a `best::object<T>` that contains an
//! otherwise invalid value of `T`. No operations need to be valid for a niche
//! representation, not even the destructor.
//!
//! If `T` has a niche representation, it must satisfy `best::constructible<T,
//! niche>` and `best::equatable<T, niche>`. Only the niche representation
//! (obtained only by constructing from a `best::niche`) must compare as equal
//! to `best::niche`.
//!
//! Niche representations are used for compressing the layout of some types,
//! such as `best::choice`.

namespace best {
/// # `best::niche`
///
/// A tag for constructing niche representations.
struct niche final {};

/// # `best::has_niche`
///
/// Whether T is a type with a niche.
template <typename T>
concept has_niche =
  best::is_ref<T> || (best::is_object<T> && best::constructible<T, niche> &&
                      best::equatable<T, best::niche>);
}  // namespace best

#endif  // BEST_BASE_NICHE_H_
