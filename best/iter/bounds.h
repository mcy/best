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

#ifndef BEST_ITER_BOUNDS_H_
#define BEST_ITER_BOUNDS_H_

#include <array>
#include <cstddef>
#include <type_traits>

#include "best/base/fwd.h"
#include "best/container/internal/simple_option.h"
#include "best/log/location.h"
#include "best/math/overflow.h"

//! Callsite-readable array access bounds specifications.
//!
//! `best::bounds` is analogous to Rust's various `Range` types, and allows
//! writing accesses of array-like types with keyword arguments.
//!
//! For example, to use Rust/Go/Python-style start-end access, you might write
//!
//! ```
//! array[{.start = 2, .end = 4}]  // Produces a 2-element span.
//! ```
//!
//! For C++-style start-count access, you would instead use
//!
//! ```
//! array[{.start = 2, .count = 4}]  // Produces a 4-element span.
//! ```
//!
//! `best::bounds` provides functions for computing the size of the resulting
//! slicing operation regardless of how the bounds are specified, and for
//! performing bounds checks in a standardized way, including reporting a bounds
//! check failure to the user.
//!
//! It goes without saying, but best does not provide a way to globally disable
//! bounds checks.

namespace best {
/// # `best::bounds`.
///
/// A specification for a subrange of some contiguous range.
using bounds = best::int_range<size_t>;

namespace bounds_internal {
/// Prints a nice error for bounds check failure and crashes.
[[noreturn]] void crash(bounds, size_t max_size, best::location loc);
}  // namespace bounds_internal

///
/// To specify slicing a `best::span` starting at index 2 and ending at index 4,
/// we must specify a start (how much to offset the base pointer) and a count
/// (the number of elements in the resulting array).
///
/// ```
/// array[{.start = 2, .end = 4}]
/// ```
///
/// This contains elements 2 and 3, but not 4, since the end is exclusive. For
/// it to contain element 4, mark the bounds as inclusive:
///
/// ```
/// array[{.start = 2, .including_end = 4}]
/// ```
///
/// Instead of specifying an end, you can specify a count. In this case,
/// The end is inferred to be `start + count`.
///
/// ```
/// array[{.start = 2, .count = 3}]
/// ```
///
/// If neither end or count are specified, the end of the bounds is deduced
/// to be the end of the parent span.
///
/// ```
/// array[{.start = 2}]  // .end is implicit.
/// ```
///
/// Similarly, start defaults to the beginning of the span (i.e., index 0).
template <typename Int>
struct int_range final {
 private:
  using opt_t = container_internal::option<Int>;

 public:
  /// # `int_range::start`
  ///
  /// The start index for these bounds.
  Int start = 0;

  /// # `int_range::end`, `int_range::including_end`, `int_range::count`
  ///
  /// The end index for these bounds.
  ///
  /// `end` is measured from the absolute start of the indexed range.
  /// `including_end` is the same as specifying `end + 1`.
  /// `count` is measured from `start`.
  ///
  /// If more than one is specified, the first one in the above list wins.
  opt_t end, including_end, count;

  /// # `int_range::iter()`
  ///
  /// Returns an iterator over the indices encompassed by this `int_range`.
  constexpr auto iter() const;

  /// # `int_range::wrapping_normalize`
  ///
  /// Normalizes this `int_range` so that only `count` is set. If `max_size` is
  /// not provided, it is assumed to be the maximum possible size.
  ///
  /// Note that this treats the range of `int_range` as wrapping around: if `end
  /// < start`, this will compute `end - start` as the count.
  constexpr int_range wrapping_normalize(Int max_size = Max) const;

  /// # `bounds::compute_count()`
  ///
  /// Computes the count (i.e., `end - start`, whatever that might be), given
  /// a maximum size for the underlying contiguous range.
  ///
  /// If the access would be out of bounds, crashes.
  constexpr size_t compute_count(size_t max_size,
                                 best::location loc = best::here) const
    requires std::is_same_v<size_t, Int>;

  /// # `int_range::compute_count()`
  ///
  /// Like `compute_count()`, indicates failure in the return type.
  ///
  /// Also allows for a potentially missing `max_size`, in which case no
  /// explicit endpoint is an error.
  constexpr opt_t try_compute_count(opt_t max_size) const
    requires std::is_same_v<size_t, Int>;

  /// # `int_range:with_location`
  ///
  /// A carbon copy of `int_range` but which captures a `best::location` on
  /// creation.
  ///
  /// This is needed for the rare case where you want to take a bounds + a
  /// caller location into an `operator[]`, which does not allow multiple
  /// arguments in C++20.
  struct with_location final {
    /// The fields of a `best::bounds`.
    size_t start = 0;
    opt_t end, including_end, count;

    /// # `with_location::where`
    ///
    /// The captured location.
    best::location where = best::here;

    /// # `int_range::compute_count()`
    ///
    /// Forwards to `int_range::compute_count()`, but passing in `where` to it
    /// as the location for the bounds check crash.
    constexpr size_t compute_count(size_t max_size) const
      requires std::is_same_v<size_t, Int>
    {
      return to_bounds().compute_count(max_size, where);
    }

