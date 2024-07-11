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

#ifndef BEST_CONTAINER_INTERNAL_ROW_H_
#define BEST_CONTAINER_INTERNAL_ROW_H_

#include <utility>

#include "best/container/object.h"
#include "best/meta/internal/tlist.h"
#include "best/meta/tlist.h"

namespace best::row_internal {
template <typename K, typename T>
concept has_key = std::is_same_v<K, T> || requires {
  typename T::BestRowKey;
  requires std::is_same_v<K, typename T::BestRowKey>;
};

/// Prepares a lookup table for finding `K` among `Ts`. This returns an array
/// whose first element is the number of matching elements `n`, and whose next
/// `n` elements are their indices among the `Ts`.
template <typename K, typename... Ts>
inline constexpr auto lookup = [] {
  size_t next = 0;
  std::array<size_t, sizeof...(Ts) + 1> table = {};
  ((has_key<K, Ts> ? table[1 + table[0]++] = next++ : next++), ...);
  return table;
}();

template <typename K, typename... Ts, const auto& lut = lookup<K, Ts...>,
          size_t count = lut[0]>
inline constexpr auto do_lookup() {
  return best::indices<lut[0]>.apply(
      []<size_t... i> { return best::vals<lut[i + 1]...>; });
}

template <size_t i, typename T>
struct elem {
  [[no_unique_address]] best::object<T> value;
};

template <typename, typename...>
struct impl;

template <typename I>
struct impl<I> {
  constexpr decltype(auto) apply_impl(auto&& cb) const { return cb(); }
};

template <typename I, typename A>
struct impl<I, A> {
  constexpr const auto& get_impl(best::index_t<0>) const { return x0; }
  constexpr auto& get_impl(best::index_t<0>) { return x0; }

  constexpr decltype(auto) apply_impl(auto&& cb) const { return cb(x0); }
  constexpr decltype(auto) apply_impl(auto&& cb) { return cb(x0); }

  [[no_unique_address]] best::object<A> x0;
};

template <typename I, typename A, typename B>
struct impl<I, A, B> {
  constexpr const auto& get_impl(best::index_t<0>) const { return x0; }
  constexpr auto& get_impl(best::index_t<0>) { return x0; }
  constexpr const auto& get_impl(best::index_t<1>) const { return x1; }
  constexpr auto& get_impl(best::index_t<1>) { return x1; }

  constexpr decltype(auto) apply_impl(auto&& cb) const { return cb(x0, x1); }
  constexpr decltype(auto) apply_impl(auto&& cb) { return cb(x0, x1); }

  [[no_unique_address]] best::object<A> x0;
  [[no_unique_address]] best::object<B> x1;
};

template <size_t... i, typename... Elems>
  requires(sizeof...(i) > 2)
struct impl<const best::vlist<i...>, Elems...> : elem<i, Elems>... {
  template <size_t j>
  constexpr const auto& get_impl(best::index_t<j>) const {
    using T = best::tlist<Elems...>::template type<j>;
    return static_cast<const elem<j, T>&>(*this).value;
  }
  template <size_t j>
  constexpr auto& get_impl(best::index_t<j>) {
    using T = best::tlist<Elems...>::template type<j>;
    return static_cast<elem<j, T>&>(*this).value;
  }

