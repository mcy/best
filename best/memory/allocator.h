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

#ifndef BEST_MEMORY_ALLOCATOR_H_
#define BEST_MEMORY_ALLOCATOR_H_

#include <cstddef>
#include <cstdlib>

#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/init.h"

//! Low level allocator abstractions.
//!
//! This header provides analogues of Rust's std::alloc module. It does not
//! provide any compatibility whatsoever with the STL's allocator support,
//! because it is intended to purely handle memory allocation.

namespace best {
/// # `best::allocator`
///
/// An allocator: a source of raw memory.
///
/// Allocator functions need not tolerate a size of zero, and must always
/// produce non-null pointers.
///
/// Allocators must be movable and comparable. If two allocators compare as
/// equal, then pointers allocated with one may be re/deallocated with the
/// other.
template <typename A>
concept allocator =  //
  best::moveable<A> && best::equatable<A, A> &&
  requires(A& alloc, best::layout layout, best::ptr<void> ptr) {
    /// # `allocator::alloc(layout)`
    ///
    /// Allocates fresh memory. Returns a non-null pointer to it.
    /// Crashes on allocation failure.
    { alloc.alloc(layout) } -> std::same_as<best::ptr<void>>;

    /// # `allocator::zalloc(layout)`
    ///
    /// Allocates fresh zeroed memory. Returns a non-null pointer to it.
    /// Crashes on allocation failure.
    { alloc.zalloc(layout) } -> std::same_as<best::ptr<void>>;

    /// # `allocator::realloc(ptr, old, new)`
    ///
    /// Resizes memory previously allocated with this allocator.
    /// Returns a non-null pointer to it.
    ///
    /// The second argument is the original layout it was allocated with, the
    /// third is the desired layout.
    /// Crashes on allocation failure.
    { alloc.realloc(ptr, layout, layout) } -> std::same_as<best::ptr<void>>;

    /// # `allocator::dealloc(ptr, layout)`
    ///
    /// Deallocates memory previously allocated with this allocator.
    /// Returns a non-null pointer to it.
    ///
    /// The second argument is the original layout it was allocated with.
    /// Crashes on allocation failure.
    { alloc.dealloc(ptr, layout) };
  };

/// # `best::malloc`
///
/// The global allocator.
///
/// Note that this calls into `malloc()`, not `::operator new`, in order to
/// permit resizing-in-place (which C++, in TYOL 2024, does not allow?!).
///
/// See `best::allocator` for information on what the functions on this type do.
struct malloc final {
  static best::ptr<void> alloc(layout layout);
  static best::ptr<void> zalloc(layout layout);
  static best::ptr<void> realloc(best::ptr<void> ptr, layout old,
                                 layout layout);
  static void dealloc(best::ptr<void> ptr, layout layout);

  constexpr bool operator==(const malloc&) const = default;
};
static_assert(best::allocator<best::malloc>);
}  // namespace best

#endif  // BEST_MEMORY_ALLOCATOR_H_
