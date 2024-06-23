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

#ifndef BEST_CONTAINER_INTERNAL_PUN_H_
#define BEST_CONTAINER_INTERNAL_PUN_H_

#include "best/base/port.h"
#include "best/container/object.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"
#include "best/meta/tlist.h"

//! Internal implementation of best::pun.

namespace best::pun_internal {
struct info_t {
  bool trivial_default, trivial_copy, trivial_move, trivial_dtor;
};

// This does not use init.h because it is a very heavy hitter for compile-times,
// because every result and option ever instantiate this.
template <typename... Ts>
inline constexpr info_t info = {
    .trivial_default = (best::constructible<Ts, trivially> && ...),
    .trivial_copy = (best::copyable<Ts, trivially> && ...),
    .trivial_move = (best::moveable<Ts, trivially> && ...),
    .trivial_dtor = (best::destructible<Ts, trivially> && ...),
};

// A raw union.
template <const info_t& info, typename... Ts>
union impl;

template <const info_t& info>
union impl<info> {
  constexpr impl() = default;

  constexpr impl(const impl&) = default;
  constexpr impl& operator=(const impl&) = default;
  constexpr impl(impl&&) = default;
  constexpr impl& operator=(impl&&) = default;

  constexpr size_t which() const { return -1; }
};

template <const info_t& info, typename H, typename... T>
union BEST_RELOCATABLE impl<info, H, T...> {
 public:
  // clang-format off
  constexpr impl() requires (info.trivial_default) = default;
  constexpr impl() requires (!info.trivial_default) : t_{} {}

  constexpr impl(const impl&) requires (info.trivial_copy) = default;
  constexpr impl& operator=(const impl&) requires (info.trivial_copy) = default;
  constexpr impl(const impl&) requires (!info.trivial_copy) {}
  constexpr impl& operator=(const impl&) requires (!info.trivial_copy) { return *this; }

  constexpr impl(impl&&) requires (info.trivial_move) = default;
  constexpr impl& operator=(impl&&) requires (info.trivial_move) = default;
  constexpr impl(impl&&) requires (!info.trivial_move) {}
  constexpr impl& operator=(impl&&) requires (!info.trivial_move) { return *this; }

  constexpr ~impl() requires (info.trivial_dtor) = default;
  constexpr ~impl() requires (!info.trivial_dtor) {}
  // clang-format on

  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wc++11-narrowing")
  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args)
      : h_(best::in_place, BEST_FWD(args)...) {}
  BEST_POP_GCC_DIAGNOSTIC()

  template <size_t n, typename... Args>
  constexpr explicit impl(best::index_t<n>, Args&&... args)
    requires(n < sizeof...(T) + 1)
      : t_(best::index<n - 1>, BEST_FWD(args)...) {}

  template <size_t n>
  constexpr const auto& get(best::index_t<n>) const
    requires(n < sizeof...(T) + 1)
  {
    if constexpr (n == 0) {
      return h_;
    } else {
      return t_.get(best::index<n - 1>);
    }
  }

  template <size_t n>
  constexpr auto& get(best::index_t<n>)
    requires(n < sizeof...(T) + 1)
  {
    if constexpr (n == 0) {
      return h_;
    } else {
      return t_.get(best::index<n - 1>);
    }
  }

  constexpr const uint8_t* uninit() const { return t_.uninit(); }
  constexpr uint8_t* uninit() { return t_.uninit(); }

  /// NOTE: This type (and the union members) have very short names
  /// to minimize the size of mangled symbols that contain puns.
  best::object<H> h_;
  impl<info, T...> t_;
};
}  // namespace best::pun_internal

#endif  // BEST_CONTAINER_INTERNAL_PUN_H_
