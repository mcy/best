#ifndef BEST_BASE_PORT_H_
#define BEST_BASE_PORT_H_

//! Miscellaneous helper/portability macros.

#include <ios>
#include <utility>
namespace best {
/// Returns whether this program should have debug assertions enabled.
inline constexpr bool is_debug() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

/// Stringifies a token string.
#define BEST_STRINGIFY(...) BEST_STRINGIFY_(__VA_ARGS__)
#define BEST_STRINGIFY_(...) #__VA_ARGS__

/// Helper macro for generating more readable pragma definitions.
#define BEST_PRAGMA(...) _Pragma(BEST_STRINGIFY(__VA_ARGS__))

/// Macro implementation of std::forward.
///
/// Intended to improve compile times and gdb debugging by eliminating an
/// extremely common function that must be inlined.
#define BEST_FWD(expr_) (static_cast<decltype(expr_)&&>(expr_))

/// Marks a symbol as weak.
///
/// When the linker is resolving a symbol X, and multiple definitions of X
/// exist, it is undefined behavior (in practice, the linker either produces
/// an error or picks one arbitrarily). However, if all definitions of X are
/// weak except for one, that one non-weak definition will be chosen.
///
/// This can be used to implement symbol overrides.
#define BEST_WEAK [[gnu::weak]]

/// Marks a type as trivially relocatable.
///
/// This ensures that the type can be passed "in registers" when possible.
/// However, this is only valid when the type is semantically "trivially
/// relocatable", i.e., if moving from a value, and then destroying that value,
/// is a no-op, and the move operation itself is otherwise trivial.
#define BEST_RELOCATABLE [[clang::trivial_abi]]

/// Marks a function with strong inlining hints.
///
/// When always inlined, the compiler will try its best to inline the function
/// at all optimization levels. When never inlined, the compiler will try to
/// avoid inlining it. The compiler is free to disregard either.
#define BEST_INLINE_ALWAYS [[gnu::always_inline]] inline
#define BEST_INLINE_NEVER [[gnu::noinline]]

/// Like BEST_INLINE_ALWAYS, but the marked function will not appear in stack
/// traces.
#define BEST_INLINE_SYNTHETIC [[gnu::always_inline]] [[gnu::artificial]] inline

/// Tests whether a particular GCC-like builtin is available.
#ifndef __has_builtin
#define BEST_HAS_BUILTIN(x_) 0
#else
#define BEST_HAS_BUILTIN(x_) __has_builtin(x_)
#endif

/// Tests whether a particular GCC-like attribute is available.
#ifndef __has_attribute
#define BEST_HAS_ATTRIBUTE(x_) 0
#else
#define BEST_HAS_ATTRIBUTE(x_) __has_attribute(x_)
#endif

/// Informs the compiler that something can be assumed to be true.
///
/// If `truth` is not true at runtime, undefined behavior.
BEST_INLINE_ALWAYS constexpr void assume(bool truth) {
#if BEST_HAS_BUILTIN(__builtin_assume)
  __builtin_assume(truth);
#else
  (void)truth;
#endif
}

/// Marks a value as likely to be true.
[[nodiscard]] BEST_INLINE_SYNTHETIC constexpr bool likely(bool truthy) {
#if BEST_HAS_BUILTIN(__builtin_expect)
  return __builtin_expect(truthy, true);
#else
  return truthy;
#endif
}

/// Marks a value as likely to be false.
[[nodiscard]] BEST_INLINE_SYNTHETIC constexpr bool unlikely(bool falsey) {
#if BEST_HAS_BUILTIN(__builtin_expect)
  return __builtin_expect(falsey, false);
#else
  return falsey;
#endif
}

/// Hides a value from the compiler's optimizer.
[[nodiscard]] BEST_INLINE_SYNTHETIC auto&& black_box(auto&& value) {
  asm volatile("" : "+r"(value));
  return BEST_FWD(value);
}

#if BEST_HAS_ATTRIBUTE(enable_if)
#define BEST_HAS_ENABLE_IF 1

/// Marks a symbol as "conditionally enabled".
///
/// This is a GCC version of SFINAE that has one nice property: you can pass
/// function arguments into it and evaluate functions on them.
#define BEST_ENABLE_IF(expr_, why_) __attribute__((enable_if(expr_, why_)))

/// Like BEST_ENABLE_IF, but merely requires `expr_` to be constexpr.
#define BEST_ENABLE_IF_CONSTEXPR(expr_) \
  __attribute__((                       \
      enable_if(__builtin_constant_p(expr_), "expected a constexpr value")))

#else
#define BEST_HAS_ENABLE_IF 0
#define BEST_ENABLE_IF(...)
#define BEST_ENABLE_IF_CONSTEXPR(...)
#endif  // BEST_HAS_ATTRIBUTE(enable_if)

/// Helpers for disabling diagnostics.
#define BEST_PUSH_GCC_DIAGNOSTIC() BEST_PRAGMA(GCC diagnostic push)
#define BEST_POP_GCC_DIAGNOSTIC() BEST_PRAGMA(GCC diagnostic push)
#define BEST_IGNORE_GCC_DIAGNOSTIC(W_) BEST_PRAGMA(GCC diagnostic ignored W_)

// HACK: Wait for BestFmt.
template <typename Os, typename A, typename B>
Os& operator<<(Os& os, const std::pair<A, B>& pair) {
  return os << "(" << pair.first << ", " << pair.second << ")";
}
template <typename Os>
Os& operator<<(Os& os, std::byte b) {
  return os << "0x" << std::hex << int(b);
}

}  // namespace best

#endif  // BEST_BASE_PORT_H_