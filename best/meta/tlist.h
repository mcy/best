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
/// # `best::is_tlist`, `best::is_vlist`
///
/// Whether a type is a tlist or vlist for some unknown set of template
/// arguments.
template <typename T>
concept is_tlist = requires(T t) { tlist_internal::is_tlist(t); };
template <typename T>
concept is_vlist = requires(T t) {
  tlist_internal::is_tlist(t);
  requires T::is_values();
};

/// # `best::is_tlist_of_size`
///
/// Whether a type is a tlist of a specified size.
template <typename T, size_t n>
concept is_tlist_of_size = requires(T t) {
  tlist_internal::is_tlist(t);
  requires T::size() == n;
};

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

/// # `best::index_t<n>`, `best::indices_t<n...>`
///
/// A aliases of `vals`/`vlist` for constraining it to `size_t` elements.
template <size_t n>
using index_t = val<n>;
template <size_t... n>
using indices_t = vlist<n...>;

/// # `best::index<n>`
///
/// Helper for constructing `best::index_t`. Note that `best::val<0>` is not
/// `best::index_t<0>`, because the former is `best::val<int(0)>`.
template <size_t n>
inline constexpr index_t<n> index;

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
 private:
  using strict = tlist_internal::strict;

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
  static constexpr bool is_values();

  /// # `best::unique_pack`
  ///
  /// Whether this pack of types has no duplicates.
  static constexpr bool is_unique() { return tlist_internal::uniq<Elems...>; };

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying `&&`.
  static constexpr decltype(auto) all() { return reduce<best::op::AndAnd>(); };

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying `||`.
  static constexpr decltype(auto) any() { return reduce<best::op::OrOr>(); };

  /// # `tlist::type<n>`
  ///
  /// Gets the type of the `n`th element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified, in which case that is returned instead.
  template <size_t n, typename Default = strict>
  using type = tlist_internal::nth<n, Default, tlist>;

  /// # `tlist::value<n>`
  ///
  /// Gets the value of the `n`th element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `default_` is
  /// specified, in which case that is returned instead.
  template <size_t n, auto default_ = strict{}>
  static constexpr auto value =
      type<n, best::select<tlist_internal::not_strict<decltype(default_)>,
                           tlist_internal::strict, val<default_>>>::value;

  /// # `tlist::at()`
  ///
  /// Slices into `Elems` with `bounds` and returns a new tlist with the
  /// corresponding types.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified, in which case that is returned instead.
  template <best::bounds bounds, typename Default = strict>
  static constexpr auto at();

  /// # `tlist::gather()`
  ///
  /// Selects elements of this tlist by index to form a new tlist.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified.
  template <size_t... ns>
  static constexpr auto gather();
  template <typename Default, size_t... ns>
  static constexpr auto gather();

  /// # `tlist::join()`, `tlist::operator+`
  ///
  /// Concatenates several tlists with this one.
  ///
  /// This is also made available via `operator+`, but beware: using a fold with
  /// `operator+` here is quadratic; you should generally prefer `join()` for
  /// variadic cases.
  static constexpr auto join(best::is_tlist auto... those);
  auto operator+(best::is_tlist auto that) { return join(that); }

#define BEST_TLIST_MUST_USE(func_)                            \
  [[nodiscard("best::tlist::" #func_                          \
              "() does not mutate its argument; instead, it " \
              "returns a new best::tlist")]]

  /// # `tlist::push()`
  ///
  /// Inserts a new value at the end of the list.
  template <typename T>
  BEST_TLIST_MUST_USE(push)
  static constexpr auto push();
  template <auto v>
  BEST_TLIST_MUST_USE(push)
  static constexpr auto push();

  /// # `tlist::insert()`
  ///
  /// Inserts a new value at the specified index.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified, in which case that is returned instead.
  template <size_t n, typename T, typename Default = strict>
  BEST_TLIST_MUST_USE(insert)
  static constexpr auto insert();
  template <size_t n, auto v, typename Default = strict>
  BEST_TLIST_MUST_USE(insert)
  static constexpr auto insert();

  /// # `tlist::splice()`
  ///
  /// Slices into `Elems` with `bounds` and returns a new tlist with the
  /// sliced portion replaced with `those`.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified, in which case that is returned instead.
  template <best::bounds bounds, typename Default = strict>
  BEST_TLIST_MUST_USE(splice)
  static constexpr auto splice(best::is_tlist auto that);

  /// # `tlist::scatter()`
  ///
  /// Updates elements of this tlist by selecting from the list provided.
  ///
  /// Ouf-of-bounds "writes" are discarded.
  template <auto those, size_t... ns>
  BEST_TLIST_MUST_USE(scatter)
  static constexpr auto scatter()
    requires best::is_tlist_of_size<decltype(those), sizeof...(ns)>
  {
    return tlist_internal::scatter<tlist, decltype(those), ns...>{};
  }

  /// # `tlist::update()`
  ///
  /// Updates a single element of this list.
  ///
  /// Ouf-of-bounds "writes" are discarded.
  template <size_t n, typename T>
  BEST_TLIST_MUST_USE(update)
  static constexpr auto update();
  template <size_t n, auto v>
  BEST_TLIST_MUST_USE(update)
  static constexpr auto update();

  /// # `tlist::remove()`
  ///
  /// Removes the element at index `n`.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified.
  template <size_t n, typename Default = strict>
  BEST_TLIST_MUST_USE(remove)
  static constexpr auto remove();

  /// # `tlist::erase()`
  ///
  /// Slices into `Elems` with `bounds` and returns a new tlist tlist with the
  /// sliced portion removed.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless `Default` is
  /// specified.
  template <best::bounds bounds, typename Default = strict>
  BEST_TLIST_MUST_USE(erase)
  static constexpr auto erase();

  /// # `tlist::trim_prefix()`
  ///
  /// If `list` is a prefix of this tlist, returns a new tlist with those
  /// elements chopped off.
  template <typename Default = strict>
  BEST_TLIST_MUST_USE(trim_prefix)
  static constexpr auto trim_prefix(best::is_tlist auto prefix);

  /// # `tlist::trim_prefix()`
  ///
  /// If `list` is a suffix of this tlist, returns a new tlist with those
  /// elements chopped off.
  template <typename Default = strict>
  BEST_TLIST_MUST_USE(trim_suffix)
  static constexpr auto trim_suffix(best::is_tlist auto suffix);

