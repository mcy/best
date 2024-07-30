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

#ifndef BEST_META_TRAITS_OBJECTS_H_
#define BEST_META_TRAITS_OBJECTS_H_

#include <type_traits>

//! Object type traits.
//!
//! This header provides traits related to object types. In C++, an "object
//! type" is any type that is not void, a reference, or a function. `best`
//! additionally does not consider array types to be object types, because they
//! do not behave like aggregates as one might expect.

namespace best {
/// # `best::is_object`
///
/// An object type: anything that is not a reference, function, void, or an
/// array.
template <typename T>
// TODO: make arrays not be objects.
concept is_object = std::is_object_v<T> /*&& !std::is_array_v<T>*/;
}  // namespace best

#endif  // BEST_META_TRAITS_OBJECTS_H_