  constexpr decltype(auto) apply_impl(auto cb) const {
    return cb(static_cast<const elem<i, Elems>&>(*this).value...);
  }
  constexpr decltype(auto) apply_impl(auto&& cb) {
    return cb(static_cast<elem<i, Elems>&>(*this).value...);
  }
};

constexpr decltype(auto) object_call(auto&& cb, auto&& obj) {
  if constexpr (best::is_void<typename best::as_auto<decltype(obj)>::type> &&
                best::callable<decltype(cb), void()>) {
    best::call(static_cast<best::dependent<decltype(cb), decltype(obj)>&&>(cb));
  } else {
    best::call(BEST_FWD(cb), BEST_FWD(obj).or_empty());
  }
}

// See tlist_internal::slice_impl().
using ::best::tlist_internal::splat;
template <typename Out, size_t... i, size_t... j, size_t... k>
constexpr auto make_slicer(std::index_sequence<i...>, std::index_sequence<j...>,
                           std::index_sequence<k...>) {
  return []<splat<i>... prefix, splat<j>... infix, splat<k>... suffix>(
             prefix&&..., infix&&... args, suffix&&...) {
    return Out{BEST_FWD(args)...};
  };
}
template <best::bounds b, typename... Ts,
          auto count = b.try_compute_count(sizeof...(Ts))>
  requires(count.has_value())
constexpr auto slice(auto&& row) {
  using Out = decltype(row.types.template at<b>().template apply<best::row>());
  return BEST_FWD(row).apply([](auto&&... args) {
    return make_slicer<Out>(
        std::make_index_sequence<b.start>{}, std::make_index_sequence<*count>{},
        std::make_index_sequence<sizeof...(args) - (b.start + *count)>{})(
        BEST_FWD(args)...);
  });
}

// See tlist_internal::splice_impl().
using ::best::tlist_internal::splat;
template <typename Out, size_t... i, size_t... j, size_t... k>
constexpr auto make_splicer(std::index_sequence<i...>,
                            std::index_sequence<j...>,
                            std::index_sequence<k...>, auto&&... args) {
  return [&]<splat<i>... prefix, splat<j>... infix, splat<k>... suffix>(
             prefix&&... pre, infix&&..., suffix&&... suf) {
    return Out{BEST_FWD(pre)..., BEST_FWD(args)..., BEST_FWD(suf)...};
  };
}
template <best::bounds b, typename... Ts,
          auto count = b.try_compute_count(sizeof...(Ts))>
  requires(count.has_value())
constexpr auto splice(auto&& row, auto those_types, auto&& those) {
  using Out = decltype(row.types.template splice<b>(those_types)
                           .template apply<best::row>());
  return BEST_FWD(row).apply([&](auto&&... args) {
    return BEST_FWD(those).apply([&](auto&&... insert) {
      return make_splicer<Out>(
          std::make_index_sequence<b.start>{},
          std::make_index_sequence<*count>{},
          std::make_index_sequence<sizeof...(args) - (b.start + *count)>{},
          BEST_FWD(insert)...)(BEST_FWD(args)...);
    });
  });
}

// See tlist_internal::gather_impl() and tlist_internal::scatter_impl().
template <size_t... i>
constexpr auto gather(auto&& row)
  requires((i < best::as_auto<decltype(row)>::size()) && ...)
{
  using Out =
      decltype(row.types.template gather<i...>().template apply<best::row>());
  return Out{BEST_FWD(row)[best::index<i>]...};
}
template <size_t... i>
constexpr auto scatter(auto&& row, auto those_types, auto&& those)
  requires((i < best::as_auto<decltype(row)>::size()) && ...) &&
          (sizeof...(i) <= best::as_auto<decltype(those)>::size()) &&
          (sizeof...(i) <= best::as_auto<decltype(those_types)>::size()) &&
          (best::as_auto<decltype(those_types)>::size() <=
           best::as_auto<decltype(those)>::size())
{
  using Out =
      decltype(row.types
                   .template scatter<i...>(
                       those_types.template at<bounds{.count = sizeof...(i)}>())
                   .template apply<best::row>());
  return row.indices.apply([&]<typename... J>() {
    constexpr auto lut = [&] {
      std::array<size_t, best::as_auto<decltype(row)>::size()> lut{
          (J::value - J::value)...};
      size_t n = 1;
      ((i < lut.size() ? lut[i] = n++ : 0), ...);
      return lut;
    }();
    return those.apply([&](auto&&... args) {
      return Out{best::row{best::bind, BEST_FWD(row)[J{}],
                           args...}[best::index<lut[J::value]>]...};
    });
  });
}

// See tlist_internal::join().
using ::best::tlist_internal::fast_nth;
using ::best::tlist_internal::join_lut;
constexpr auto join(auto&&... those) {
  return [&]<auto... i>(best::vlist<i...>) {
    auto rowrow = best::row{best::bind, BEST_FWD(those)...};
    constexpr auto lut = join_lut<sizeof...(i), decltype(those.types)...>;

    return row<typename fast_nth<lut[i * 2], decltype(those.types)...>  //
               ::template type<lut[i * 2 + 1]>...>{
        BEST_MOVE(rowrow)[best::index<lut[i * 2]>]  //
            .get(best::index<lut[i * 2 + 1]>)...};
  }(best::indices<(0 + ... + best::as_auto<decltype(those)>::size())>);
}
}  // namespace best::row_internal

#endif  // BEST_CONTAINER_INTERNAL_ROW_H_