#undef BEST_TLIST_MUST_USE

  /// # `tlist::find()`
  ///
  /// Returns the first index of this list that satisfies the given type
  /// predicate. This predicate can be a callback (as in `map()`), a bool-typed
  /// value trait, or a type/value to search for.
  static constexpr auto find(tlist_internal::t_callable<Elems...> auto pred) {
    size_t n = -1;
    if (((++n, best::call<Elems>(pred)) || ...)) {
      return opt_size(n);
    }
    return opt_size{};
  }
  static constexpr auto find(tlist_internal::v_callable<Elems...> auto pred) {
    return find([&]<typename V> { return pred(V::value); });
  }
  template <typename T>
  static constexpr auto find() {
    return find([]<typename U> { return std::same_as<T, U>; });
  }
  template <template <typename> typename Trait>
  static constexpr auto find() {
    return find([]<typename U> { return Trait<U>::value; });
  }
  static constexpr auto find(auto value)
    requires(best::equatable<decltype(value), decltype(Elems::value)> && ...)
  {
    return find([&](auto that) { return value == that; });
  }

  /// # `tlist::find_unique()`
  ///
  /// Like `find()`, but requires that the returned element be the unique match.
  static constexpr auto find_unique(
      tlist_internal::t_callable<Elems...> auto pred) {
    size_t n = -1;
    size_t found = 0;
    if ((((found || ++n), best::call<Elems>(pred) && (++found == 2)) || ...) ||
        found != 1) {
      return opt_size{};
    }
    return opt_size{n};
  }
  static constexpr auto find_unique(
      tlist_internal::v_callable<Elems...> auto pred) {
    return find_unique([&]<typename V> { return pred(V::value); });
  }
  template <typename T>
  static constexpr auto find_unique() {
    return find_unique([]<typename U> { return std::same_as<T, U>; });
  }
  template <template <typename> typename Trait>
  static constexpr auto find_unique()
    requires(... && value_trait<Trait<Elems>>)
  {
    return find_unique([]<typename U> { return Trait<U>::value; });
  }
  static constexpr auto find_unique(auto value)
    requires(best::equatable<decltype(value), decltype(Elems::value)> && ...)
  {
    return find_unique([&](auto that) { return value == that; });
  }

  /// # `tlist::map()`
  ///
  /// Applies a function or a type trait to each type in this list and returns a
  /// new list with the results.
  ///
  /// `cb` can either take zero arguments and exactly one template parameter, or
  /// exactly one argument. In the latter case, this tlist must be a vlist. If
  /// passing a trait instead, it must take either one `typename` or one `auto`.
  template <tlist_internal::t_callable<Elems...> auto cb>
  static constexpr decltype(auto) map() {
    return best::vals<best::call<Elems>(cb)...>;
  }
  template <tlist_internal::v_callable<Elems...> auto cb>
  static constexpr decltype(auto) map() {
    return best::vals<best::call(cb, Elems::value)...>;
  }
  template <template <typename> typename Trait>
  static constexpr decltype(auto) map();
  template <template <auto> typename Trait>
  static constexpr decltype(auto) map();

  /// # `tlist::each()`
  ///
  /// Applies `cb` to each type in this list as in `map()` but does not require
  /// that `cb` return a value.
  static constexpr void each(tlist_internal::t_callable<Elems...> auto cb) {
    (best::call<Elems>(cb), ...);
  }
  static constexpr void each(tlist_internal::v_callable<Elems...> auto cb) {
    (best::call(cb, Elems::value), ...);
  }

  /// # `tlist::apply()`
  ///
  /// Applies `cb` to *every* type in this list at once. `cb` may either have
  /// a `typename...` parameter or an `auto...` argument.
  static constexpr decltype(auto) apply(
      tlist_internal::ts_callable<Elems...> auto cb) {
    return best::call<Elems...>(cb);
  }
  static constexpr decltype(auto) apply(
      tlist_internal::vs_callable<Elems...> auto cb) {
    return best::call(cb, Elems::value...);
  };
  template <template <typename...> typename Trait>
  static constexpr Trait<Elems...> apply() {
    return {};
  }
  template <template <auto...> typename Trait, typename delay = void>
  static constexpr Trait<best::dependent<Elems, delay>::value...> apply() {
    return {};
  };

  /// # `tlist::reduce()`
  ///
  /// Reduces this vlist by applying op to each element's value..
  template <best::op op>
  static constexpr decltype(auto) reduce() {
    return best::operate<op>(Elems::value...);
  }

  constexpr bool operator==(best::is_tlist auto) const;
  constexpr best::partial_ord operator<=>(best::is_tlist auto that) const;

 private:
  template <typename...>
  friend class tlist;
  using opt_size = best::container_internal::option<size_t>;

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

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename... Elems>
constexpr bool tlist<Elems...>::is_values() {
  return (best::value_trait<Elems> && ...);
}

