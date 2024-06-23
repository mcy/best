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

#ifndef BEST_META_INTERNAL_TLIST_H_
#define BEST_META_INTERNAL_TLIST_H_

#include <stddef.h>

#include <array>
#include <type_traits>
#include <utility>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/container/bounds.h"
#include "best/func/call.h"

//! Type traits for working with parameter packs.

// This is wrapped up in a define like this so test can override it to make sure
// we can turn it off.
#ifndef BEST_TLIST_USE_CLANG_MAGIC_
#ifdef __clang__
#define BEST_TLIST_USE_CLANG_MAGIC_ 1
#else
#define BEST_TLIST_USE_CLANG_MAGIC_ 0
#endif
#endif  // BEST_TLIST_USE_CLANG_MAGIC_

namespace best::tlist_internal {
struct secret;
struct strict {};

template <typename... T>
void is_tlist(tlist<T...>);

template <typename T>
concept not_strict = !std::is_same_v<T, strict>;

template <size_t>
struct discard {
  discard(auto...) {}
};

// Low-level implementation of "index me a list". This is very similar to the
// implementation given below for make_slicer(), but it dodges a GCC bug around
// variadic constrained template parameters.
template <size_t... i>
auto make_indexer(std::index_sequence<i...>) {
  return [](discard<i>... prefix, auto infix, auto... suffix) { return infix; };
}

template <size_t i, typename... Ts>
using fast_nth =
#if BEST_TLIST_USE_CLANG_MAGIC_ && BEST_HAS_BUILTIN(__type_pack_element)
    __type_pack_element<i, Ts...>;
#else
    typename decltype(typename make_indexer(std::make_index_sequence<i_>{})(
        best::id<Ts>{}...))::type;
#endif

template <size_t n, typename Default, typename... Ts>
  requires(n < sizeof...(Ts))
auto nth_impl(tlist<Ts...>) {
#if BEST_TLIST_USE_CLANG_MAGIC_ && BEST_HAS_BUILTIN(__type_pack_element)
  return best::id<__type_pack_element<n, Ts...>>{};
#else
  return make_indexer(std::make_index_sequence<n>{})(best::id<Ts>{}...);
#endif
}

template <size_t n, not_strict Default, typename... Ts>
  requires(n >= sizeof...(Ts))
auto nth_impl(tlist<Ts...>) {
  return best::id<Default>{};
}

template <size_t n, typename Default, typename Pack>
using nth = decltype(nth_impl<n, Default>(best::lie<Pack>))::type;

template <typename, size_t>
concept splat = true;

// Low-level implementation of "slice me a tlist". This works by constructing
// a lambda that takes exactly sizeof...(i) + sizeof...(j) + sizeof...(k)
// template parameters and returning it. To create that, we take the packs of
// integers constructed using make_index_sequence, and splats them into
// a template parameter list using constrained-template-parameters, which allows
// use to use `...` in the template-parameter-list without creating an
// arbitrary-length pack argument. It has to return a lambda, because we want to
// "bake" i, j, and k into the lambda's type, so that they cannot be deduced
// when we actually call it.
template <size_t... i, size_t... j, size_t... k>
auto make_slicer(std::index_sequence<i...>, std::index_sequence<j...>,
                 std::index_sequence<k...>) {
  return []<splat<i>... prefix, splat<j>... infix, splat<k>... suffix>(
             id<prefix>..., id<infix>..., id<suffix>...) {
    return best::id<tlist<infix...>>{};
  };
}

template <size_t start, auto count, typename Default, typename... Ts>
  requires(count.has_value())
auto slice_impl(tlist<Ts...>) {
  return make_slicer(
      std::make_index_sequence<start>{}, std::make_index_sequence<*count>{},
      std::make_index_sequence<sizeof...(Ts) - (start + *count)>{})(
      best::id<Ts>{}...);
}
template <size_t start, auto count, not_strict Default, typename... Ts>
  requires(!count.has_value())
best::id<Default> slice_impl(tlist<Ts...>);

template <best::bounds b, typename Default, typename Pack>
using slice =
    decltype(slice_impl<b.start, b.try_compute_count(Pack::size()), Default>(
        best::lie<Pack>))::type;

// Inside-out version of make_slicer.
template <typename... Ts, size_t... i, size_t... j, size_t... k>
auto make_splicer(std::index_sequence<i...>, std::index_sequence<j...>,
                  std::index_sequence<k...>) {
  return []<splat<i>... prefix, splat<j>... infix, splat<k>... suffix>(
             id<prefix>..., id<infix>..., id<suffix>...) {
    return best::id<tlist<prefix..., Ts..., suffix...>>{};
  };
}

template <size_t start, auto count, typename Default, typename... Ts,
          typename... Us>
  requires(count.has_value())
auto splice_impl(tlist<Ts...>, tlist<Us...>) {
  return make_splicer<Us...>(
      std::make_index_sequence<start>{}, std::make_index_sequence<*count>{},
      std::make_index_sequence<sizeof...(Ts) - (start + *count)>{})(
      best::id<Ts>{}...);
}
template <size_t start, auto count, not_strict Default, typename... Ts,
          typename... Us>
  requires(!count.has_value())
best::id<Default> splice_impl(tlist<Ts...>, tlist<Us...>);

template <best::bounds b, typename Default, typename Pack, typename Insert>
using splice =
    decltype(splice_impl<b.start, b.try_compute_count(Pack::size()), Default>(
        best::lie<Pack>, best::lie<Insert>))::type;

template <typename Default, size_t... i, typename... Ts>
  requires((i < sizeof...(Ts)) && ...)  // Fast path for no out-of-bounds.
auto gather_impl(tlist<Ts...>) {
  return best::tlist<fast_nth<i, Ts...>...>{};
}
template <not_strict Default, size_t... i, typename... Ts>
  requires((i >= sizeof...(Ts)) || ...)
auto gather_impl(tlist<Ts...>) {
  return best::tlist<nth<i, Default, tlist<Ts...>>...>{};
}

template <typename Default, typename Pack, size_t... i>
using gather = decltype(gather_impl<Default, i...>(best::lie<Pack>));

template <size_t... i, size_t... j, typename... Ts,
          typename... Us>
  requires((i < sizeof...(Ts)) && ...)  // Fast path for no out-of-bounds.
auto scatter_impl(std::index_sequence<j...>, tlist<Ts...>, tlist<Us...>) {
  constexpr auto lut = [] {
    std::array<size_t, sizeof...(Ts)> lut{(j - j)...};
    size_t n = 1;
    ((i < lut.size() ? lut[i] = n++ : 0), ...);
    return lut;
  }();

  return best::tlist<fast_nth<lut[j], Ts, Us...>...>{};
}

template <typename Pack, typename Those, size_t... i>
using scatter =
    decltype(scatter_impl<i...>(std::make_index_sequence<Pack::size()>{},
                                best::lie<Pack>, best::lie<Those>));

// This generates a lookup table for computing a join.
//
// The even elements of the table are `{0, 0, 0, 1, 1, 1, 2, 2, 2}`, one `n`
// for each element of the `n`th list. The odd elements are then `{0, 1, 2, 0,
// 1, 2, 0, 1, 2}`: for each list `l`, the sequence `{.count = l.size() - 1}`.
template <size_t total, typename... Packs>
constexpr auto join_lut() {
  std::array<size_t, total * 2> lut;

  size_t running_total = 0;
  size_t list = 0;
  auto fill = [&](size_t n) {
    for (size_t i = 0; i < n; ++i) {
      lut[running_total * 2] = list;
      lut[running_total * 2 + 1] = i;
      ++running_total;
    }
    ++list;
  };
  (fill(Packs::size()), ...);
  return lut;
}
template <typename... Packs, size_t... i>
auto join_impl(std::index_sequence<i...>, Packs...) {
  constexpr auto lut = join_lut<sizeof...(i), Packs...>();
  return tlist<typename fast_nth<lut[i * 2], Packs...>  //
               ::template type<lut[i * 2 + 1]>...>{};
}
template <typename... Packs>
using join = decltype(join_impl(
    std::make_index_sequence<(0 + ... + Packs::size())>{}, Packs{}...));

template <typename T>
struct entry {};

struct fail : std::false_type {
  constexpr auto operator+(auto) { return fail{}; }
};

template <typename... Ts>
struct set final : std::true_type, entry<Ts>... {
  template <typename T>
  constexpr auto operator+(entry<T>) {
    if constexpr (std::is_base_of_v<entry<T>, set>) {
      return fail{};
    } else {
      return set<Ts..., T>{};
    }
  }
};

template <typename... Ts>
concept uniq = (set<>{} + ... + entry<Ts>{}).value;

// Concepts for checking if a function is callable with a particular set of
// type or value arguments.
template <typename F, typename... Elems>
concept t_callable = (... && best::callable<F, void(), Elems>);

template <typename F, typename... Elems>
concept v_callable = sizeof...(Elems) > 0 &&
                     (... && requires(F f) { best::call(f, Elems::value); });

template <typename F, typename... Elems>
concept ts_callable = best::callable<F, void(), Elems...>;

template <typename F, typename... Elems>
concept vs_callable =
    sizeof...(Elems) > 0 && requires(F f) { best::call(f, Elems::value...); };

}  // namespace best::tlist_internal

#endif  // BEST_META_INTERNAL_TLIST_H_
