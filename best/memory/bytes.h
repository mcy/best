#ifndef BEST_MEMORY_BYTES_H_
#define BEST_MEMORY_BYTES_H_

#include "best/base/hint.h"
#include "best/container/option.h"
#include "best/memory/internal/bytes.h"

//! Raw byte manipulation functions.
//!
//! This header provides convenient and type-safe versions of the `mem*()`
//! functions.

namespace best {
/// # `best::byte_comparable`
///
/// Whether a pair of types' equality is modeled by `memcmp()`.
template <typename T, typename U = T>
concept byte_comparable =
    (std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>) &&
    (std::is_integral_v<U> || std::is_enum_v<U> || std::is_pointer_v<U>) &&
    sizeof(T) == sizeof(U);

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
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS bool equate_bytes(best::span<T> lhs, best::span<U> rhs)
  requires(sizeof(T) == sizeof(U))
{
  if (lhs.size() != rhs.size()) return false;
  if (lhs.data() == rhs.data() || lhs.is_empty()) return true;

  return 0 ==
         bytes_internal::memcmp(lhs.data(), rhs.data(), lhs.size() * sizeof(T));
}

/// # `best::compare_bytes()`
///
/// A typed wrapper over `memcmp()`. This performs total lexicographic
/// comparison between two spans.
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS best::ord compare_bytes(best::span<T> lhs, best::span<U> rhs)
  requires(sizeof(T) == sizeof(U))
{
  if (lhs.data() == rhs.data() || lhs.is_empty() || rhs.is_empty()) {
    return lhs.size() <=> rhs.size();
  }

  auto to_compare = best::min(lhs.size(), rhs.size()) * sizeof(T);
  int result = bytes_internal::memcmp(lhs.data(), rhs.data(), to_compare);
  if (result == 0) {
    return lhs.size() <=> rhs.size();
  }
  return result <=> 0;
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
template <typename T, typename U = const T>
BEST_INLINE_ALWAYS best::option<size_t> search_bytes(best::span<T> haystack,
                                                     best::span<U> needle)
  requires(sizeof(T) == sizeof(U))
{
  auto* data =
      reinterpret_cast<const char*>(static_cast<const void*>(haystack.data()));
  size_t size = haystack.size() * sizeof(T);
  size_t travel = 0;

  do {
    void* found = bytes_internal::memmem(data, size, needle.data(),
                                         needle.size() * sizeof(T));
    if (!found) return best::none;

    size_t offset = static_cast<const char*>(found) - data;
    travel += offset;
    data += offset;
    size -= offset;
  } while (travel % alignof(T) != 0);

  return travel / sizeof(T);
}
}  // namespace best

#endif  // BEST_MEMORY_BYTES_H_