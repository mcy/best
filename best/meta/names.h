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

#include <type_traits>

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
  template <typename T>
  static const type_names of;

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
  constexpr explicit type_names(best::str name);
  best::str full_name_;
  size_t last_colcol_, first_angle_;
};

/// # `best::type_name<T>`
///
/// The short name of a type. To access longer versions of this name, use
/// `best::type_names::of<T>`.
template <typename T>
inline constexpr best::str type_name =
    [] { return best::type_names::of<T>.ident(); }();

/// # `best::field_name<T>
///
/// The name of a field represented by a pointer-to-member.
template <auto pm>
  requires std::is_member_pointer_v<decltype(pm)>
inline constexpr best::str field_name = [] {
  auto offsets = names_internal::FieldOffsets;
  auto raw = names_internal::raw_name<pm>();
  auto name =
      best::str(unsafe("the compiler made this string, it better be UTF-8"),
                raw[{
                    .start = offsets.prefix,
                    .count = raw.size() - offsets.prefix - offsets.suffix,
                }]);

  // `name` is going to be scoped, so we need to strip off a leading path.
  return names_internal::remove_namespace(name);
}();

/// # `best::value_name<T>
///
/// The name of an enum value, if `e` refers to a named enumerator.
template <auto e>
inline constexpr best::option<best::str> value_name = [] {
  auto offsets = names_internal::ValueOffsets;
  auto raw = names_internal::raw_name<e>();
  auto name =
      best::str(unsafe("the compiler made this string, it better be UTF-8"),
                raw[{
                    .start = offsets.prefix,
                    .count = raw.size() - offsets.prefix - offsets.suffix,
                }]);

  // `name` is going to be scoped, so we need to strip off a leading path.
  return names_internal::remove_namespace(name);
}();
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
constexpr type_names::type_names(best::str name) : full_name_(name) {
  first_angle_ = full_name_.find('<').value_or(full_name_.size());

  last_colcol_ = first_angle_ - names_internal::remove_namespace(
                                    full_name_[{.end = first_angle_}])
                                    .size();
}

template <typename T>
inline constexpr type_names type_names::of{[] {
  auto offsets = names_internal::TypeOffsets;
  auto raw = names_internal::raw_name<T>();
  return best::str(unsafe("the compiler made this string, it better be UTF-8"),
                   raw[{
                       .start = offsets.prefix,
                       .count = raw.size() - offsets.prefix - offsets.suffix,
                   }]);
}()};
}  // namespace best
#endif  // BEST_META_REFLECT_H_