template <typename... Elems>
constexpr auto tlist<Elems...>::join(best::is_tlist auto... those) {
  return tlist_internal::join<tlist, decltype(those)...>{};
}

template <typename... Elems>
template <template <typename> typename Trait>
constexpr decltype(auto) tlist<Elems...>::map() {
  return types<Trait<Elems>...>;
}
template <typename... Elems>
template <template <auto> typename Trait>
constexpr decltype(auto) tlist<Elems...>::map() {
  return types<Trait<Elems::values>...>;
}

template <typename... Elems>
template <best::bounds bounds, typename Default>
constexpr auto tlist<Elems...>::at() {
  return tlist_internal::slice<bounds, Default, tlist>{};
}

template <typename... Elems>
template <typename T>
constexpr auto tlist<Elems...>::push() {
  return insert<size(), T>();
}
template <typename... Elems>
template <auto v>
constexpr auto tlist<Elems...>::push() {
  return insert<size(), v>();
}

template <typename... Elems>
template <size_t n, typename T, typename Default>
constexpr auto tlist<Elems...>::insert() {
  return splice<bounds{.start = n, .count = 0}, Default>(best::types<T>);
}
template <typename... Elems>
template <size_t n, auto v, typename Default>
constexpr auto tlist<Elems...>::insert() {
  return splice<bounds{.start = n, .count = 0}, Default>(best::vals<v>);
}

template <typename... Elems>
template <best::bounds bounds, typename Default>
constexpr auto tlist<Elems...>::splice(best::is_tlist auto that) {
  return tlist_internal::splice<bounds, Default, tlist, decltype(that)>{};
}

template <typename... Elems>
template <size_t... ns>
constexpr auto tlist<Elems...>::gather() {
  return tlist_internal::gather<best::tlist_internal::strict, tlist, ns...>{};
}
template <typename... Elems>
template <typename Default, size_t... ns>
constexpr auto tlist<Elems...>::gather() {
  return tlist_internal::gather<Default, tlist, ns...>{};
}

template <typename... Elems>
template <size_t n, typename T>
constexpr auto tlist<Elems...>::update() {
  return splice<bounds{.start = n, .count = 0}, tlist>(best::types<T>);
}
template <typename... Elems>
template <size_t n, auto v>
constexpr auto tlist<Elems...>::update() {
  return splice<bounds{.start = n, .count = 0}, tlist>(best::vals<v>);
}

template <typename... Elems>
template <size_t n, typename Default>
constexpr auto tlist<Elems...>::remove() {
  return splice<bounds{.start = n, .count = 1}, Default>(best::types<>);
}

template <typename... Elems>
template <best::bounds bounds, typename Default>
constexpr auto tlist<Elems...>::erase() {
  return splice<bounds, Default>(best::types<>);
}

template <typename... Elems>
template <typename Default>
constexpr auto tlist<Elems...>::trim_prefix(best::is_tlist auto list) {
  using D = best::select<tlist_internal::not_strict<Default>, Default, tlist>;
  using Out = decltype(list.remove_prefix_from(tlist{}));
  if constexpr (std::is_void_v<Out>) {
    return D{};
  } else {
    return Out{};
  }
}
template <typename... Elems>
template <typename Default>
constexpr auto tlist<Elems...>::trim_suffix(best::is_tlist auto list) {
  using D = best::select<tlist_internal::not_strict<Default>, Default, tlist>;
  using Out = decltype(list.remove_suffix_from(tlist{}));
  if constexpr (std::is_void_v<Out>) {
    return D{};
  } else {
    return Out{};
  }
}

template <typename... Elems>
constexpr bool tlist<Elems...>::operator==(best::is_tlist auto that) const {
  return std::is_same_v<tlist, decltype(that)>;
}
template <typename... Elems>
constexpr best::partial_ord tlist<Elems...>::operator<=>(
    best::is_tlist auto that) const {
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
}  // namespace best

#endif  // BEST_META_TLIST_H_
