#ifndef BEST_BASE_UNSAFE_H_
#define BEST_BASE_UNSAFE_H_

//! Unsafe operation tracking.
//!
//! This header provides `best::unsafe`, a tag type for specifying that an
//! operation has non-trivial preconditions. A function whose first argument is
//! `best::unsafe` is an unsafe function.

namespace best {
/// # `best::unsafe`
///
/// The unsafe tag type. This value is used for tagging functions that have
/// non-trivial preconditions. This is not exactly the same as `unsafe` in Rust;
/// many operations in `best` and C++ at large cannot have such a check.
/// Instead, it primarily exists for providing unsafe overloads of functions
/// that skip safety checks.
///
/// ```
/// int evil(best::unsafe, int);
///
/// int x = evil(best::unsafe("I checked the preconditions"), 42);
/// ```
struct unsafe final {
  /// # `unsafe::unsafe()`
  ///
  /// Constructs a new `unsafe`; the user must provide justification for doing
  /// so, usually in the form of a string literal.
  constexpr unsafe(auto&& why) {}

 private:
  unsafe() = default;
};
}  // namespace best

#endif  // BEST_BASE_UNSAFE_H_