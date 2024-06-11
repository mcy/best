#ifndef BEST_MATH_OVERFLOW_H_
#define BEST_MATH_OVERFLOW_H_

#include <stddef.h>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/math/int.h"
#include "best/meta/ops.h"

//! Overflow-detection utilities.

namespace best {
template <integer>
struct overflow;

/// The result of calling best::div: a quotient and a remainder.
template <integer Int>
struct div_t {
  overflow<Int> quot, rem;
};

/// Simultaneously computes the quotient and remainder.
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr best::div_t<common_int<A, B>> div(
    overflow<A> a, overflow<B> b, best::location loc = best::here) {
  if (best::unlikely(b.value == 0)) {
    best::crash_internal::crash({"division by zero", loc});
  }

  using C = common_int<A, B>;
  C ca = a.value, cb = b.value;

  if (best::signed_int<C> && best::unlikely(ca == min_of<C> && cb == -1)) {
    return {{min_of<C>, true}, {min_of<C>, true}};
  }

  return {{ca / cb, a.overflowed || b.overflowed},
          {ca % cb, a.overflowed || b.overflowed}};
}
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr best::div_t<common_int<A, B>> div(
    A a, B b, best::location loc = best::here) {
  return best::div(overflow(a), overflow(b));
}

/// Computes ceil(a / b).
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr overflow<best::common_int<A, B>> ceildiv(
    overflow<A> a, overflow<B> b) {
  auto [q, r] = best::div(a, b);
  if ((r.value > 0 && b.value > 0) || (r.value < 0 && b.value < 0)) {
    return q + 1;
  }
  return q;
}
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr overflow<best::common_int<A, B>> ceildiv(A a,
                                                                      B b) {
  return best::ceildiv(overflow(a), overflow(b));
}

/// A wrapper over an integer type that records whether overflow occurs.
///
/// This type is convertible from Int, and offers all of the standard integer
/// operations: +. -. *, /, %, ~, &, |, ^, <<, and >>.
///
/// +, -, and * have the usual definition of overflow. / and % underflow only
/// when computing best::div(best::min_of<Int>, -1). << and >> overflow when the
/// shift amount is negative or than or equal to best::bits<Int>.
template <integer Int>
struct overflow {
  // Fields are public for structured bindings.
  Int value = 0;
  bool overflowed = false;

  constexpr overflow() = default;
  constexpr overflow(const overflow&) = default;
  constexpr overflow& operator=(const overflow&) = default;
  constexpr overflow(overflow&&) = default;
  constexpr overflow& operator=(overflow&&) = default;

  constexpr overflow(Int value, bool overflowed = false)
      : value(value), overflowed(overflowed) {}

  template <typename J>
  constexpr overflow(overflow<J> that)
      : value(that.value),
        overflowed(that.overflowed ||
                   best::int_cmp(best::min_of<Int>, that.value) < 0 ||
                   best::int_cmp(best::max_of<Int>, that.value) > 0) {}

  /// Returns the result as if wrapping arithmetic was used.
  BEST_INLINE_ALWAYS constexpr Int wrap() const { return value; }

  /// Returns the result, but only if it did not overflow.
  BEST_INLINE_ALWAYS constexpr best::option<Int> checked() const {
    if (overflowed) return {};
    return value;
  }

  /// Returns the result, but crashes if overflow occurred.
  BEST_INLINE_ALWAYS constexpr Int strict(
      best::location loc = best::here) const {
    if (best::unlikely(overflowed)) {
      best::crash_internal::crash({"arithmetic overflow", loc});
    }
    return value;
  }

