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

#ifndef BEST_META_TRAITS_REFS_H_
#define BEST_META_TRAITS_REFS_H_

#include <type_traits>

#include "best/meta/traits/quals.h"

//! References type traits.
//!
//! This header provides traits related to reference types.
//!
//! If `T` is an object, array, or tame function type, there exist two
//! additional types `T&` and `T&&`. From `best`'s perspective, `T&` and `T&&`
//! are essentially `T* const` which is not null.

namespace best {
/// # `BEST_FWD()`
///
/// Macro implementation of `std::forward`.
///
/// Intended to improve compile times and gdb debugging by eliminating an
/// extremely common function that must be inlined.
#define BEST_FWD(expr_) (static_cast<decltype(expr_)&&>(expr_))

/// # `BEST_MOVE()`
///
/// Macro implementation of `std::forward`.
///
/// Intended to improve compile times and gdb debugging by eliminating an
/// extremely common function that must be inlined.
#define BEST_MOVE(expr_) \
  (static_cast<::best::as_rref<::best::un_ref<decltype(expr_)>>>(expr_))

/// # `best::ref_kind`
///
/// A kind of reference.
enum class ref_kind : uint8_t {
  Lvalue = 0,  // T&
  Rvalue = 1,  // T&&
};

/// # `best::is_lref`
///
/// An lvalue reference type.
template <typename T>
concept is_lref = std::is_lvalue_reference_v<T>;

/// # `best::is_rref`
///
/// An rvalue reference type.
template <typename T>
concept is_rref = std::is_rvalue_reference_v<T>;

/// # `best::is_ref`
///
/// A reference type, selectable based on the `kind` parameter. Not specifying
/// it makes this concept recognize both kinds of references.
template <typename T, ref_kind kind = ref_kind(-1)>
concept is_ref = (kind != ref_kind::Rvalue && is_lref<T>) ||
                 (kind != ref_kind::Lvalue && is_rref<T>);

/// # `best::as_ref<T>
///
/// If `T` is an object or tame function type, returns an (lvalue) reference to
/// it. If `T` is an lvalue reference or void, returns `T`. If `T` is an rvalue
/// reference, returns `best::un_ref<T>&&`.
///
/// Currently, abominable functions are left unmodified. This may change in the
/// future.
template <typename T>
using as_ref = std::add_lvalue_reference_t<T>;

/// # `best::as_rref<T>
///
/// If `T` is an object or tame function type, returns an rvalue reference to
/// it. If `T` is an reference or void, returns `T`.
///
/// Currently, abominable functions are left unmodified. This may change in the
/// future.
template <typename T>
using as_rref = std::add_rvalue_reference_t<T>;

/// # `best::copy_ref<Dst, Src>`
///
/// If `Src` is a reference type, the kind of reference, and cv-qualification,
/// are copied to `Dst`, as if by the appropriate calls to `as_ref<>` or
/// `as_rref<>`.
template <typename Dst, typename Src>
using copy_ref = best::traits_internal::refs<Dst, Src>::copied;

/// # `best::un_ref<T>`
///
/// If `T` is `U&` or `U&&` for some type `U`, returns `U`; otherwise returns
/// `T`.
template <typename T>
using un_ref = std::remove_reference_t<T>;

/// # `best::as_auto<T>`
///
/// Performs `auto` deduction: if `T` is a reference and/or is cv-qualified,
/// the reference and qualification is removed.
template <typename T>
using as_auto = best::un_qual<best::un_ref<T>>;
}  // namespace best
#endif  // BEST_META_TRAITS_REFS_H_
