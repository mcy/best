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

#ifndef BEST_META_TLIST_H_
#define BEST_META_TLIST_H_

#include <stddef.h>

#include <type_traits>

#include "best/base/ord.h"
#include "best/container/bounds.h"
#include "best/func/call.h"
#include "best/meta/internal/tlist.h"
#include "best/meta/ops.h"
#include "best/meta/traits.h"

//! Type-level list types.
//!
//! This file provides two types: best::tlist and best::vlist. Each
//! represents a `typename...` and `auto...` pack, respectively.

namespace best {
/// # `best::val<x>`
///
/// A value-as-a-type.
///
/// This type helps bridge the type/value universes by being the canonical
/// empty type with a variable named `value`. It is also a type trait that
/// produces the type of `x`.
template <auto x>
struct val {
  using type = decltype(x);
  static constexpr auto value = x;
};

/// # `best::vlist<...>`
///
/// A "value list", interpreted as a type list of the canonical value trait.
template <auto... elems>
using vlist = best::tlist<best::val<elems>...>;

/// # `best::types<...>`
///
/// Helper for constructing a `tlist` value.
template <typename... Elems>
inline constexpr best::tlist<Elems...> types;

/// # `best::values<...>`
///
/// Helper for constructing a `vlist` value.
template <auto... elems>
inline constexpr best::vlist<elems...> vals;

/// # `best::indices<...>`
///
/// Constructs a `vlist` containing the elements from `0` to `n`, exclusive.
template <size_t n>
inline constexpr auto indices = []<size_t... i>(std::index_sequence<i...>) {
  return best::vals<i...>;
}(std::make_index_sequence<n>{});

/// Variadic version of std::is_same/std::same_as.
template <typename... Ts>
concept same =
    (std::same_as<Ts, typename best::tlist<Ts...>::template type<0, void>> &&
     ...);

/// # `best::tlist<...>`
///
/// A type-level type list. This type makes it easy to manipulate packs of types
/// as first-class objects.
///
/// Type lists are partially-ordered. If two nth have the same elements, they
/// are equal. If `a`'s elements are a prefix of `b`'s, then `a < b`. Otherwise,
/// `a` and `b` are incomparable.
template <typename... Elems>
class tlist final {
 public:
  constexpr tlist() = default;
  constexpr tlist(const tlist&) = default;
  constexpr tlist& operator=(const tlist&) = default;
  constexpr tlist(tlist&&) = default;
  constexpr tlist& operator=(tlist&&) = default;

  /// # `tlist::global`
  ///
  /// A stateless global mutable variable of this list's type.
  inline static tlist global;

  /// # `tlist::size()`
  ///
  /// The number of elements in this list.
  static constexpr size_t size() { return sizeof...(Elems); }

  /// # `tlist::is_empty()`
  ///
  /// Whether the list is empty.
  static constexpr bool is_empty() { return size() == 0; }

  /// # `tlist::is_values()`
  ///
  /// Returns whether every element of this list is a `best::value_trait`.
  static constexpr bool is_values() {
    return (best::value_trait<Elems> && ...);
  }

  /// # `tlist::concat()`
  ///
  /// Concatenates several tlists with this one. If an argument to this type is
  /// not a tlist, it is treated as a one-element tlist.
  static constexpr auto concat(auto... those) {
    return tlist_internal::concat<Elems...>(those...);
  }

  /// # `best::unique_pack`
  ///
  /// Whether this pack of types is a
  template <typename... Ts>
  static constexpr bool is_unique() {
    return tlist_internal::uniq<Ts...>;
  };

 private:
  template <typename F>
  static constexpr bool type_callable = (... && callable<F, void(), Elems>);
  template <typename F>
  static constexpr bool value_callable = !is_empty() && (... && requires {
    Elems::value;
    requires callable<F, void(decltype(Elems::value))>;
  });

  template <typename F>
  static constexpr bool types_callable = callable<F, void(), Elems...>;

  template <typename F>
  static constexpr bool values_callable = !is_empty() && requires {
    (Elems::value, ...);
    requires callable<F, void(decltype(Elems::value)...)>;
  };

