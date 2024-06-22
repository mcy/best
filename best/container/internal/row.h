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
#include "best/meta/empty.h"
#include "best/meta/tags.h"
#include "best/meta/tlist.h"

namespace best::row_internal {
// This builds a set of lookup indices that find the first index that matches
// lookup up a column type or column name.
template <size_t n, typename T>
struct entry {
  size_t table[n];
  size_t total = 0;

  constexpr void append(size_t m) { table[total++] = m; }
};

template <typename T>
concept has_col_name = requires {
  typename T::BestRowKey;
  requires(!best::same<T, typename T::BestRowKey>);
};

template <size_t n, typename... Ts>
struct lookup_table final : entry<n, Ts>... {
  size_t next_idx = 0;

  template <typename T>
  constexpr auto operator+(entry<n, T>*)
    requires(!has_col_name<T>)
  {
    if constexpr (std::is_base_of_v<entry<n, T>, lookup_table>) {
      static_cast<entry<n, T>&>(*this).append(next_idx++);
      return *this;
    } else {
      return lookup_table<n, Ts..., T>{
          entry<n, Ts>(*this)...,
          entry<n, T>{{next_idx}, 1},
          next_idx + 1,
      };
    }
  }

  template <typename T>
  constexpr auto operator+(entry<n, T>*)
    requires has_col_name<T>
  {
    using K = T::BestRowKey;
    if constexpr (std::is_base_of_v<entry<n, T>, lookup_table>) {
      static_cast<entry<n, T>&>(*this).append(next_idx);
      static_cast<entry<n, K>&>(*this).append(next_idx++);
      return *this;
    } else if constexpr (std::is_base_of_v<entry<n, K>, lookup_table>) {
      static_cast<entry<n, K>&>(*this).append(next_idx++);
      return lookup_table<n, Ts..., T>{
          entry<n, Ts>(*this)...,
          entry<n, T>{{next_idx}, 1},
          next_idx + 1,
      };
    } else {
      return lookup_table<n, Ts..., T, K>{
          entry<n, Ts>(*this)...,
          entry<n, T>{{next_idx}, 1},
          entry<n, K>{{next_idx}, 1},
          next_idx + 1,
      };
    }
  }

  template <typename T>
  constexpr size_t count() const {
    if constexpr (std::is_base_of_v<entry<n, T>, lookup_table>) {
      return static_cast<const entry<n, T>&>(*this).total;
    }
    return {};
  }
};

template <typename... Ts>
inline constexpr auto lookup =
    (lookup_table<sizeof...(Ts)>{} + ... + (entry<sizeof...(Ts), Ts>*){});

template <typename K, typename... Ts, const auto& lut = lookup<Ts...>,
          size_t count = lut.template count<K>()>
inline constexpr auto apply_lookup(auto cb) {
  if constexpr (count == 0) {
    return best::call(cb);
  } else {
    const size_t(&table)[sizeof...(Ts)] =
        static_cast<const entry<sizeof...(Ts), K>&>(lut).table;

    return [&]<size_t... i>(std::index_sequence<i...>) {
      return best::call(cb, best::index<table[i]>...);
    }(std::make_index_sequence<count>{});
  }
}

template <typename, typename...>
struct impl;

template <size_t... i, typename... Elems>
struct impl<const best::vlist<i...>, Elems...>
    : best::ebo<best::object<Elems>, Elems, i>... {};
}  // namespace best::row_internal

#endif  // BEST_CONTAINER_INTERNAL_ROW_H_
