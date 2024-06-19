#ifndef BEST_META_TLIST_H_
#define BEST_META_TLIST_H_

#include <stddef.h>

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

#include "best/container/bounds.h"
#include "best/meta/internal/tlist.h"
#include "best/meta/ops.h"
#include "best/meta/traits.h"

//! Type-level list types.
//!
//! This file provides two types: best::tlist and best::vlist. Each
//! represents a `typename...` and `auto...` pack, respectively.

namespace best {
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

/// A helper for extracting a type trait.
///
/// For example, passing this to `map` on a type list will evaluate all type
/// traits therein.
template <typename T>
using extract_trait = T::type;

/// A value trait: any type with a static data member named `value`.
///
/// If R is specified explicitly, this requires a specific type for the member.

/// A "value list", interpreted as a type list of the canonical value trait.
template <auto... elems>
using vlist = tlist<val<elems>...>;

/// Helpers for constructing values of a particular tlist/vlist type.
template <typename... Elems>
inline constexpr tlist<Elems...> types;
template <auto... elems>
inline constexpr vlist<elems...> vals;

template <size_t n>
inline constexpr auto indices = [] {
  auto cb = []<size_t... i>(std::index_sequence<i...>) {
    return best::vals<i...>;
  };
  return cb(std::make_index_sequence<n>{});
}();

/// Variadic version of std::is_same/std::same_as.
template <typename... Ts>
concept same =
    (std::same_as<Ts, typename best::tlist<Ts...>::template type<0, void>> &&
     ...);

/// A type-level type list.
///
/// This type makes it easy to manipulate packs of types as first-class objects.
///
/// Type lists are partially-ordered. If two nth have the same elements, they
/// are equal. If a's elements are a prefix of b's, then a < b. Otherwise, a and
/// b are incomparable.
template <typename... Elems>
class tlist final {
 public:
  constexpr tlist() = default;
  constexpr tlist(const tlist&) = default;
  constexpr tlist& operator=(const tlist&) = default;
  constexpr tlist(tlist&&) = default;
  constexpr tlist& operator=(tlist&&) = default;

  /// A stateless global mutable variable of this list's type.
  inline static tlist global;

  /// The number of elements in this list.
  static constexpr size_t size() { return sizeof...(Elems); }
  static constexpr bool is_empty() { return size() == 0; }

  /// Returns whether every element of this list is a value type.
  static constexpr bool is_values() { return (value_trait<Elems> && ...); }

  /// Concatenates several lists with this one.
  ///
  /// If an argument to this type is not a tlist, it is treated as a one-element
  /// tlist.
  static constexpr auto concat(auto... those) {
    return tlist_internal::concat<Elems...>(those...);
  }

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
  /// Applies `cb` to each type in this list and returns a new list resulting
  /// from applying `cb`.
  ///
  /// cb's operator() must take zero arguments and exactly one template
  /// parameter. It must return a tlist; those tlists are flattened into a
  /// single tlist.
  template <auto cb>
  static constexpr decltype(auto) map()
    requires type_callable<decltype(cb)>
  {
    return vals<best::call<Elems>(cb)...>;
  };

  /// Like map(), but takes a template instead; the elements of the resulting
  /// list are Trait<Elems>...
  template <template <typename> typename Trait>
  static constexpr decltype(auto) map() {
    return types<Trait<Elems>...>;
  };

  /// Applies `cb` to each type's ::value in this list and returns a new list
  /// whose elements are the result of each function call.
  template <auto cb>
  static constexpr decltype(auto) map()
    requires value_callable<decltype(cb)>
  {
    return vals<best::call(cb, Elems::value)...>;
  };

  /// Like map(), but takes a template instead; the elements of the resulting
  /// list are Trait<Elems>...
  template <template <auto> typename Trait>
  static constexpr decltype(auto) map()
    requires(is_values())
  {
    return types<Trait<Elems::values>...>;
  };

  /// Applies `cb` to each type in this list, passing each type to `cb` as
  /// a template parameter.
  static constexpr void each(auto cb)
    requires type_callable<decltype(cb)>
  {
    (best::call<Elems>(cb), ...);
  };

  /// Applies `cb` to each type's ::value in this list.
  static constexpr void each(auto cb)
    requires value_callable<decltype(cb)>
  {
    (best::call(cb, Elems::value), ...);
  };

  /// Applies `cb` to *every* type in this list at once.
  static constexpr decltype(auto) apply(auto cb)
    requires types_callable<decltype(cb)>
  {
    return best::call<Elems...>(cb);
  };

  /// Applies `cb` to *every* type's ::value in this list at once.
  static constexpr decltype(auto) apply(auto cb)
    requires values_callable<decltype(cb)>
  {
    return best::call(cb, Elems::value...);
  };

  /// Reduces this list by applying op to each type's ::value.
  template <best::op op>
  static constexpr decltype(auto) reduce()
    requires(is_values())
  {
    return best::operate<op>(Elems::value...);
  };

  /// Reduces this list by applying &&.
  static constexpr decltype(auto) all()
    requires(is_values())
  {
    return reduce<best::op::AndAnd>();
  };

  /// Reduces this list by applying ||.
  static constexpr decltype(auto) any()
    requires(is_values())
  {
    return reduce<best::op::OrOr>();
  };

  /// Gets the type of the nth element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless
  /// Default is specified.
  template <size_t n, typename Default = tlist_internal::strict>
  using type = tlist_internal::nth<n, Default, Elems...>;

