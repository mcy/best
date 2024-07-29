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

#ifndef BEST_META_TRAITS_PTRS_H_
#define BEST_META_TRAITS_PTRS_H_

#include <type_traits>

#include "best/meta/traits/funcs.h"

//! Pointer type traits.
//!
//! This header provides traits related to C++ raw pointer types.

namespace best {
/// # `best::is_raw_ptr`
///
/// Whether this is a raw pointer type. This does not include pointer-to-member
/// types.
template <typename T>
concept is_raw_ptr = std::is_pointer_v<T>;

/// # `best::un_raw_ptr<T>`
///
/// If `T` is an object, function, or void raw pointer, returns its pointee;
/// otherwise, returns `T`.
template <typename T>
using un_raw_ptr = std::remove_pointer_t<T>;

/// # `best::is_member_ptr`
///
/// Whether this is a pointer-to-member type.
template <typename T>
concept is_member_ptr = std::is_member_pointer_v<T>;

/// # `best::is_func_ptr`
///
/// Whether this is a function pointer type.
template <typename T>
concept is_func_ptr = best::is_raw_ptr<T> && best::is_func<best::un_raw_ptr<T>>;

/// # `best::as_raw_ptr<T>`
///
/// If `T` is an object, void, or tame function type, returns a pointer to it.
/// If `T` is an abominable function, it returns `best::tame<F>*`. If `T` is
/// a reference type, it returns `best::un_ref<T>*`.
///
/// The returned type is *always* a raw pointer.
template <typename T>
using as_raw_ptr = std::add_pointer_t<best::tame<T>>;

/// # `best::addr()`
///
/// Obtains the address of a reference without hitting `operator&`.
constexpr auto addr(auto&& ref) { return __builtin_addressof(ref); }
}  // namespace best

#endif  // BEST_META_TRAITS_PTRS_H_
