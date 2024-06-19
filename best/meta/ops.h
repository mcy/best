#ifndef BEST_META_OPS_H_
#define BEST_META_OPS_H_

#include <stddef.h>

#include <compare>
#include <concepts>
#include <functional>
#include <utility>

#include "best/meta/internal/ops.h"

//! Helpers for working with overloadable operators.

namespace best {
/// An overloadable operator.
enum class op {
  Add,   // a + b
  Sub,   // a - b
  Mul,   // a * b
  Div,   // a / b
  Rem,   // a % b
  Plus,  // +a
  Neg,   // -a

  AndAnd,  // a && b
  OrOr,    // a || b
  Not,     // !a

  And,   // a & b
  Or,    // a | b
  Xor,   // a ^ b
  Shl,   // a << b
  Shr,   // a >> b
  Cmpl,  // ~a

  Deref,      // *a
  AddrOf,     // &a
  Arrow,      // a->m
  ArrowStar,  // a->*m

  Eq,  // a == b
  Ne,  // a != b
  Lt,  // a < b
  Le,  // a <= b
  Gt,  // a > b
  Ge,  // a >= b

  Spaceship,  // a <=> b
  Comma,      // a, b

  Call,   // a(b)
  Index,  // a[b]

  Assign,     // a = b
  AddAssign,  // a += b
  SubAssign,  // a -= b
  MulAssign,  // a *= b
  DivAssign,  // a /= b
  RemAssign,  // a %= b
  AndAssign,  // a &= b
  OrAssign,   // a |= b
  XorAssign,  // a ^= b
  ShlAssign,  // a <<= b
  ShrAssign,  // a >>= b

  PreInc,   // ++a
  PostInc,  // a++
  PreDec,   // --a
  PostDec,  // a--
};

/// Executes an overloadable operator on the given arguments.
///
/// When args has three or more elements, this will perform a fold of the form
/// (... op args), if the operation supports folding.
template <best::op op>  // clang-format off
constexpr auto operate(auto &&...args) -> decltype(ops_internal::run<best::op>(ops_internal::tag<op>{}, BEST_FWD(args)...)) {
  return ops_internal::run<best::op>(ops_internal::tag<op>{}, BEST_FWD(args)...);
}  // clang-format on

/// Infers the result type of `best::operate<op>(Args...)`.
template <best::op op, typename... Args>
using op_output = decltype(best::operate<op>(std::declval<Args>()...));

/// Whether best::operate<op>(args...) is well-formed.
template <best::op op, typename... Args>
concept has_op = requires(Args &&...args) {
  { best::operate<op>(BEST_FWD(args)...) };
};

/// Whether best::operate<op>(args...) is well-formed and has output converting
/// to R.
template <best::op op, typename R, typename... Args>
concept has_op_r = requires(Args &&...args) {
  { best::operate<op>(BEST_FWD(args)...) } -> std::convertible_to<R>;
};

/// Whether two types can be compared for equality.
///
/// Somewhat different from C++'s equivalent concepts; this only checks for
/// bool-producing operator== and operator!=, and allows for void types to be
/// compared with each other.
template <typename T, typename U = T>
concept equatable = (std::is_void_v<T> && std::is_void_v<U>) ||
                    (best::has_op_r<op::Eq, bool, const T &, const U &> &&
                     best::has_op_r<op::Ne, bool, const T &, const U &>);

/// Whether two types can be compared for ordering.
///
/// Somewhat different from C++'s equivalent concepts; this only checks for <=>
/// comparisons, and allows for void types to be compared with each other.
template <typename T, typename U = T>
concept comparable = (std::is_void_v<T> && std::is_void_v<U>) ||
                     (best::equatable<T, U> &&
                      best::has_op<op::Spaceship, const T &, const U &>);

/// The ordering type for T and U.
///
/// If both types are void, returns std::strong_ordering.
template <typename T, comparable<T> U>
using order_type = decltype([] {
  auto a = std::declval<std::add_pointer_t<T>>();
  auto b = std::declval<std::add_pointer_t<U>>();
  if constexpr (std::is_void_v<T>) {
    return std::strong_ordering::equal;
  } else {
    return *a <=> *b;
  }
}());

/// Compares any two pointers for address equality.
inline constexpr auto addr_eq(const volatile void *a, const volatile void *b) {
  return a == b;
}

/// Compares any two pointers for address order.
inline constexpr std::strong_ordering addr_cmp(const volatile void *a,
                                               const volatile void *b) {
  if (a == b) {
    return std::strong_ordering::equal;
  }
  if (std::less<decltype(a)>{}(a, b)) {
    return std::strong_ordering::less;
  }
  return std::strong_ordering::greater;
}
}  // namespace best

#endif  // BEST_META_OPS_H_