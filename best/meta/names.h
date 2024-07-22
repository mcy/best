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

#ifndef BEST_META_NAMES_H_
#define BEST_META_NAMES_H_

#include "best/meta/internal/names.h"
#include "best/text/str.h"

//! Name reflection.
//!
//! Utilities for obtaining the names of C++ program entities, such as types and
//! enum variants.

namespace best {
/// # `best::type_names`
///
/// The pretty-printed names of some type. This type provides access to the name
/// of a type in various formats; generally, you'll want the "good default" of
/// `best::type_name`.
class type_names final {
 public:
  /// # `type_names::of<T>`
  ///
  /// Extracts the names of tye type `T`.
  template <typename T, typename type_names_ = type_names>
  static constexpr type_names_ of = names_internal::parse<T, type_names_>();

  /// # `type_names::name()`
  ///
  /// Returns this type's identifier, i.e., without its path prefix or its
  /// template parameters.
  ///
  /// This is what gets used for debug printing by default.
  constexpr best::str name() const {
    return full_name_[{.start = last_colcol_, .end = first_angle_}];
  }

  /// # `type_names::path()`
  ///
  /// Returns this type's full path (but not its template parameters).
  constexpr best::str path() const { return full_name_[{.end = first_angle_}]; }

  /// # `type_names::name_space()`
  ///
  /// Returns this type's containing namespace. May be empty.
  constexpr best::str name_space() const {
    return full_name_[{.end = best::saturating_sub(last_colcol_, 2)}];
  }

  /// # `type_names::namespace_()`
  ///
  /// Returns this type's template parameters. May be empty.
  constexpr best::str params() const {
    return full_name_[{.start = first_angle_}];
  }

  /// # `type_names::template_ident()`
  ///
  /// Returns this type's identifier with template parameters.
  constexpr best::str name_with_params() const {
    return full_name_[{.start = last_colcol_}];
  }

  /// # `type_names::full()`
  ///
  /// Returns this type's full path with template parameters.
  constexpr best::str path_with_params() const { return full_name_; }

 private:
  best::str full_name_;
  size_t last_colcol_, first_angle_;

 public:
  constexpr explicit type_names(names_internal::priv, best::str name);
};

/// # `best::type_name<T>`, `best::field_name<pm>`, `best::enum_name<e>`
///
/// The name of a type, a field, or an enum value.
template <typename T>
inline constexpr best::str type_name = type_names::of<T>.name();
template <best::is_member_ptr auto pm>
inline constexpr best::str field_name = best::names_internal::parse<pm>();
template <best::is_enum auto e>
inline constexpr best::option<best::str> value_name = BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wenum-constexpr-conversion")
    best::names_internal::parse<e>();
BEST_POP_GCC_DIAGNOSTIC()
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
constexpr type_names::type_names(names_internal::priv, best::str name)
  : full_name_(name) {
  first_angle_ = full_name_.find('<').value_or(full_name_.size());

  last_colcol_ =
    first_angle_ -
    names_internal::remove_namespace(full_name_[{.end = first_angle_}]).size();
}

}  // namespace best
#endif  // BEST_META_NAMES_H_
