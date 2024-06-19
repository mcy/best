#ifndef BEST_BASE_ORD_H_
#define BEST_BASE_ORD_H_

#include <compare>

#include "best/base/internal/ord.h"
#include "best/func/call.h"
#include "best/meta/init.h"
#include "best/meta/taxonomy.h"
#include "best/meta/traits.h"

//! Ordering types.
//!
//! In an ideal universe, we could throw `<compare>` in the trash.
//! Unfortunately, C++ has baked the names of its types into the language, so we
//! have to work with it.
//!
//! As a compromise, we provide this header that offers shorthand names for all
//! the relevant types and values.

namespace best {
/// # `best::ord`
///
/// An alias for `std::strong_ordering`. This is for types where exactly one
/// ordering result `a < b`, `a == b`, and `a > b` is possible. `best`
/// explicitly rejects `best::weak_ordering`.
using ord = ::std::strong_ordering;

/// # `best::partial_ord`
///
/// An alias for `std::partial_ordering`. This is for types where at most one
/// ordering result `a < b`, `a == b`, and `a > b` is possible.
using partial_ord = ::std::partial_ordering;

/// # `best::Less`, `best::Equal`, `best::Greater`, `best::Unordered`
///
/// Values which convert to the appropriate values of `best::ord` and
/// `best::partial_ord`.
inline constexpr auto Less = best::ord_internal::le{};
inline constexpr auto Equal = best::ord_internal::eq{};
inline constexpr auto Greater = best::ord_internal::gt{};
inline constexpr auto Unordered = best::ord_internal::uo{};

/// # `best::common_ord`
///
/// Computes the common order category among a set of types. In addition to
/// `std::
template <typename... Ts>
using common_ord = ord_internal::common<Ts...>;

/// # `best::testable`
///
/// Whether `T` is "`bool` enough" to jam into `if` conditions, `&&`, and `||`.
///
/// The standard calls this "boolean-testable".
template <typename T>
concept testable = best::converts_to<T, bool> && requires(T cond) {
  { !BEST_FWD(cond) } -> best::converts_to<bool>;
};

/// # `best::equatable`
///
/// Whether two types can be compared for equality.
///
/// Somewhat different from C++'s equivalent concept: this allows for both
/// types to be void, and does not require either to be self-comparable.
template <typename T, typename U = T>
concept equatable =
    (best::is_void<T> && best::is_void<U>) || requires(const T& a, const U& b) {
      { a == b } -> best::testable;
      { b == a } -> best::testable;
      { a != b } -> best::testable;
      { b != a } -> best::testable;
    };

/// # `best::comparable`
///
/// Whether two types can be compared for ordering.
///
/// Somewhat different from C++'s equivalent concept: this allows for both
/// types to be void, and does not require either to be self-comparable.
template <typename T, typename U = T>
concept comparable =
    (best::is_void<T> && best::is_void<U>) || requires(const T& a, const U& b) {
      requires best::equatable<T, U>;
      { a <=> b } -> best::converts_to<decltype(a <=> b)>;
      { b <=> a } -> best::converts_to<decltype(b <=> a)>;
    };

/// # `best::order_type<T, U>`
///
/// The ordering type for T and U. If both types are void, returns `best::ord`.
template <typename T, best::comparable<T> U>
using order_type = decltype([](auto x, auto y) {
  if constexpr (best::is_void<T>) {
    return best::ord::equal;
  } else {
    return *x <=> *y;
  }
}(best::lie<best::as_ptr<const T>>, best::lie<best::as_ptr<const U>>));

/// # `best::equal()`
///
/// Compares two values for equality. It does so by performing the following
/// algorithm:
///
/// * If `best::equatable<a, b>` is true, returns `a == b`.
/// * If both `a` and `b` are any pointer type, they are cast to `void*` and
///   then compared.
/// * In all other cases, returns `false`.
constexpr bool equal(const auto& a, const auto& b) {
  if constexpr (best::equatable<decltype(a), decltype(b)>) {
    return a == b;
  } else if constexpr (best::is_ptr<best::as_auto<decltype(a)>> &&
                       best::is_ptr<best::as_auto<decltype(b)>>) {
    return (const volatile void*)a == (const volatile void*)b;
  } else {
    return false;
  }
}

/// # `best::compare()`
///
/// Compares two values for ordering. It does so by performing the following
/// algorithm:
///
/// * If `best::comparable<a, b>` is true, returns `a <=> b`.
/// * If both `a` and `b` are any pointer type, they are cast to `void*` and
///   then compared.
/// * If both `a` and `b` are `best::equatable` and can be compared with `<`,
///   performs a fallback comparison.
/// * In all other cases, returns `best::Unordered`.
constexpr auto compare(const auto& a, const auto& b) {
  if constexpr (best::comparable<decltype(a), decltype(b)>) {
    return a <=> b;
  } else if constexpr (best::is_ptr<best::as_auto<decltype(a)>> &&
                       best::is_ptr<best::as_auto<decltype(b)>>) {
    return std::compare_three_way{}((const volatile void*)a,
                                    (const volatile void*)b);
  } else if constexpr (best::equatable<decltype(a), decltype(b)> && requires {
                         { a < b } -> best::testable;
                         { b < a } -> best::testable;
                       }) {
    return a == b  ? best::partial_ord::equivalent
           : a < b ? best::partial_ord::less
           : b < a ? best::partial_ord::greater
                   : best::partial_ord::unordered;
  } else {
    return best::partial_ord::unordered;
  }
}

/// # best::or_cmp()
///
/// Given a function, returns an object that can be chained to a C++ ordering
/// type that is only called if that ordering type represents equality.
///
/// That is, to perform a lexicographic comparison, you could write
///
/// ```
/// (x <=> y)->*best::or_cmp([] { ... });
/// ```
///
/// The type on the LHS of the chain operator `->*` can be any type that can be
/// compared for `==` with the literal `0`.
constexpr auto or_cmp(best::callable<void()> auto&& cb) {
  return best::ord_internal::chain(cb);
}

inline void BestFmt(auto& fmt, best::partial_ord x) {
  if (x == 0) {
    fmt.write("Equal");
  } else if (x < 0) {
    fmt.write("Less");
  } else if (x > 0) {
    fmt.write("Greater");
  } else {
    fmt.write("Unordered");
  }
}
}  // namespace best

#endif  // BEST_BASE_UNSAFE_H_