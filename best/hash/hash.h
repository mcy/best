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

#ifndef BEST_HASH_HASH_H_
#define BEST_HASH_HASH_H_

#include "best/base/fwd.h"
#include "best/hash/internal/hash.h"
#include "best/meta/init.h"
#include "best/meta/traits/types.h"

//! General hashing utilities for use with hash tables.
//!
//! A `best::hasher` represents an in-process hash operation, which is passed
//! to hashing functions for writing data to. These "hashing functions" are
//! FTADLEs of the form
//!
//!  template <best::hash_state State>
//!  friend void BestHash(best::hasher<State> h, const T& value);
//!
//! where `T` is the type being made hashable. This function is automatically
//! provided for all equality-comparable types in best. Reflectable types are
//! automatically made hashable by hashing their fields.
//!
//! Implementing `BestHash()` only requires including this header; other headers
//! provide the rest of the machinery.

namespace best {
/// # `best::hash_state`
///
/// An implementation of a hasher. Such types should not be used directly;
/// instead, use `best::hasher`.
///
/// A hash state is a type that provides a member alias `output`,
/// a member function `write(best::span<const char>)`, and a member function
/// `finish()`.
///
/// A hash state may provide further overloads of `write()` for optimizing
/// writing values of certain types, such as integers.
template <typename H>
concept hash_state =
  requires(H& state, best::span<best::dependent<const char, H>> data) {
    typename H::output;

    { state.write(data) };
    { state.finish() } -> best::same<typename H::output>;
  };

/// # `best::hash<S>`
///
/// The output of some `best::hasher`.
template <best::hash_state State>
using hash = State::output;

/// # `best::transient`
///
/// A field tag which marks a field as "transient" and thus not relevant for
/// hashing. Can be used to exclude fields from being hashed when using
/// reflection-generated hash functions.
struct transient_t final {};
inline constexpr best::transient_t transient{};

/// # `best::hasher`
///
/// A hasher calculates hashes of hashable values. Values should be written
/// using `write()`, and the final state
template <best::hash_state H>
class hasher final {
 public:
  /// # `hasher::hasher(...)`
  ///
  /// Constructs a new hasher with the given arguments, which are forwarded
  /// to `H`.
  template <typename... Args>
  explicit constexpr hasher(Args&&... args)
    requires best::constructible<H, Args&&...>
    : state_(BEST_FWD(args)...) {}

  constexpr hasher() = default;
  constexpr hasher(const hasher&) = default;
  constexpr hasher& operator=(const hasher&) = default;
  constexpr hasher(const hasher&&) = default;
  constexpr hasher& operator=(const hasher&&) = default;

  /// # `hasher::write(...)`
  ///
  /// Advances the state of the hasher by writing hashable values to it.
  template <typename T>
  hasher& write(const T& value) requires (
    requires { best::lie<H>.write(value); } ||
    requires { BestHash(*this, value); })
  {
    if constexpr (requires { state_.write(value); }) {
      state_.write(value);
    } else {
      BestHash(*this, value);
    }

    return *this;
  }
  template <typename... T>
  hasher& write(const T&... value)
    requires (sizeof...(T) != 1 && requires { (write(value), ...); })
  {
    (write(value), ...);
    return *this;
  }

  /// # `hasher::finish()`
  ///
  /// Completes a hashing operation, and returns the result.
  best::hash<H> finish() { return state_.finish(); }

 private:
  H state_;
};

/// # `best::hashable`
///
/// A type which can be hashed, i.e, which provides BestHash.
template <typename T>
concept hashable =
  best::is_void<T> ||
  requires(
    const T& v,
    best::hasher<best::hash_internal::dummy<best::dependent<const char, T>>>&
      h) { h.write(v); };

/// # `best::hash_of`
///
/// Returns the hash of the given elements as if they were passed to
/// a `best::hasher` in that order. The hash state used must be
/// default-constructible.
template <best::hash_state H, typename... Args>
best::hash<H> hash_of(const Args&... args) {
  return best::hasher<H>().write(args...).finish();
}

/// # `best::hash_identity`
///
/// A hash identity is a generalization of equality suitable for use with a
/// hash table. It specifies an equality function between a key type and a
/// query type, and a way to hash them, such that if two values are equal, then
/// their hashes are also equal.
///
/// A specific type may be the hash identity for many possible combinations of
/// K and Q. The only requirement is that satisfaction of this concept must be
/// an equivalence relation on types, and the equality function must also be an
/// equivalence relation in the expected way.
template <typename Id, typename K, typename Q = K>
concept hash_identity = requires(
  const Id& id, const K& key, const Q& query,
  best::hasher<best::hash_internal::dummy<best::dependent<const char, Id>>>&
    h) {
  { id.equal(key, query) } -> best::same<bool>;
  { id.hash<K>(h, key) };
  { id.hash<K>(h, query) };
};

}  // namespace best

#endif  // BEST_HASH_HASH_H_