  BEST_INLINE_ALWAYS constexpr friend overflow operator+(overflow a) {
    return a;
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator+(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    C c;
    bool of = __builtin_add_overflow(ca, cb, &c);
    return {c, a.overflowed || b.overflowed || of};
  }

  BEST_INLINE_ALWAYS constexpr friend overflow operator-(overflow a) {
    if (best::signed_int<Int>) {
      return a.value == min_of<Int> ? overflow{a.value, true}
                                    : overflow{-a.value, a.overflowed};
    }

    return {-a.value, a.overflowed || a.value != 0};
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator-(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    C c;
    bool of = __builtin_sub_overflow(ca, cb, &c);
    return {c, a.overflowed || b.overflowed || of};
  }

  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator*(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    C c;
    bool of = __builtin_mul_overflow(ca, cb, &c);
    return {c, a.overflowed || b.overflowed || of};
  }

  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator/(
      best::track_location<overflow> a, overflow<J> b) {
    return best::div(*a, b, a).quot;
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator%(
      best::track_location<overflow> a, overflow<J> b) {
    return best::div(*a, b, a).rem;
  }

  BEST_INLINE_ALWAYS constexpr friend overflow operator~(overflow a) {
    return {~a.value, a.overflowed};
  }

  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator&(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    return {ca & cb, a.overflowed || b.overflowed};
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator|(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    return {ca | cb, a.overflowed || b.overflowed};
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<common_int<Int, J>> operator^(
      overflow a, overflow<J> b) {
    using C = common_int<Int, J>;
    C ca = a.value, cb = b.value;

    return {ca ^ cb, a.overflowed || b.overflowed};
  }

  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<Int> operator<<(overflow a,
                                                               overflow<J> b) {
    auto shamt = b.value & (bits_of<Int> - 1);
    return {a.value << shamt, a.overflowed || b.overflowed || b.value != shamt};
  }
  template <integer J>
  BEST_INLINE_ALWAYS constexpr friend overflow<Int> operator>>(overflow a,
                                                               overflow<J> b) {
    auto shamt = b.value & (bits_of<Int> - 1);
    return {a.value >> shamt, a.overflowed || b.overflowed || b.value != shamt};
  }

#define BEST_OF_BOILERPLATE_(op_, opeq_)                                  \
  BEST_INLINE_ALWAYS constexpr friend auto operator op_(overflow a,       \
                                                        integer auto b) { \
    return a op_ overflow(b);                                             \
  }                                                                       \
  BEST_INLINE_ALWAYS constexpr friend auto operator op_(integer auto a,   \
                                                        overflow b) {     \
    return overflow(a) op_ b;                                             \
  }                                                                       \
                                                                          \
  template <integer J>                                                    \
  BEST_INLINE_ALWAYS constexpr friend overflow& operator opeq_(           \
      overflow & a, overflow<J> b) {                                      \
    return a = a op_ b;                                                   \
  }                                                                       \
  BEST_INLINE_ALWAYS constexpr friend overflow& operator opeq_(           \
      overflow & a, integer auto b) {                                     \
    return a = a op_ b;                                                   \
  }

  BEST_OF_BOILERPLATE_(+, +=)
  BEST_OF_BOILERPLATE_(-, -=)
  BEST_OF_BOILERPLATE_(*, *=)
  BEST_OF_BOILERPLATE_(/, /=)
  BEST_OF_BOILERPLATE_(%, %=)
  BEST_OF_BOILERPLATE_(&, &=)
  BEST_OF_BOILERPLATE_(|, |=)
  BEST_OF_BOILERPLATE_(^, ^=)
  BEST_OF_BOILERPLATE_(<<, <<=)
  BEST_OF_BOILERPLATE_(>>, >>=)

#undef BEST_OF_BOILERPLATE_

  template <integer J>
  BEST_INLINE_ALWAYS constexpr bool operator==(overflow<J> that)
    requires best::equatable<Int, J>
  {
    return value == that.value && overflowed == that.overflowed;
  }

  template <integer J>
  BEST_INLINE_ALWAYS constexpr bool operator==(J that)
    requires best::equatable<Int, J>
  {
    return value == that.value && !overflowed;
  }
};

template <integer Int>
overflow(Int) -> overflow<Int>;

/// Computes a sum, but saturates at the integer boundaries instead of
/// overflowing
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr best::common_int<A, B> saturating_add(A a, B b) {
  using C = common_int<A, B>;
  auto [c, of] = best::overflow(a) + b;
  if (of) {
    // The sign of a determines which direction to saturate in.
    // If C is unsigned, this always produces max_of<C>.
    return (C(a) < 0) ? best::min_of<C> : best::max_of<C>;
  }
  return c;
}

/// Computes a subtraction, but saturates at the integer boundaries instead of
/// overflowing
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr best::common_int<A, B> saturating_sub(A a, B b) {
  using C = common_int<A, B>;
  auto [c, of] = best::overflow(a) - b;
  if (of) {
    // The sign of a determines which direction to saturate in.
    // If C is unsigned, this condition is otherwise always false,
    // but unsigned subtraction can only underflow, so we adjust for that.
    return (C(a) < 0 || best::unsigned_int<C>) ? best::min_of<C>
                                               : best::max_of<C>;
  }
  return c;
}

/// Computes a product, but saturates at the integer boundaries instead of
/// overflowing
template <integer A, integer B>
BEST_INLINE_ALWAYS constexpr best::common_int<A, B> saturating_mul(A a, B b) {
  using C = common_int<A, B>;
  auto [c, of] = best::overflow(a) * b;
  if (of) {
    // Any combination of signs can produce overflow; the sign of the saturation
    // is the xor of the inputs' signs.
    return ((C(a) < 0) ^ (C(b) < 0)) ? best::min_of<C> : best::max_of<C>;
  }
  return c;
}
}  // namespace best

#endif  // BEST_MATH_OVERFLOW_H_