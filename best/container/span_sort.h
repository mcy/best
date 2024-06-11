#ifndef BEST_CONTAINER_SPAN_SORT_H_
#define BEST_CONTAINER_SPAN_SORT_H_

#include <algorithm>

#include "best/container/span.h"
#include "best/meta/concepts.h"

//! Implementations for best::span::sort and friends.
//!
//! Include this file if you need to sort.

namespace best {
template <best::object_type T, best::option<size_t> n>
void span<T, n>::sort() const
  requires best::comparable<T> && (!is_const)
{
  std::sort(data().raw(), data().raw() + size());
}
template <best::object_type T, best::option<size_t> n>
void span<T, n>::sort(best::callable<void(const T&)> auto&& get_key) const
  requires(!is_const)
{
  std::sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a) < best::call(BEST_FWD(get_key), b);
  });
}
template <best::object_type T, best::option<size_t> n>
void span<T, n>::sort(
    best::callable<std::partial_ordering(const T&, const T&)> auto&& get_key)
    const
  requires(!is_const)
{
  std::sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a, b) < 0;
  });
}
template <best::object_type T, best::option<size_t> n>
void span<T, n>::stable_sort() const
  requires best::comparable<T> && (!is_const)
{
  std::stable_sort(data().raw(), data().raw() + size());
}
template <best::object_type T, best::option<size_t> n>
void span<T, n>::stable_sort(
    best::callable<void(const T&)> auto&& get_key) const
  requires(!is_const)
{
  std::stable_sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a) < best::call(BEST_FWD(get_key), b);
  });
}
template <best::object_type T, best::option<size_t> n>
void span<T, n>::stable_sort(
    best::callable<std::partial_ordering(const T&, const T&)> auto&& get_key)
    const
  requires(!is_const)
{
  std::stable_sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a, b) < 0;
  });
}

/// # best::mark_sort_header_used()
///
/// This header sometimes doesn't correctly show up as "used" for the purposes
/// of some diagnostics; call this function to silence that.
inline constexpr int mark_sort_header_used() { return 0; }
}  // namespace best

#endif  // BEST_CONTAINER_SPAN_SORT_H_
