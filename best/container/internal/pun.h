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

#include <type_traits>

#include "best/base/port.h"
#include "best/base/tags.h"
#include "best/container/object.h"
#include "best/meta/tlist.h"

//! Internal implementation of best::pun.

namespace best::pun_internal {
struct info_t {
  char p[4];
  constexpr bool trivial_default() const { return p[0]; }
  constexpr bool trivial_copy() const { return p[1]; }
  constexpr bool trivial_move() const { return p[2]; }
  constexpr bool trivial_dtor() const { return p[3]; }
};
template <typename... Ts>
inline constexpr info_t info = {
  ((std::is_void_v<Ts> || std::is_trivially_default_constructible_v<Ts>)&&...),
  ((std::is_void_v<Ts> || std::is_trivially_copyable_v<Ts>)&&...),
  ((std::is_void_v<Ts> || (std::is_trivially_move_constructible_v<Ts> &&
                           std::is_trivially_move_assignable_v<Ts>)) &&
   ...),
  ((std::is_void_v<Ts> || std::is_trivially_destructible_v<Ts>)&&...),
};

// A raw union.
template <const info_t& info, typename... Ts>
union impl;
#define BEST_PUN_CTORS_(d_)                                                                 \
  /* clang-format off */                                                                  \
  constexpr impl() requires (info.trivial_default()) = default;                             \
  constexpr impl() requires (!info.trivial_default()) : d_{} {}                             \
                                                                                          \
  constexpr impl(const impl&) requires (info.trivial_copy()) = default;                     \
  constexpr impl& operator=(const impl&) requires (info.trivial_copy()) = default;          \
  constexpr impl(const impl&) requires (!info.trivial_copy()) {}                            \
  constexpr impl& operator=(const impl&) requires (!info.trivial_copy()) { return *this; }  \
                                                                                          \
  constexpr impl(impl&&) requires (info.trivial_move()) = default;                          \
  constexpr impl& operator=(impl&&) requires (info.trivial_move()) = default;               \
  constexpr impl(impl&&) requires (!info.trivial_move()) {}                                 \
  constexpr impl& operator=(impl&&) requires (!info.trivial_move()) { return *this; }       \
  constexpr ~impl() requires (info.trivial_dtor()) = default;                               \
  constexpr ~impl() requires (!info.trivial_dtor()) {} \
  /* clang-format on */

// Implementations for common sizes. This reduces the number of instantiations
// for most uses of pun.

template <const info_t& info, typename H>
union BEST_RELOCATABLE impl<info, H> {
 public:
  BEST_PUN_CTORS_(d_)

  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wc++11-narrowing")
  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args)
    : h_(best::in_place, BEST_FWD(args)...) {}
  BEST_POP_GCC_DIAGNOSTIC()

  constexpr const auto& get(best::index_t<0>) const { return h_; }
  constexpr auto& get(best::index_t<0>) { return h_; }

  /// NOTE: This type (and the union members) have very short names
  /// to minimize the size of mangled symbols that contain puns.
  best::object<H> h_;
  char d_;
};

template <const info_t& info, typename H, typename T>
union BEST_RELOCATABLE impl<info, H, T> {
 public:
  BEST_PUN_CTORS_(d_)

  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wc++11-narrowing")
  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args)
    : h_(best::in_place, BEST_FWD(args)...) {}
  template <typename... Args>
  constexpr explicit impl(best::index_t<1>, Args&&... args)
    : t_(best::in_place, BEST_FWD(args)...) {}
  BEST_POP_GCC_DIAGNOSTIC()

  constexpr const auto& get(best::index_t<0>) const { return h_; }
  constexpr auto& get(best::index_t<0>) { return h_; }
  constexpr const auto& get(best::index_t<1>) const { return t_; }
  constexpr auto& get(best::index_t<1>) { return t_; }

  /// NOTE: This type (and the union members) have very short names
  /// to minimize the size of mangled symbols that contain puns.
  best::object<H> h_;
  best::object<T> t_;
  char d_;
};

template <const info_t& info, typename H, typename H2, typename... T>
union BEST_RELOCATABLE impl<info, H, H2, T...> {
 public:
  BEST_PUN_CTORS_(t_)

  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wc++11-narrowing")
  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args)
    : h_(best::in_place, BEST_FWD(args)...) {}
  template <typename... Args>
  constexpr explicit impl(best::index_t<1>, Args&&... args)
    : h2_(best::in_place, BEST_FWD(args)...) {}
  BEST_POP_GCC_DIAGNOSTIC()

  template <size_t n, typename... Args>
  constexpr explicit impl(best::index_t<n>, Args&&... args)
    : t_(best::index<n - 2>, BEST_FWD(args)...) {}

  constexpr const auto& get(best::index_t<0>) const { return h_; }
  constexpr auto& get(best::index_t<0>) { return h_; }
  constexpr const auto& get(best::index_t<1>) const { return h2_; }
  constexpr auto& get(best::index_t<1>) { return h2_; }

  template <size_t n>
  constexpr const auto& get(best::index_t<n>) const {
    return t_.get(best::index<n - 2>);
  }

  template <size_t n>
  constexpr auto& get(best::index_t<n>) {
    return t_.get(best::index<n - 2>);
  }

  /// NOTE: This type (and the union members) have very short names
  /// to minimize the size of mangled symbols that contain puns.
  best::object<H> h_;
  best::object<H2> h2_;
  impl<info, T...> t_;
};
#undef BEST_PUN_CTORS_
}  // namespace best::pun_internal

#endif  // BEST_CONTAINER_INTERNAL_PUN_H_
