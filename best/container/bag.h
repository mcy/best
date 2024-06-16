#ifndef BEST_CONTAINER_BAG_H_
#define BEST_CONTAINER_BAG_H_

#include <compare>

#include "best/container/internal/bag.h"
#include "best/container/object.h"
#include "best/meta/concepts.h"
#include "best/meta/ebo.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

//! A product type, like `std::tuple`.
//!
//! `best::bag` is the `best` tool for representing a heterogenous sequence.
//!
//! `best::bag` tries to approximate the spirit of Rust tuples, where it makes
//! sense.

namespace best {
/// # `best::bag`
///
/// A runtime bag of heterogenous things. Note that the semantics of `best::bag`
/// for non-object types does not match `std::tuple`. Instead, a `best::bag`
/// models a sequence of `best::object`s. In particular, a `best::bag<T>` and
/// a `best::choice<T>` have the same internal semantics.
///
/// ## Construction
///
/// `best::bag`s are constructed just the same way as ordinary `std::tuples`:
///
/// ```
/// best::bag things{1, &x, 5.6, false};
/// ```
///
/// You can, naturally, specify the type of the thing being put into the bag.
/// This is necessary when creating a tuple of references.
///
/// NOTE: Unlike `std::tuple`, assigning `best::bag` DOES NOT assign through;
/// instead, it rebinds, like all other `best` containers.
///
/// ## Access
///
/// Accessing the elements of a bag is done through either `at()` or
/// `operator[]`. They are semantically identical, but different ones are easier
/// to call in different contexts.
///
/// ```
/// best::bag things{1, &x, 5.6, false};
/// things[best::index<0>]++;
/// things.at<1>()->bonk();
/// ```
///
/// ## Other Features
///
/// - `best::bag` supports structured bindings.
/// - `best::bag` is comparable in the obvious way.
/// - `best::bag<>` is guaranteed to be an empty type.
/// - `best::bag` is trivial when all of its elements are trivial.
template <typename... Elems>
class bag final
    : bag_internal::impl<decltype(best::indices<sizeof...(Elems)>), Elems...> {
 public:
  /// # `bag::types`
  ///
  /// A `tlist` of the elements in this bag.
  static constexpr auto types = best::types<Elems...>;

  /// # `bag::indices`
  ///
  /// A `vlist` of the indices of the elements in this bag.
  static constexpr auto indices = best::indices<types.size()>;

  /// # `bag::type<n>`
  ///
  /// Gets the nth type in this choice.
  template <size_t n>
  using type = decltype(types)::template type<n>;

  // clang-format off
  template <size_t n> using cref = best::as_ref<const type<n>>;
  template <size_t n> using ref = best::as_ref<type<n>>;
  template <size_t n> using crref = best::as_rref<const type<n>>;
  template <size_t n> using rref = best::as_rref<type<n>>;
  template <size_t n> using cptr = best::as_ptr<const type<n>>;
  template <size_t n> using ptr = best::as_ptr<type<n>>;
  // clang-format on

 private:
  using impl = bag_internal::impl<decltype(indices), Elems...>;

 public:
  /// # `bag::bag()`.
  ///
  /// Default constructs each element of the bag.
  constexpr bag() = default;

  /// # `bag::bag(bag)`.
  ///
  /// These forward to the appropriate move/copy constructor of each element.
  constexpr bag(const bag&) = default;
  constexpr bag& operator=(const bag&) = default;
  constexpr bag(bag&&) = default;
  constexpr bag& operator=(bag&&) = default;

 public:
  /// # `bag::bag(...)`
  ///
  /// Constructs a bag by initializing each element from the corresponding
  /// argument.
  constexpr bag(auto&&... args)
    requires(best::constructible<Elems, decltype(args)> && ...)
      : impl{{best::in_place, best::in_place, BEST_FWD(args)}...} {}

  /// # `bag[index<n>]`
  ///
  /// Returns the `n`th element.
  // clang-format off
  template <size_t n> constexpr cref<n> operator[](best::index_t<n> idx) const&;
  template <size_t n> constexpr ref<n> operator[](best::index_t<n> idx) &;
  template <size_t n> constexpr crref<n> operator[](best::index_t<n> idx) const&&;
  template <size_t n> constexpr rref<n> operator[](best::index_t<n> idx) &&;
  // clang-format on

  /// # `bag::at(index<n>)`
  ///
  /// Identical to `operator[]` in all ways.
  // clang-format off
  template <size_t n> constexpr cref<n> at(best::index_t<n> = {}) const&;
  template <size_t n> constexpr ref<n> at(best::index_t<n> = {}) &;
  template <size_t n> constexpr crref<n> at(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr rref<n> at(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `bag::at(index<n>)`
  ///
  /// Identical to `operator[]` in all ways except that when we would return a
  /// void type , we instead return a `best::empty` values (not a reference).
  ///
  /// This function exists for the benefit of structured bindings.
  // clang-format off
  template <size_t n> constexpr decltype(auto) get(best::index_t<n> = {}) const&;
  template <size_t n> constexpr decltype(auto) get(best::index_t<n> = {}) &;
  template <size_t n> constexpr decltype(auto) get(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr decltype(auto) get(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `bag::apply()`
  ///
  /// Calls `f` with a pack of references of the elements of this tuple.
  /// Elements of void type are replaced with `best::empty` values, as in
  /// `get()`.
  constexpr decltype(auto) apply(auto&& f) const&;
  constexpr decltype(auto) apply(auto&& f) &;
  constexpr decltype(auto) apply(auto&& f) const&&;
  constexpr decltype(auto) apply(auto&& f) &&;

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, const bag& bag) {
    os << "(";
    bool first = true;
    bag.indices.apply([&]<typename... I> {
      (void)((std::exchange(first, false)
                  ? os << bag.get(best::index<I::value>)
                  : os << ", " << bag.get(best::index<I::value>)),
             ...);
    });
    return os << ")";
  }

  // Comparisons.
  template <typename... Us>
  constexpr bool operator==(const bag<Us...>& that) const
    requires(best::equatable<Elems, Us> && ...)
  {
    return indices.apply([&]<typename... I> {
      return (... &&
              (get(best::index<I::value>) == that.get(best::index<I::value>)));
    });
  }

  template <typename... Us>
  constexpr std::common_comparison_category_t<best::order_type<Elems, Us>...>
  operator<=>(const choice<Us...>& that) const
    requires(best::comparable<Elems, Us> && ...)
  {
    using Output =
        std::common_comparison_category_t<best::order_type<Elems, Us>...>;

    return indices.apply([&]<typename... I> {
      Output result = Output::equivalent;
      return (..., (result == 0 ? result = at(best::index<I::value>) <=>
                                           that.at(best::index<I::value>)
                                : result));
    });
  }

 private:
  constexpr const bag&& moved() const {
    return static_cast<const bag&&>(*this);
  }
  constexpr bag&& moved() { return static_cast<bag&&>(*this); }
};

template <typename... Elems>
bag(Elems&&...) -> bag<Elems...>;

/// --- IMPLEMENTATION DETAILS BELOW ---

namespace bag_internal {}  // namespace bag_internal

template <typename... A>
template <size_t n>
constexpr bag<A...>::cref<n> bag<A...>::operator[](
    best::index_t<n> idx) const& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<const B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::ref<n> bag<A...>::operator[](best::index_t<n> idx) & {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::crref<n> bag<A...>::operator[](
    best::index_t<n> idx) const&& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<const T>>(
      *static_cast<const B&>(*this).get());
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::rref<n> bag<A...>::operator[](best::index_t<n> idx) && {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<T>>(*static_cast<B&>(*this).get());
}

template <typename... A>
template <size_t n>
constexpr bag<A...>::cref<n> bag<A...>::at(best::index_t<n> idx) const& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<const B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::ref<n> bag<A...>::at(best::index_t<n> idx) & {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::crref<n> bag<A...>::at(best::index_t<n> idx) const&& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<const T>>(
      *static_cast<const B&>(*this).get());
}
template <typename... A>
template <size_t n>
constexpr bag<A...>::rref<n> bag<A...>::at(best::index_t<n> idx) && {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<T>>(*static_cast<B&>(*this).get());
}

template <typename... A>
template <size_t n>
constexpr decltype(auto) bag<A...>::get(best::index_t<n> idx) const& {
  using T = type<n>;
  if constexpr (best::void_type<T>) {
    return best::empty{};
  } else {
    using B = best::ebo<best::object<T>, T, n>;
    return *static_cast<const B&>(*this).get();
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) bag<A...>::get(best::index_t<n> idx) & {
  using T = type<n>;
  if constexpr (best::void_type<T>) {
    return best::empty{};
  } else {
    using B = best::ebo<best::object<T>, T, n>;
    return *static_cast<const B&>(*this).get();
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) bag<A...>::get(best::index_t<n> idx) const&& {
  using T = type<n>;
  if constexpr (best::void_type<T>) {
    return best::empty{};
  } else {
    using B = best::ebo<best::object<T>, T, n>;
    return static_cast<best::as_rref<const T>>(
        *static_cast<const B&>(*this).get());
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) bag<A...>::get(best::index_t<n> idx) && {
  using T = type<n>;
  if constexpr (best::void_type<T>) {
    return best::empty{};
  } else {
    using B = best::ebo<best::object<T>, T, n>;
    return static_cast<best::as_rref<T>>(*static_cast<B&>(*this).get());
  }
}

template <typename... A>
constexpr decltype(auto) bag<A...>::apply(auto&& f) const& {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) bag<A...>::apply(auto&& f) & {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) bag<A...>::apply(auto&& f) const&& {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), moved().get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) bag<A...>::apply(auto&& f) && {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), moved().get(best::index<I::value>)...);
  });
}
}  // namespace best

// Enable structured bindings.
namespace std {
template <typename... Elems>
struct tuple_size<::best::bag<Elems...>> {
  static constexpr size_t value = sizeof...(Elems);
};
template <size_t i, typename... Elems>
struct tuple_element<i, ::best::bag<Elems...>> {
  using type = ::best::bag<Elems...>::template type<i>;
};
}  // namespace std

#endif  // BEST_CONTAINER_BAG_H_