 public:
  /// # `tlist::map()`
  ///
  /// Applies a function or a type trait to each type in this list and returns a
  /// new list with the results.
  ///
  /// `cb` can either take zero arguments and exactly one template parameter, or
  /// exactly one argument. In the latter case, this must be a `vlist`. If
  /// passing a trait instead, it must take either one `typename` or one `auto`.
  template <auto cb>
  static constexpr decltype(auto) map()
    requires type_callable<decltype(cb)>
  {
    return vals<best::call<Elems>(cb)...>;
  }
  template <template <typename> typename Trait>
  static constexpr decltype(auto) map() {
    return types<Trait<Elems>...>;
  }
  template <auto cb>
  static constexpr decltype(auto) map()
    requires value_callable<decltype(cb)>
  {
    return vals<best::call(cb, Elems::value)...>;
  }
  template <template <auto> typename Trait>
  static constexpr decltype(auto) map()
    requires(is_values())
  {
    return types<Trait<Elems::values>...>;
  }

  /// # `tlist::each()`
  ///
  /// Applies `cb` to each type in this list as in `map()` but does not require
  /// that `cb` return a value.
  static constexpr void each(auto cb)
    requires type_callable<decltype(cb)>
  {
    (best::call<Elems>(cb), ...);
  }
  static constexpr void each(auto cb)
    requires value_callable<decltype(cb)>
  {
    (best::call(cb, Elems::value), ...);
  }

  /// # `tlist::apply()`
  ///
  /// Applies `cb` to *every* type in this list at once. `cb` may either have
  /// a `typename...` parameter or an `auto...` argument.
  static constexpr decltype(auto) apply(auto cb)
    requires types_callable<decltype(cb)>
  {
    return best::call<Elems...>(cb);
  }
  static constexpr decltype(auto) apply(auto cb)
    requires values_callable<decltype(cb)>
  {
    return best::call(cb, Elems::value...);
  };

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying op to each element's value..
  template <best::op op>
  static constexpr decltype(auto) reduce()
    requires(is_values())
  {
    return best::operate<op>(Elems::value...);
  }

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying `&&`.
  static constexpr decltype(auto) all()
    requires(is_values())
  {
    return reduce<best::op::AndAnd>();
  };

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying `||`.
  static constexpr decltype(auto) any()
    requires(is_values())
  {
    return reduce<best::op::OrOr>();
  };

  /// # `tlist::type<n>`
  ///
  /// Gets the type of the `n`th element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified.
  template <size_t n, typename Default = tlist_internal::strict>
  using type = tlist_internal::nth<n, Default, Elems...>;

  /// # `tlist::value<n>`
  ///
  /// Gets the value of the `n`th element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `default_` is
  /// specified.
  template <size_t n, auto default_ = tlist_internal::strict{}>
  static constexpr auto value =
      type<n, best::select<
                  std::is_same_v<decltype(default_), tlist_internal::strict>,
                  tlist_internal::strict, val<default_>>>::value;

  /// # `tlist::at()`
  ///
  /// Slices into `Elems` with `bounds` and returns a new tlist with the
  /// corresponding types.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified.
  template <best::bounds bounds, typename Default = tlist_internal::strict>
  static constexpr tlist_internal::slice<bounds, Default, tlist> at() {
    return {};
  }

  /// # `tlist::trim_prefix()`
  ///
  /// If `list` is a prefix of this tlist, returns a new tlist with those
  /// elements chopped off.
  template <typename Default = tlist_internal::strict>
  static constexpr auto trim_prefix(auto list) {
    using D = best::select<std::is_same_v<Default, tlist_internal::strict>,
                           tlist, Default>;
    using Out = decltype(list.remove_prefix_from(tlist{}));
    if constexpr (std::is_void_v<Out>) {
      return D{};
    } else {
      return Out{};
    }
  }

