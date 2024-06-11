#ifndef BEST_MEMORY_ALLOCATOR_H_
#define BEST_MEMORY_ALLOCATOR_H_

#include <cstddef>
#include <cstdlib>

#include "best/memory/layout.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"

//! Low level allocator abstractions.
//!
//! This header provides analogues of Rust's std::alloc module. It does not
//! provide any compatibility whatsoever with the STL's allocator support,
//! because it is intended to purely handle memory allocation.

namespace best {
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
    requires(A& alloc, best::layout layout, void* ptr) {
      /// Allocates fresh memory. Returns a non-null pointer to it.
      ///
      /// Crashes on allocation failure.
      { alloc.alloc(layout) } -> std::same_as<void*>;

      /// Allocates fresh zeroed memory. Returns a non-null pointer to it.
      ///
      /// Crashes on allocation failure.
      { alloc.zalloc(layout) } -> std::same_as<void*>;

      /// Resizes memory previously allocated with this allocator.
      /// Returns a non-null pointer to it.
      ///
      /// The second argument is the original layout it was allocated with, the
      /// third is the desired layout.
      ///
      /// Crashes on allocation failure.
      { alloc.realloc(ptr, layout, layout) } -> std::same_as<void*>;

      /// Deallocated memory previously allocated with this allocator.
      /// Returns a non-null pointer to it.
      ///
      /// The second argument is the original layout it was allocated with.
      ///
      /// Crashes on allocation failure.
      { alloc.dealloc(ptr, layout) };
    };

/// The global allocator.
///
/// Note that this calls into malloc(), not ::operator new, in order to permit
/// resizing-in-place.
///
/// See best::allocator for information on what the functions on this type do.
struct malloc final {
  static void* alloc(layout layout);
  static void* zalloc(layout layout);
  static void* realloc(void* ptr, layout old, layout layout);
  static void dealloc(void* ptr, layout layout);

  constexpr bool operator==(const malloc&) const = default;
};
static_assert(best::allocator<best::malloc>);
}  // namespace best

#endif  // BEST_MEMORY_ALLOCATOR_H_