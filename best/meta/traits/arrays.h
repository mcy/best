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

#ifndef BEST_META_TRAITS_ARRAYS_H_
#define BEST_META_TRAITS_ARRAYS_H_

#include <type_traits>

#include "best/base/fwd.h"

//! Type traits for array types.
//!
//! This header provides traits for classes, in particular those related to
//! inheritance relationships.

namespace best {
/// # `best::is_array`
///
/// Identifies an array type.
template <typename T>
concept is_array = std::is_array_v<T>;

/// # `best::as_array<T, n>`
///
/// Adds an array bound to `T`. If `extent` is zero, returns `Default` instead.
/// This is valuable for handling the zero length case gracefully.
template <typename T, size_t extent, typename Default = best::empty>
using as_array = best::select<extent == 0, Default, T[extent + (extent == 0)]>;

/// # `best::un_array<T>
///
/// Removes a single array modifier from `T`, if it is an array.
template <typename T>
using un_array = std::remove_extent_t<T>;

/// # `best::is_sized_array`
///
/// Identifies a sized array type, i.e. `T[n]`.
template <typename T>
concept is_sized_array = std::is_bounded_array_v<T>;

/// # `best::is_unsized_array`
///
/// Identifies a unsized array type, i.e. `T[]`.
template <typename T>
concept is_unsized_array = std::is_unbounded_array_v<T>;

/// # `best::shape_of<T>`
///
/// Static information about the shape of the multi-dimensional array type `T`.
/// If `T` is not an array, returns a `best::shape` of rank `0`. If `T` is an
/// unsized array, the outermost dimension extent will be `-1`.
template <typename T>
inline constexpr auto shape_of = [] {
  if constexpr (!best::is_array<T>) {
    return best::shape<T, 0>({});
  } else {
    constexpr auto inner = best::shape_of<best::un_array<T>>;
    size_t dims[inner.rank() + 1] = {};
    for (size_t i = 0; i < inner.rank(); ++i) { dims[i + 1] = inner.dims()[i]; }

    if constexpr (best::is_sized_array<T>) {
      dims[0] = std::extent_v<T>;
    } else {
      dims[0] = -1;
    }

    return best::shape<T, inner.rank() + 1>(dims);
  }
}();

/// # `best::shape<T, rank>`
///
/// Static information about the shape of a multi-dimensional array type.
template <typename T, size_t rank_>
class shape {
  using span = best::span<best::dependent<size_t, T>, rank_>;

 public:
  /// # `shape::element`
  ///
  /// The type of the elements in the array.
  using element = T;

  /// # `shape::rank()`
  ///
  /// The number of dimensions in the shape.
  static constexpr size_t rank() { return rank_; }

  /// # `shape::is_scalar()`
  ///
  /// Returns whether this is a scalar shape, i.e., rank zero.
  static constexpr bool is_scalar() { return rank() == 0; }

  /// # `shape::shape()`
  ///
  /// Constructs a new shape with the given dimensions.
  constexpr explicit shape(span dims) {
    if constexpr (rank() > 0) { span(dims_).emplace_from(dims); }
  }

  /// # `shape::is_unsized()`
  ///
  /// Returns whether this is an unsized shape (i.e., the outermost dimension
  /// is -1).
  constexpr bool is_unsized() const { return rank() > 0 && dims()[0] == 0; }

  /// # `shape::dims()`
  ///
  /// The dimensions for this shape, from outermost to innermost. If this is
  /// the shape of an unsized array, `dims()[0]` will be zero.
  constexpr span dims() const {
    if constexpr (rank() == 0) {
      return {};
    } else {
      return dims_;
    }
  }

  /// # `shape::volume()`
  ///
  /// The total number of elements in this shape, i.e., the product of all
  /// the dimensions. If `best::rank() == 0`, this returns 1. If this is an
  /// unsized shape, returns 0.
  constexpr size_t volume() const {
    if (is_unsized()) { return 0; }

    size_t total = 1;
    for (size_t dim : dims()) { total *= dim; }
    return total;
  }

 private:
  best::as_array<size_t, rank()> dims_{};
};
}  // namespace best

#endif  // BEST_META_TRAITS_ARRAYS_H_
