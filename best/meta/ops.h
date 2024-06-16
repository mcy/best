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

/// Calls a function.
///
/// This is a highly generic operation: it emulates the behavior of std::invoke,
/// which treats T C::* as a C -> T function; it also allows passing a
/// best::tlist as the first argument, which will pass explicit template
/// parameters to the underlying operator().
///
/// Additionally, any type parameters passed to this function will be forwarded
/// to `call`.
template <typename... TParams>
constexpr auto call(auto &&...args)
    -> decltype(ops_internal::call(ops_internal::tlist<TParams...>{},
                                   BEST_FWD(args)...)) {
  return ops_internal::call(ops_internal::tlist<TParams...>{},
                            BEST_FWD(args)...);
}

/// Returns true if a particular call to best::call is possible.
///
/// Use as `best::callable<F, int(char*, size_t, size_t)>`. This will validate
/// that `F` can be called with the arguments `char*, size_t, size_t`, and that
/// the result is convertible to `int`. If the result type is specified to be
/// a void type, no constraints are placed on F's result type.
///
/// Arguments after the function signature are explicit template parameters for
/// operator().
template <typename F, typename Signature, typename... TParams>
concept callable = ops_internal::can_call<F>(ops_internal::tlist<TParams...>{},
                                             (Signature *)nullptr);

/// Returns the result of calling F with a single argument.
///
/// If Arg is a void type, it instead returns the result of calling F with no
/// arguments.
///
/// This is of primary value when computing the result of calling a function on
/// an alternative of a best::choice, which can be simply "void".
template <typename F, typename Arg>
using call_result_with_void =
    decltype([](auto &&f, auto &&...args) -> decltype(auto) {
      if constexpr (std::is_void_v<Arg>) {
        return best::call(BEST_FWD(f));
      } else {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      }
    }(std::declval<F>(), std::declval<std::conditional_t<std::is_void_v<Arg>,
                                                         int, Arg>>()));

/// Calls a function by folding.
///
/// For example, when args has three elements, this expands into
///
///   call<TParams...>(
///     func,
///     call<TParams...>(
///       func,
///       call<TParams...>(
///         func,
///         init,
///         args[0]),
///       arg[1]),
///     arg[2])
///
/// This is performed recursively.
template <typename... TParams>
constexpr auto fold_call(auto &&init, auto &&func, auto &&first, auto &&...args)
    -> decltype(ops_internal::call(BEST_FWD(args)...)) {
  best::call<TParams...>(
      BEST_FWD(func),
      best::fold_call(BEST_FWD(init), BEST_FWD(func), BEST_FWD(args)...),
      BEST_FWD(first));
}
template <typename... TParams>
constexpr decltype(auto) call_reduce(auto &&init, auto &&func) {
  return BEST_FWD(init);
}

/// Constructs a generic lambda that will call the function named by
/// `path`.
///
/// This is useful for capturing a "function pointer" to an overloaded
/// function.
#define BEST_CALLABLE(path_) \
  [](auto &&...args) -> decltype(auto) { return path_(BEST_FWD(args)...); }

}  // namespace best

#endif  // BEST_META_OPS_H_