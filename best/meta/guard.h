#ifndef BEST_META_GUARD_H_
#define BEST_META_GUARD_H_

#include "best/meta/tags.h"

//! A middling approximation of Rust's `?` operator.
//!
//! Guardable types are those which can be passed into `BEST_GUARD()`, which
//! is an early-return construct.

namespace best {
/// # `best::guardable`
///
/// A type that you can use `BEST_GUARD()` with. This is any type that is
/// contextually convertible to bool and provides the FTADLEs below.
///
/// Guardable types can be used with `BEST_GUARD()`.
template <typename R>
concept guardable = requires(best::ftadle f, const R& r) {
  static_cast<bool>(r);
  BestGuardResidual(f, r);
  BestGuardReturn(f, r, BestGuardResidual(f, r));
};

/// # `BEST_GUARD()`
///
/// If `expr_` is a guardable type, and it is "false" (i.e.,
/// `static_cast<bool>(expr_)` is `false), this returns out of the current
/// function with the "residual" produced by calling the FTADLEs
/// `BestGuardResidual()` and `BestGuardReturn()`.
///
/// For example, calling `BEST_GUARD()` on a `best::option` will return a
/// `best::none`. When guarding a `best::result`, it returns a corresponding
/// `best::err`.
///
/// Any postfix calls are applied to the contents of the residual before it
/// is wrapped up for return. This enables code such as
///
/// ```
/// best::result<int, error> r = ...;
/// BEST_GUARD(r).frobbincate();
/// ```
///
/// The `.frobbnicate()` function will be called on the `error` value, not the
/// intermediate `best::err<error>`.
#define BEST_GUARD(expr_) \
  for (::best::guard_internal::impl g_{expr_}; !g_;) return g_, g_.residual()

namespace guard_internal {
template <guardable R>
struct impl {
  constexpr decltype(auto) residual() {
    return BestGuardResidual(best::ftadle{}, BEST_FWD(result));
  }
  constexpr decltype(auto) operator,(auto&& residual) {
    return BestGuardReturn(best::ftadle{}, BEST_FWD(result), residual);
  }
  constexpr explicit operator bool() { return static_cast<bool>(result); }

  R result;
};
template <typename R>
impl(R&&) -> impl<R&&>;
}  // namespace guard_internal
}  // namespace best
#endif  // BEST_META_GUARD_H_