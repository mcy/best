#ifndef BEST_CONTAINER_INTERNAL_PUN_H_
#define BEST_CONTAINER_INTERNAL_PUN_H_

#include <memory>

#include "best/base/port.h"
#include "best/container/object.h"
#include "best/memory/layout.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"

//! Internal implementation of best::pun.

namespace best::pun_internal {
struct info_t {
  const init_info_t& init;
  best::layout layout;
};

template <typename... Ts>
inline constexpr info_t info{
    .init = init_info<Ts...>,
    .layout = best::layout::of_union<Ts...>(),
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
  constexpr impl() requires (info.init.trivial_default) = default;
  constexpr impl() requires (!info.init.trivial_default) : t_{} {}

  constexpr impl(const impl&) requires (info.init.trivial_copy) = default;
  constexpr impl& operator=(const impl&) requires (info.init.trivial_copy) = default;
  constexpr impl(const impl&) requires (!info.init.trivial_copy) {}
  constexpr impl& operator=(const impl&) requires (!info.init.trivial_copy) { return *this; }

  constexpr impl(impl&&) requires (info.init.trivial_move) = default;
  constexpr impl& operator=(impl&&) requires (info.init.trivial_move) = default;
  constexpr impl(impl&&) requires (!info.init.trivial_move) {}
  constexpr impl& operator=(impl&&) requires (!info.init.trivial_move) { return *this; }

  constexpr ~impl() requires (info.init.trivial_dtor) = default;
  constexpr ~impl() requires (!info.init.trivial_dtor) {}
  // clang-format on

  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wc++11-narrowing")
  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args)
    requires best::object_type<H>
      : h_(BEST_FWD(args)...) {}
  BEST_POP_GCC_DIAGNOSTIC()

  template <typename... Args>
  constexpr explicit impl(best::index_t<0>, Args&&... args) : h_{} {
    best::object_ptr<H>(std::addressof(h_))
        .construct_in_place(BEST_FWD(args)...);
  }

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
  typename best::object<H>::wrapped_type h_;
  impl<info, T...> t_;
};
}  // namespace best::pun_internal

#endif  // BEST_CONTAINER_INTERNAL_PUN_H_