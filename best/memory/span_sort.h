/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

#ifndef BEST_MEMORY_SPAN_SORT_H_
#define BEST_MEMORY_SPAN_SORT_H_

#include <algorithm>

#include "best/memory/span.h"

//! Implementations for best::span::sort and friends.
//!
//! Include this file if you need to sort.

namespace best {
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::sort() const
  requires best::comparable<T> && (!is_const)
{
  std::sort(data().raw(), data().raw() + size());
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::sort(
  best::callable<void(const T&)> auto&& get_key) const requires (!is_const)
{
  std::sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a) < best::call(BEST_FWD(get_key), b);
  });
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::sort(
  best::callable<best::partial_ord(const T&, const T&)> auto&& get_key) const
  requires (!is_const)
{
  std::sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a, b) < 0;
  });
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::stable_sort() const
  requires best::comparable<T> && (!is_const)
{
  std::stable_sort(data().raw(), data().raw() + size());
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::stable_sort(
  best::callable<void(const T&)> auto&& get_key) const requires (!is_const)
{
  std::stable_sort(data().raw(), data().raw() + size(), [&](auto& a, auto& b) {
    return best::call(BEST_FWD(get_key), a) < best::call(BEST_FWD(get_key), b);
  });
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::stable_sort(
  best::callable<best::partial_ord(const T&, const T&)> auto&& get_key) const
  requires (!is_const)
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

#endif  // BEST_MEMORY_SPAN_SORT_H_
