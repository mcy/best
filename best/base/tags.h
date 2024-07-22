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

#ifndef BEST_BASE_TAGS_H_
#define BEST_BASE_TAGS_H_

#include <stddef.h>

//! Commonly-used tag types.
//!
//! These overlap with some of the ones from the STL; where possible,
//! they are aliases!
///
/// See https://abseil.io/tips/198 to get to know tag types better.
namespace best {
/// A helper for ranked overloading. See https://abseil.io/tips/229.
template <size_t n>
struct rank : rank<n - 1> {};
template <>
struct rank<0> {};

/// An alias for std::in_place.
///
/// Use this to tag variadic constructors that represent constructing a value
/// in place.
inline constexpr struct in_place_t {
} in_place;

/// # `best::bind`
///
/// A tag for overriding the standard CTAD behavior of types like `best::row`
/// and `best::option`. By default, these types strip references off of their
/// arguments during CTAD, but if you pass `best::refs` as the first
/// constructor, it will cause them to keep the references.
inline constexpr struct bind_t {
} bind;

/// A tag for uninitialized values.
///
/// Use this to define a non-default constructor that produces some kind of
/// "uninitialized" value.
struct uninit_t {};
inline constexpr uninit_t uninit;

/// # `best::ftadle`
///
/// A tag used for ensuring that FTADLE implementations are actually
/// FTADLEs. With some exceptions, every FTADLE implementation must tolerate
/// being passed ANY type in an unevaluated context. This also ensures we have
/// *a* type to pass in that position, so that calling the FTADLE finds
/// overloads in the best:: namespace, too.
struct ftadle final {};

namespace tags_internal_do_not_use {
struct ctad_guard;
#define BEST_CTAD_GUARD_(type_, Tag_)                                     \
  static_assert(                                                          \
    ::std::is_same_v<Tag_, ::best::tags_internal_do_not_use::ctad_guard>, \
    "you may not instantiate " type_ " manually; please use CTAD instead")
}  // namespace tags_internal_do_not_use
}  // namespace best

#endif  // BEST_BASE_TAGS_H_
