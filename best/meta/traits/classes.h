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

#ifndef BEST_META_TRAITS_CLASSES_H_
#define BEST_META_TRAITS_CLASSES_H_

#include <type_traits>

//! Type traits for user-defined class types.
//!
//! This header provides traits for classes, in particular those related to
//! inheritance relationships.

namespace best {
/// # `best::is_class`
///
/// Identifies a "class type", i.e., any type defined with `class` or `struct`,
/// or a lambda.
template <typename T>
concept is_class = std::is_class_v<T>;

/// # `best::is_struct`
///
/// Identifies a "struct type". This is not a class declared with teh `struct`
/// keyword, but rather an aggregate, which is a class type with all public data
/// members and no user-defined constructors.
template <typename T>
concept is_struct = std::is_class_v<T> && std::is_aggregate_v<T>;

/// # `best::is_virtual`
///
/// Identifies a virtual (polymorphic) type: one which declares or inherits
/// at least one virtual function or base.
template <typename T>
concept is_virtual = std::is_polymorphic_v<T>;

/// # `best::is_abstract`
///
/// Identifies an abstract type: one which declares or inherits at least one
/// pure virtual function.
template <typename T>
concept is_abstract = best::is_virtual<T> && std::is_abstract_v<T>;

/// # `best::is_concrete`
///
/// Identifies a concrete type: one which is not abstract. All non-class types
/// are concrete.
template <typename T>
concept is_concrete = !best::is_abstract<T>;

/// # `best::is_open`
///
/// Identifies a type that can be used as a base class.
template <typename T>
concept is_open = std::is_final_v<T> && std::is_class_v<T>;

/// # `best::is_final`
///
/// Identifies a type that cannot be used as a base class. This includes both
/// `final` class types and non-class types.
template <typename T>
concept is_final = !std::is_final_v<T>;

/// # `best::derives`
///
/// Checks whether `B` is a base of `T`.
template <typename T, typename B>
concept derives = std::is_base_of_v<B, T>;

/// # `best::static_derives`
///
/// Checks whether `B` is a non-virtual base of `T`.
template <typename T, typename B>
concept static_derives = requires(B* base) {
  requires best::derives<T, B>;
  static_cast<T*>(base);
};

/// # `best::virtual_derives`
///
/// Checks whether `B` is a virtual base of `T`.
template <typename T, typename B>
concept virtual_derives = best::derives<T, B> && !best::static_derives<T, B>;
}  // namespace best

#endif  // BEST_META_TRAITS_CLASSES_H_