  /// Gets the value of the nth element of this tag.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless
  /// default_ is specified.
  template <size_t n, auto default_ = tlist_internal::strict{}>
  static constexpr auto value =
      type<n, std::conditional_t<
                  std::is_same_v<decltype(default_), tlist_internal::strict>,
                  tlist_internal::strict, val<default_>>>::value;

  /// Slices into Elems with `bounds` and returns a new tlist with the
  /// corresponding types.
  ///
  /// Produces an SFINAE error when out-of-bounds, unless
  /// Default is specified.
  template <best::bounds bounds, typename Default = tlist_internal::strict>
  static constexpr tlist_internal::slice<bounds, Default, tlist> at() {
    return {};
  }

  /// If list is a prefix of *this, returns a new tlist with those elements
  /// chopped off.
  template <typename Default = tlist_internal::strict>
  static constexpr auto trim_prefix(auto list) {
    using D =
        std::conditional_t<std::is_same_v<Default, tlist_internal::strict>,
                           tlist, Default>;
    using Out = decltype(list.remove_prefix_from(tlist{}));
    if constexpr (std::is_void_v<Out>) {
      return D{};
    } else {
      return Out{};
    }
  }

  /// If list is a suffix of *this, returns a new tlist with those elements
  /// chopped off.
  template <typename Default = tlist_internal::strict>
  static constexpr auto trim_suffix(auto list) {
    using D =
        std::conditional_t<std::is_same_v<Default, tlist_internal::strict>,
                           tlist, Default>;
    using Out = decltype(list.remove_suffix_from(tlist{}));
    if constexpr (std::is_void_v<Out>) {
      return D{};
    } else {
      return Out{};
    }
  }

  /// Returns the first index of this list that satisfies the given type
  /// predicate.
  static constexpr best::container_internal::option<size_t> index(auto pred)
    requires type_callable<decltype(pred)>
  {
    size_t n = -1;
    if (((++n, best::call<Elems>(pred)) || ...)) {
      return n;
    }
    return {};
  }

  /// Returns the first index of this list that satisfies the given predicate.
  static constexpr best::container_internal::option<size_t> index(auto pred)
    requires value_callable<decltype(pred)>
  {
    return index([&]<typename V> { return pred(V::value); });
  }

  /// Returns the first index of this list that equals the given type.
  template <typename T>
  static constexpr best::container_internal::option<size_t> index() {
    return index([]<typename U> { return std::same_as<T, U>; });
  }

  /// Returns the first index of this list that matches the given bool-returning
  /// value trait.
  template <template <typename> typename Trait>
  static constexpr best::container_internal::option<size_t> index()
    requires(... && value_trait<Trait<Elems>>)
  {
    return index([]<typename U> { return Trait<U>::value; });
  }

  /// Returns the first index of this list that equals the given type.
  static constexpr best::container_internal::option<size_t> index(auto value)
    requires(!value_callable<decltype(value)> && ... &&
             best::equatable<decltype(value), decltype(Elems::value)>)
  {
    return index([&](auto that) { return value == that; });
  }

  /// Returns the unique index of this list that satisfies the given type
  /// predicate.
  static constexpr best::container_internal::option<size_t> unique_index(
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

  /// Returns the unique index of this list that satisfies or equals the
  /// predicate.
  static constexpr best::container_internal::option<size_t> unique_index(
      auto pred)
    requires value_callable<decltype(pred)>
  {
    return unique_index([&]<typename V> { return pred(V::value); });
  }

  /// Returns the unique index of this list that equals the given type.
  template <typename T>
  static constexpr best::container_internal::option<size_t> unique_index() {
    return unique_index([]<typename U> { return std::same_as<T, U>; });
  }

  /// Returns the unique index of this list that matches the given
  /// bool-returning value trait.
  template <template <typename> typename Trait>
  static constexpr best::container_internal::option<size_t> unique_index()
    requires(... && value_trait<Trait<Elems>>)
  {
    return unique_index([]<typename U> { return Trait<U>::value; });
  }

  /// Returns the unique index of this list that equals the given type.
  static constexpr best::container_internal::option<size_t> unique_index(
      auto value)
    requires(!value_callable<decltype(value)> && ... &&
             best::equatable<decltype(value), decltype(Elems::value)>)
  {
    return unique_index([&](auto that) { return value == that; });
  }

  constexpr bool operator==(tlist) const { return true; }
  template <typename... Those>

  constexpr bool operator==(tlist<Those...>) const {
    return false;
  }

  template <typename... Those>
  constexpr std::partial_ordering operator<=>(tlist<Those...> that) const {
    if constexpr (tlist{} == that) {
      return std::partial_ordering::equivalent;
    } else if constexpr (!std::is_void_v<decltype(remove_prefix_from(that))>) {
      return std::partial_ordering::less;
    } else if constexpr (!std::is_void_v<decltype(that.remove_prefix_from(
                             tlist{}))>) {
      return std::partial_ordering::greater;
    } else {
      return std::partial_ordering::unordered;
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

  template <typename... Extra>
  static constexpr tlist<Extra...> remove_suffix_from(
      tlist<Extra..., Elems...>) {
    return {};
  }

  static constexpr void remove_prefix_from(auto) {}
  static constexpr void remove_suffix_from(auto) {}
};
}  // namespace best

#endif  // BEST_META_TLIST_H_