    /// # `int_range::to_bounds()`
    ///
    /// Converts to an equivalent `best::bounds`.
    constexpr bounds to_bounds() const {
      return {start, end, including_end, count};
    }
    constexpr operator bounds() const { return to_bounds(); }
  };

  constexpr operator with_location() const {
    return {start, end, including_end, count};
  }

  friend void BestFmt(auto& fmt, const int_range& bounds) {
    /// NOTE: This does not use the struct printer because we want very
    /// fine-grained control so this looks like syntax the user would ordinarily
    /// write.
    fmt.write('{');
    if (bounds.start != 0) {
      fmt.format(".start = {}", bounds.start);
    }
    if (bounds.end) {
      if (bounds.start != 0) fmt.write(", ");
      fmt.format(".end = {}", *bounds.end);
    } else if (bounds.including_end) {
      if (bounds.start != 0) fmt.write(", ");
      fmt.format(".including_end = {}", *bounds.including_end);
    } else if (bounds.count) {
      if (bounds.start != 0) fmt.write(", ");
      fmt.format(".count = {}", *bounds.count);
    }
    fmt.write('}');
  }

 private:
  struct iter_impl {
    Int start, count;
    bool plus_one = false;

    constexpr opt_t next() {
      if (plus_one) {
        plus_one = false;

        auto ret = start;
        start = (best::overflow(start) + 1).wrap();
        return ret;
      }

      if (count == 0) return {};
      count = (best::overflow(count) - 1).wrap();

      auto ret = start;
      start = (best::overflow(start) + 1).wrap();
      return ret;
    }

    constexpr std::array<size_t, 2> size_hint() const {
      // TODO(mcyoung): This will overflow for uint64 ranges, need to return
      // something else.
      return {count, count};
    }
  };

  constexpr int_range normalize(opt_t max_size) const;

  static constexpr Int Max =
      std::make_unsigned_t<Int>(-1) >> std::is_signed_v<Int>;
  static constexpr Int Min = ~Max;
};
template <typename Int>
int_range(Int) -> int_range<Int>;
template <typename Int>
int_range(Int, Int) -> int_range<Int>;

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename Int>
constexpr auto int_range<Int>::iter() const {
  // Deal with a funny special case: we need to yield every integer. This
  // happens when start == min and end/count is not set or including_end is
  // Max.
  if (start == Min && ((including_end && *including_end == Max) ||
                       (!including_end && !end && !count))) {
    return best::iter(
        iter_impl{.start = Min, .count = Int(-1), .plus_one = true});
  }

  return best::iter(iter_impl{.start = start, .count = *normalize(Max).count});
}

template <typename Int>
constexpr int_range<Int> int_range<Int>::wrapping_normalize(
    Int max_size) const {
  if (end) {
    return {.start = start, .count = (best::overflow(*end) - start).wrap()};
  }

  if (including_end) {
    // NOTE: in the case that start = max and including_end = max, this
    // correctly produces a one-element range that yields `max`.
    return {.start = start,
            .count = (best::overflow(*including_end) - start + 1).wrap()};
  }

  if (count) {
    return {.start = start, .count = count};
  }

  return {.start = start, .count = (best::overflow(max_size) - start).wrap()};
}

template <typename Int>
constexpr int_range<Int> int_range<Int>::normalize(opt_t max_size) const {
  if (max_size && start > *max_size) {
    return {.start = start};
  }

  if (end) {
    if (start > *end) {
      return {.start = start};
    }
    return {.start = start, .count = (best::overflow(*end) - start).wrap()};
  }

  if (including_end) {
    // NOTE: in the case that start = max and including_end = max, this
    // correctly produces a one-element range that yields `max`.
    if (start > *including_end) {
      return {.start = start};
    }
    return {.start = start,
            .count = (best::overflow(*including_end) - start + 1).wrap()};
  }

  if (count) {
    if ((best::overflow(start) + *count).overflowed) {
      return {.start = start};
    }
    return {.start = start, .count = count};
  }

  if (!max_size) {
    return {.start = start, .count = Max - start};
  }

  return {.start = start, .count = (best::overflow(*max_size) - start).wrap()};
}

template <typename Int>
constexpr size_t int_range<Int>::compute_count(size_t max_size,
                                               best::location loc) const
  requires std::is_same_v<size_t, Int>
{
  if (auto result = try_compute_count(max_size)) {
    return *result;
  }
  bounds_internal::crash(*this, max_size, loc);
}

template <typename Int>
constexpr int_range<Int>::opt_t int_range<Int>::try_compute_count(
    opt_t max_size) const
  requires std::is_same_v<size_t, Int>
{
  if (!max_size && !end && !including_end && !count) {
    return {};
  }

  auto count = normalize(max_size).count;
  if (!max_size) return count;

  auto [end, of] = (best::overflow(start) + *count);
  if (of || end > *max_size) return {};
  return count;
}

template <typename Int>
constexpr auto begin(const int_range<Int>& b) {
  return b.iter().into_range();
}

template <typename Int>
constexpr auto end(const int_range<Int>&) {
  return best::iter_range_end{};
}

}  // namespace best
#endif  // BEST_ITER_BOUNDS_H_
