#ifndef BEST_MEMORY_BYTES_H_
#define BEST_MEMORY_BYTES_H_

#include <type_traits>

#include "best/base/hint.h"
#include "best/container/option.h"
#include "best/memory/internal/bytes.h"

//! Raw byte manipulation functions.
//!
//! This header provides convenient and type-safe versions of the `mem*()`
//! functions.

namespace best {
/// # `BEST_CONSTEXPR_MEMCMP`
///
/// Whether constexpr `memcmp()` is known to be available.
#define BEST_CONSTEXPR_MEMCMP BEST_CONSTEXPR_MEMCMP_

/// # `best::byte_comparable`
///
/// Whether a pair of types' equality is modeled by `memcmp()`.
template <typename T, typename U = T>
concept byte_comparable = requires {
  best::bytes_internal::can_memcmp(best::bytes_internal::tag<T>{},
                                   best::bytes_internal::tag<U>{});
};

/// # `best::constexpr_byte_comparable`
///
/// Whether a pair of types can be `memcmp`'d in constexpr.
template <typename T, typename U = T>
concept constexpr_byte_comparable = requires {
  requires byte_comparable<T, U>;
  requires bytes_internal::is_char<T> && bytes_internal::is_char<U>;
  requires bool(BEST_CONSTEXPR_MEMCMP);
};

/// # `best::copy_bytes()`
///
/// A typed wrapper over `memcpy()`. This will copy the largest common prefix of
/// the spans.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS void copy_bytes(best::span<T> dst, best::span<U> src)
  requires(sizeof(T) == sizeof(U))
{
  auto to_copy = best::min(dst.size(), src.size()) * sizeof(T);
  if (to_copy == 0) return;
  bytes_internal::memcpy(dst.data(), src.data(), to_copy);
}

/// # `best::copy_overlapping_bytes()`
///
/// A typed wrapper over `memmove()`. This will copy the largest common prefix
/// of the spans.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS void copy_overlapping_bytes(best::span<T> dst,
                                               best::span<U> src)
  requires(sizeof(T) == sizeof(U))
{
  auto to_copy = best::min(dst.size(), src.size()) * sizeof(T);
  if (to_copy == 0) return;
  bytes_internal::memmove(dst.data(), src.data(), to_copy);
}

/// # `best::fill_bytes()`
///
/// A typed wrapper over `memset()`.
template <typename T>
BEST_INLINE_ALWAYS void fill_bytes(best::span<T> dst, uint8_t fill) {
  if (dst.is_empty()) return;
  bytes_internal::memset(dst.data(), fill, dst.size() * sizeof(T));
}

/// # `best::compare_bytes()`
///
/// A typed wrapper over `memcmp()` that is optimized for performing equality
/// comparisons.
///
/// If T and U are both `char`, this function is constexpr.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS constexpr bool equate_bytes(best::span<T> lhs,
                                               best::span<U> rhs)
  requires best::byte_comparable<T, U>
{
  return bytes_internal::equate(lhs, rhs);
}

/// # `best::compare_bytes()`
///
/// A typed wrapper over `memcmp()`. This performs total lexicographic
/// comparison between two spans.
///
/// If T and U are both `char`, this function is constexpr.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS constexpr best::ord compare_bytes(best::span<T> lhs,
                                                     best::span<U> rhs)
  requires best::byte_comparable<T, U>
{
  return bytes_internal::compare(lhs, rhs);
}

/// # `best::search_bytes()`
///
/// A typed wrapper over `memmem()`, a GNU extension that is the `mem` version
/// of `strstr()`. This finds the index of the first occurrence of `needle` in
/// `haystack`.
///
/// Note that this will find the first *aligned* index, which is to say that
/// calls to `memmem()` will continue until it either fails or returns an
/// aligned index.
///
/// If T and U are both `char`, this function is constexpr.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS constexpr best::option<size_t> search_bytes(
    best::span<T> haystack, best::span<U> needle)
  requires best::byte_comparable<T, U>
{
  if (!std::is_constant_evaluated()) {
    return bytes_internal::search(haystack, needle);
  } else if constexpr (best::constexpr_byte_comparable<T, U>) {
    return bytes_internal::search_constexpr(haystack, needle);
  } else {
    best::crash_internal::crash(
        "cannot call best::search_bytes() in constexpr for this type");
  }
}
}  // namespace best

#endif  // BEST_MEMORY_BYTES_H_