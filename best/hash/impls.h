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

#ifndef BEST_HASH_IMPLS_H_
#define BEST_HASH_IMPLS_H_

#include "best/hash/hash.h"
#include "best/math/int.h"
#include "best/memory/span.h"
#include "best/meta/reflect.h"
#include "best/meta/traits/arrays.h"
#include "best/meta/traits/enums.h"

//! Implementations of `BestHash` for built-in types.

namespace best {
/// # `best::default_identity`
///
/// The default identity, which uses `operator==` and ordinary hashing.
struct default_identity final {
  template <typename K, best::equatable<K> Q>
  constexpr bool equal(const K& key, const Q& query) const {
    return key == query;
  }

  template <typename K, best::hashable Q, best::hash_state H>
  constexpr void hash(best::hasher<H>& h, const Q& query) const {
    if constexpr (best::is_string<K>) {
      // Need to deal with nul-terminated strings here.

      using code = best::code<best::encoding_type<K>>;
      using S = best::as_auto<Q>;
      if constexpr (best::same<S, const code*>) {
        h.write(best::span<const code>::from_nul(query));
        return;
      } else if constexpr (best::is_sized_array_of<S, code>) {
        constexpr auto n = *best::static_size<S> - 1;
        h.write(best::span<const code, n>(&query[0], n));
        return;
      }
    }

    h.write(query);
  }
};

template <best::hash_state State>
constexpr void BestHash(best::hasher<State>& h, bool value) {
  h.write((char)value);
}

template <best::hash_state State, best::is_int Int>
constexpr void BestHash(best::hasher<State>& h, Int value) {
  h.write(best::span<Int>(&value, 1).as_bytes());
}

template <best::hash_state State>
constexpr void BestHash(best::hasher<State>& h,
                        const best::contiguous auto& value)
  requires requires { h.write(best::span(value)[0]); }
{
  best::span sp = value;
  if constexpr (best::bytes_internal::byte_comparable<
                  best::data_type<decltype(sp)>>) {
    h.write(sp.size(), sp.as_bytes());
    return;
  }

  h.write(sp.size());
  for (const auto& v : sp) { h.write(v); }
}

template <best::hash_state State>
constexpr void BestHash(best::hasher<State>& h,
                        const best::is_reflected_struct auto& value) {
  best::reflect<decltype(value)>.each([&](auto field) {
    if (!field.template tags<best::transient>().is_empty()) { return; }

    h.write(value->*field);
  });
}

template <best::hash_state State>
constexpr void BestHash(best::hasher<State>& h,
                        const best::is_enum auto& value) {
  h.write(best::to_underlying(value));
}
}  // namespace best

#endif  // BEST_HASH_IMPLS_H_