  /// # `tlist::trim_prefix()`
  ///
  /// If `list` is a suffix of this tlist, returns a new tlist with those
  /// elements chopped off.
  template <typename Default = tlist_internal::strict>
  static constexpr auto trim_suffix(auto list) {
    using D = best::select<std::is_same_v<Default, tlist_internal::strict>,
                           tlist, Default>;
    using Out = decltype(list.remove_suffix_from(tlist{}));
    if constexpr (std::is_void_v<Out>) {
      return D{};
    } else {
      return Out{};
    }
  }

  /// # `tlist::find()`
  ///
  /// Returns the first index of this list that satisfies the given type
  /// predicate. This predicate can be a callback (as in `map()`), a bool-typed
  /// value trait, or a type/value to search for.
  static constexpr best::container_internal::option<size_t> find(auto pred)
    requires type_callable<decltype(pred)>
  {
    size_t n = -1;
    if (((++n, best::call<Elems>(pred)) || ...)) {
      return n;
    }
    return {};
  }
  static constexpr best::container_internal::option<size_t> find(auto pred)
    requires value_callable<decltype(pred)>
  {
    return find([&]<typename V> { return pred(V::value); });
  }
  template <typename T>
  static constexpr best::container_internal::option<size_t> find() {
    return find([]<typename U> { return std::same_as<T, U>; });
  }
  template <template <typename> typename Trait>
  static constexpr best::container_internal::option<size_t> find()
    requires(... && value_trait<Trait<Elems>>)
  {
    return find([]<typename U> { return Trait<U>::value; });
  }
  static constexpr best::container_internal::option<size_t> find(auto value)
    requires(!value_callable<decltype(value)> && ... &&
             best::equatable<decltype(value), decltype(Elems::value)>)
  {
    return find([&](auto that) { return value == that; });
  }

  /// # `tlist::find_unique()`
  ///
  /// Like `find()`, but requires that the returned element be the unique match.
  static constexpr best::container_internal::option<size_t> find_unique(
      auto pred)
    requires type_callable<decltype(pred)>
  {
    size_t n = -1;
    size_t found = 0;
    if ((((found || ++n), best::call<Elems>(pred) && (++found == 2)) || ...) ||
        found != 1) {
      return {};
    }
    return n;
  }
  static constexpr best::container_internal::option<size_t> find_unique(
      auto pred)
    requires value_callable<decltype(pred)>
  {
    return find_unique([&]<typename V> { return pred(V::value); });
  }
  template <typename T>
  static constexpr best::container_internal::option<size_t> find_unique() {
    return find_unique([]<typename U> { return std::same_as<T, U>; });
  }
  template <template <typename> typename Trait>
  static constexpr best::container_internal::option<size_t> find_unique()
    requires(... && value_trait<Trait<Elems>>)
  {
    return find_unique([]<typename U> { return Trait<U>::value; });
  }
  static constexpr best::container_internal::option<size_t> find_unique(
      auto value)
    requires(!value_callable<decltype(value)> && ... &&
             best::equatable<decltype(value), decltype(Elems::value)>)
  {
    return find_unique([&](auto that) { return value == that; });
  }

  constexpr bool operator==(tlist) const { return true; }
  template <typename... Those>
  constexpr bool operator==(tlist<Those...>) const {
    return false;
  }
  template <typename... Those>
  constexpr best::partial_ord operator<=>(tlist<Those...> that) const {
    if constexpr (tlist{} == that) {
      return best::partial_ord::equivalent;
    } else if constexpr (!std::is_void_v<decltype(remove_prefix_from(that))>) {
      return best::partial_ord::less;
    } else if constexpr (!std::is_void_v<decltype(that.remove_prefix_from(
                             tlist{}))>) {
      return best::partial_ord::greater;
    } else {
      return best::partial_ord::unordered;
    }
  }

 private:
  template <typename...>
  friend class tlist;

  template <typename... Extra>
  static constexpr tlist<Extra...> remove_prefix_from(
      tlist<Elems..., Extra...>) {
    return {};
  }
  static constexpr void remove_prefix_from(auto) {}

  template <typename... Extra>
  static constexpr tlist<Extra...> remove_suffix_from(
      tlist<Extra..., Elems...>) {
    return {};
  }
  static constexpr void remove_suffix_from(auto) {}
};
}  // namespace best

#endif  // BEST_META_TLIST_H_
