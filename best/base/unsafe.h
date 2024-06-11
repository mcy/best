#ifndef BEST_BASE_UNSAFE_H_
#define BEST_BASE_UNSAFE_H_

#include <stddef.h>

#include "best/base/port.h"

//! Unsafe operation tracking.
//!
//! This header provides `best::unsafe`, a tag type for specifying that an
//! operation has non-trivial preconditions. A function whose first argument is
//! `best::unsafe` is an unsafe function.

namespace best {
/// # `best::unsafe`
///
/// The unsafe tag type. This value cannot be constructed directly; instead,
/// its scope is limited to an unsafe block:
///
/// ```
/// int evil(best::unsafe, int);
///
/// int x = best::unsafe::in([](auto u) {
///   return evil(u, 42);
/// });
/// ```
///
/// Although it is possible to escape the `unsafe` tag out of the block, this
/// is relatively difficult to do accidentally, which helps limit the blast
/// radius of an unsafe block.
struct unsafe final {
  /// # `unsafe::in()`
  ///
  /// Executes an unsafe block.
  BEST_INLINE_SYNTHETIC static constexpr decltype(auto) in(auto&& block,
                                                           auto&&... args) {
    return BEST_FWD(block)(unsafe{}, BEST_FWD(args)...);
  }

 private:
  unsafe() = default;
};
}  // namespace best

#endif  // BEST_BASE_UNSAFE_H_