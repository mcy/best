#ifndef BEST_CONTAINER_ROW_H_
#define BEST_CONTAINER_ROW_H_

#include <compare>
#include <type_traits>

#include "best/base/fwd.h"
#include "best/container/internal/row.h"
#include "best/container/object.h"
#include "best/meta/concepts.h"
#include "best/meta/ebo.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

//! A product type, like `std::tuple`.
//!
//! `best::row` is the `best` tool for representing a heterogenous sequence.
//!
//! `best::row` tries to approximate the spirit of Rust tuples, where it makes
//! sense.

namespace best {
/// # `best::is_row`
///
/// Whether this is a specialization of `best::row`.
template <typename T>
concept is_row = requires {
  {
    best::as_deref<T>::types.apply([]<typename... U>() -> best::row<U...> {})
  } -> best::same<T>;
};

/// # `best::row`
///
/// A list of heterogenous things. Note that the semantics of `best::row`
/// for non-object types does not match `std::tuple`. Instead, a `best::row`
/// models a sequence of `best::object`s. In particular, a `best::row<T>` and
/// a `best::choice<T>` have the same internal semantics.
///
/// This type is named `best::row` instead of `best::tuple` because it is a
/// common "type wart" that makes function signatures longer than they need to
/// be, so saving single characters can help a lot!
///
/// ## Construction
///
/// `best::row`s are constructed just the same way as ordinary `std::tuples`:
///
/// ```
/// best::row things{1, &x, 5.6, false};
/// ```
///
/// You can, naturally, specify the type of the thing being put into the row.
/// This is necessary when creating a tuple of references.
///
/// NOTE: Unlike `std::tuple`, assigning `best::row` DOES NOT assign through;
/// instead, it rebinds, like all other `best` containers.
///
/// ## Access
///
/// Accessing the elements of a row is done through either `at()` or
/// `operator[]`. They are semantically identical, but different ones are easier
/// to call in different contexts.
///
/// ```
/// best::row things{1, &x, 5.6, false};
/// things[best::index<0>]++;
/// things.at<1>()->bonk();
/// ```
///
/// ## Other Features
///
/// - `best::row` supports structured bindings.
/// - `best::row` is comparable in the obvious way.
/// - `best::row<>` is guaranteed to be an empty type.
/// - `best::row` is trivial when all of its elements are trivial.
template <typename... Elems>
class row final
    : row_internal::impl<decltype(best::indices<sizeof...(Elems)>), Elems...> {
 public:
  /// # `row::types`
  ///
  /// A `tlist` of the elements in this row.
  static constexpr auto types = best::types<Elems...>;

  /// # `row::indices`
  ///
  /// A `vlist` of the indices of the elements in this row.
  static constexpr auto indices = best::indices<types.size()>;

  /// # `row::type<n>`
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
  using impl = row_internal::impl<decltype(indices), Elems...>;

 public:
  /// # `row::row()`.
  ///
  /// Default constructs each element of the row.
  constexpr row() = default;

  /// # `row::row(row)`.
  ///
  /// These forward to the appropriate move/copy constructor of each element.
  constexpr row(const row&) = default;
  constexpr row& operator=(const row&) = default;
  constexpr row(row&&) = default;
  constexpr row& operator=(row&&) = default;

 public:
  /// # `row::row(...)`
  ///
  /// Constructs a row by initializing each element from the corresponding
  /// argument.
  constexpr row(auto&&... args)
    requires(best::constructible<Elems, decltype(args)> && ...)
      : impl{{best::in_place, best::in_place, BEST_FWD(args)}...} {}

  /// # `row[index<n>]`
  ///
  /// Returns the `n`th element.
  // clang-format off
  template <size_t n> constexpr cref<n> operator[](best::index_t<n> idx) const&;
  template <size_t n> constexpr ref<n> operator[](best::index_t<n> idx) &;
  template <size_t n> constexpr crref<n> operator[](best::index_t<n> idx) const&&;
  template <size_t n> constexpr rref<n> operator[](best::index_t<n> idx) &&;
  // clang-format on

  /// # `row::at(index<n>)`
  ///
  /// Identical to `operator[]` in all ways.
  // clang-format off
  template <size_t n> constexpr cref<n> at(best::index_t<n> = {}) const&;
  template <size_t n> constexpr ref<n> at(best::index_t<n> = {}) &;
  template <size_t n> constexpr crref<n> at(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr rref<n> at(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `row::at(index<n>)`
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

  /// # `row::apply()`
  ///
  /// Calls `f` with a pack of references of the elements of this tuple.
  /// Elements of void type are replaced with `best::empty` values, as in
  /// `get()`.
  constexpr decltype(auto) apply(auto&& f) const&;
  constexpr decltype(auto) apply(auto&& f) &;
  constexpr decltype(auto) apply(auto&& f) const&&;
  constexpr decltype(auto) apply(auto&& f) &&;

  /// # `row::forward()`
  ///
  /// Constructs a corresponding `best::row_forward()` for this row. The
  /// elements of the resulting forwarded row will be the result of calling
  /// `get()`: references, except if an element is of void type, a best::empty
  /// value instead.
  constexpr auto forward() const&;
  constexpr auto forward() &;
  constexpr auto forward() const&&;
  constexpr auto forward() &&;

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, const row& row) {
    os << "(";
    bool first = true;
    row.indices.apply([&]<typename... I> {
      (void)((std::exchange(first, false)
                  ? os << row.get(best::index<I::value>)
                  : os << ", " << row.get(best::index<I::value>)),
             ...);
    });
    return os << ")";
  }

  friend void BestFmt(auto& fmt, const row& row)
    requires requires(best::object<Elems>... els) { (fmt.format(els), ...); }
  {
    auto tup = fmt.tuple();
    row.apply([&](const auto&... x) { (tup.entry(x), ...); });
  }

  template <typename Q>
  friend constexpr void BestFmtQuery(Q& query, row*) {
    query.supports_width = (query.template of<Elems>.supports_width || ...);
    query.supports_prec = (query.template of<Elems>.supports_prec || ...);
    query.uses_method = [](auto r) {
      return (Q::template of<Elems>.uses_method(r) && ...);
    };
  }

  // Comparisons.
  template <typename... Us>
  constexpr bool operator==(const row<Us...>& that) const
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
  constexpr const row&& moved() const {
    return static_cast<const row&&>(*this);
  }
  constexpr row&& moved() { return static_cast<row&&>(*this); }
};

template <typename... Elems>
row(Elems&&...) -> row<std::remove_cvref_t<Elems>...>;

/// # `best::row_forward`
///
/// A wrapper over a `best::row<>` that will instruct the various "in place
/// constructor" constructors throughout `best` to construct using the elements
/// of the wrapped row.
template <typename... Args>
struct row_forward final {
  best::row<Args...> row;

  template <best::constructible<Args...> T>
  constexpr operator T() && {
    return std::move(row).apply(
        [](auto&&... args) { return T(BEST_FWD(args)...); });
  }
};

/// --- IMPLEMENTATION DETAILS BELOW ---

template <typename... A>
constexpr auto row<A...>::forward() const& {
  return apply([](auto&&... args) {
    return row_forward<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::forward() & {
  return apply([](auto&&... args) {
    return row_forward<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::forward() const&& {
  return moved().apply([](auto&&... args) {
    return row_forward<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::forward() && {
  return moved().apply([](auto&&... args) {
    return row_forward<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}

template <typename... A>
template <size_t n>
constexpr row<A...>::cref<n> row<A...>::operator[](
    best::index_t<n> idx) const& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<const B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr row<A...>::ref<n> row<A...>::operator[](best::index_t<n> idx) & {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr row<A...>::crref<n> row<A...>::operator[](
    best::index_t<n> idx) const&& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<const T>>(
      *static_cast<const B&>(*this).get());
}
template <typename... A>
template <size_t n>
constexpr row<A...>::rref<n> row<A...>::operator[](best::index_t<n> idx) && {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<T>>(*static_cast<B&>(*this).get());
}

template <typename... A>
template <size_t n>
constexpr row<A...>::cref<n> row<A...>::at(best::index_t<n> idx) const& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<const B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr row<A...>::ref<n> row<A...>::at(best::index_t<n> idx) & {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return *static_cast<B&>(*this).get();
}
template <typename... A>
template <size_t n>
constexpr row<A...>::crref<n> row<A...>::at(best::index_t<n> idx) const&& {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<const T>>(
      *static_cast<const B&>(*this).get());
}
template <typename... A>
template <size_t n>
constexpr row<A...>::rref<n> row<A...>::at(best::index_t<n> idx) && {
  using T = type<n>;
  using B = best::ebo<best::object<T>, T, n>;
  return static_cast<best::as_rref<T>>(*static_cast<B&>(*this).get());
}

template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) const& {
  if constexpr (best::void_type<type<n>>) {
    return best::empty{};
  } else {
    return at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) & {
  if constexpr (best::void_type<type<n>>) {
    return best::empty{};
  } else {
    return at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) const&& {
  if constexpr (best::void_type<type<n>>) {
    return best::empty{};
  } else {
    return moved().at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) && {
  if constexpr (best::void_type<type<n>>) {
    return best::empty{};
  } else {
    return moved().at(idx);
  }
}

template <typename... A>
constexpr decltype(auto) row<A...>::apply(auto&& f) const& {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) row<A...>::apply(auto&& f) & {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) row<A...>::apply(auto&& f) const&& {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), moved().get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) row<A...>::apply(auto&& f) && {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f), moved().get(best::index<I::value>)...);
  });
}
}  // namespace best

// Enable structured bindings.
namespace std {
template <typename... Elems>
struct tuple_size<::best::row<Elems...>> {
  static constexpr size_t value = sizeof...(Elems);
};
template <size_t i, typename... Elems>
struct tuple_element<i, ::best::row<Elems...>> {
  using type = ::best::row<Elems...>::template type<i>;
};
}  // namespace std

#endif  // BEST_CONTAINER_ROW_H_