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

#ifndef BEST_CONTAINER_ROW_H_
#define BEST_CONTAINER_ROW_H_

#include "best/base/fwd.h"
#include "best/base/ord.h"
#include "best/container/internal/row.h"
#include "best/container/object.h"
#include "best/meta/empty.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"
#include "best/meta/taxonomy.h"
#include "best/meta/tlist.h"

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
concept is_row = requires(T t) {
  {
    t.types.apply([]<typename... U>() -> best::row<U...> { std::abort(); })
  } -> best::same<best::as_auto<T>>;
};

/// # `best::args<...>`
///
/// A wrapper over a `best::row<>` that will instruct the various "in place
/// constructor" constructors throughout `best` to construct using the elements
/// of the wrapped row.
///
/// `best::args`'s deduction guides will always cause it to capture references.
template <typename... Args>
struct args final {
  best::row<Args...> row;

  constexpr explicit args(Args... args) : row(BEST_FWD(args)...) {}
  constexpr explicit args(best::row<Args...> args) : row(BEST_FWD(args)) {}

  template <best::constructible<Args...> T>
  constexpr operator T() && {
    return std::move(row).apply(
        [](auto&&... args) { return T(BEST_FWD(args)...); });
  }

  friend void BestFmt(auto& fmt, const args& args)
    requires requires { (fmt.format(args.row)); }
  {
    fmt.format(args.row);
  }

  friend constexpr void BestFmtQuery(auto& query, args*) {
    query = query.template of<best::row<Args...>>;
  }
};
template <typename... Args>
args(Args&&...) -> args<Args&&...>;

/// # `best::row<...>`
///
/// A list of heterogenous thing0s. Note that the semantics of `best::row`
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

  /// # `row::selectable<T>`
  ///
  /// Whether an element of this row can be selected by the type `T`, in other
  /// words, if `T` is among `Elems`, *or* there is a type `U` among `Elems`
  /// such that `U::BestRowKey` is `T`.
  template <typename T>
  static constexpr bool selectable =
      row_internal::lookup<T>.template count<T>() > 0;

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
    requires(best::constructible<Elems, decltype(args)> && ...) &&
            (!best::same<decltype(args), best::as_rref<Elems>> || ...)
      : impl{{best::object<Elems>(best::in_place, BEST_FWD(args))}...} {}
  constexpr row(devoid<Elems>&&... args)
    requires(sizeof...(args) > 0)
      : impl{{best::object<Elems>(best::in_place, BEST_FWD(args))}...} {}

  constexpr row(best::bind_t, auto&&... args)
    requires(best::constructible<Elems, decltype(args)> && ...) &&
            (!best::same<decltype(args), best::as_rref<Elems>> || ...)
      : impl{{best::object<Elems>(best::in_place, BEST_FWD(args))}...} {}
  constexpr row(best::bind_t, devoid<Elems>&&... args)
    requires(sizeof...(args) > 0)
      : impl{{best::object<Elems>(best::in_place, BEST_FWD(args))}...} {}

  /// # `row::size()`
  ///
  /// Returns the number of elements in this row.
  constexpr static size_t size() { return types.size(); }

  /// # `row::is_empty()`
  ///
  /// Returns whether this is the empty row `best::row<>`.
  constexpr static bool is_empty() { return types.size() == 0; }

  /// # `row::as_ref()`
  ///
  /// Creates a row of references to the elements of this row.
  constexpr best::row<best::as_ref<const Elems>...> as_ref() const&;
  constexpr best::row<best::as_ref<Elems>...> as_ref() &;
  constexpr best::row<best::as_rref<const Elems>...> as_ref() const&&;
  constexpr best::row<best::as_rref<Elems>...> as_ref() &&;

  /// # `row[index<n>]`, `row[best::values<bounds{...}>]`
  ///
  /// Returns a reference to the `n`th element, or a subrange as a row (values
  /// are copied/moved as appropriate).
  // clang-format off
  template <size_t n> constexpr cref<n> operator[](best::index_t<n>) const&;
  template <size_t n> constexpr ref<n> operator[](best::index_t<n>) &;
  template <size_t n> constexpr crref<n> operator[](best::index_t<n>) const&&;
  template <size_t n> constexpr rref<n> operator[](best::index_t<n>) &&;
  template <best::bounds range> constexpr auto operator[](best::vlist<range>) const&;
  template <best::bounds range> constexpr auto operator[](best::vlist<range>) &;
  template <best::bounds range> constexpr auto operator[](best::vlist<range>) const&&;
  template <best::bounds range> constexpr auto operator[](best::vlist<range>) &&;
  // clang-format on

  /// # `row::at(index<n>)`, `row::at(best::values<bounds{...}>)`
  ///
  /// Identical to `operator[]` in all ways.
  // clang-format off
  template <size_t n> constexpr cref<n> at(best::index_t<n> = {}) const&;
  template <size_t n> constexpr ref<n> at(best::index_t<n> = {}) &;
  template <size_t n> constexpr crref<n> at(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr rref<n> at(best::index_t<n> = {}) &&;
  template <best::bounds range> constexpr auto at(best::vlist<range> = {}) const&;
  template <best::bounds range> constexpr auto at(best::vlist<range> = {}) &;
  template <best::bounds range> constexpr auto at(best::vlist<range> = {}) const&&;
  template <best::bounds range> constexpr auto at(best::vlist<range> = {}) &&;
  // clang-format on

  /// # `row::object(index<n>)`
  ///
  /// Identical to `operator[]`, but returns a reference to a `best::object`.
  // clang-format off
  template <size_t n> constexpr const best::object<type<n>>& object(best::index_t<n> = {}) const&;
  template <size_t n> constexpr best::object<type<n>>& object(best::index_t<n> = {}) &;
  template <size_t n> constexpr const best::object<type<n>>&& object(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr best::object<type<n>>&& object(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `row::get(index<n>)`
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

  /// # `row::select()`
  ///
  /// Returns a row containing all types in this row that are either of that
  /// type, or which have a member alias named `BestRowKey` of that type. They
  /// are returned in the order they occur in this row.
  // clang-format off
  template <typename T> constexpr auto select(best::tlist<T> idx = {}) const&;
  template <typename T> constexpr auto select(best::tlist<T> idx = {}) & ;
  template <typename T> constexpr auto select(best::tlist<T> idx = {}) const&&; 
  template <typename T> constexpr auto select(best::tlist<T> idx = {}) &&;
  // clang-format on

  /// # `row::select_indices()`
  ///
  /// Returns a `best::vlist` of the indices of the elements that
  /// `row::select()` returns.
  template <typename T>
  static constexpr auto select_indices(best::tlist<T> idx = {});

  /// # `row::first()`, `row::second()`, `row::last()`
  ///
  /// Gets the first, second, or last value; helper for `std::pair`-like uses.
  // clang-format off
  constexpr decltype(auto) first() const& requires (!types.is_empty());
  constexpr decltype(auto) first() & requires (!types.is_empty());
  constexpr decltype(auto) first() const&& requires (!types.is_empty());
  constexpr decltype(auto) first() && requires (!types.is_empty());
  constexpr decltype(auto) second() const& requires (types.size() >= 2);
  constexpr decltype(auto) second() & requires (types.size() >= 2);
  constexpr decltype(auto) second() const&& requires (types.size() >= 2);
  constexpr decltype(auto) second() && requires (types.size() >= 2);
  constexpr decltype(auto) last() const& requires (!types.is_empty());
  constexpr decltype(auto) last() & requires (!types.is_empty());
  constexpr decltype(auto) last() const&& requires (!types.is_empty());
  constexpr decltype(auto) last() && requires (!types.is_empty());
  // clang-format on

  /// # `tlist::join()`, `tlist::operator+`
  ///
  /// Concatenates several rows with this one.
  ///
  /// This is also made available via `operator+`, but beware: using a fold with
  /// `operator+` here is quadratic; you should generally prefer `join()` for
  /// variadic cases.
  constexpr auto join(best::is_row auto&&...) const&;
  constexpr auto join(best::is_row auto&&...) &;
  constexpr auto join(best::is_row auto&&...) const&&;
  constexpr auto join(best::is_row auto&&...) &&;
  constexpr auto operator+(best::is_row auto&&) const&;
  constexpr auto operator+(best::is_row auto&&) &;
  constexpr auto operator+(best::is_row auto&&) const&&;
  constexpr auto operator+(best::is_row auto&&) &&;

  /// # `row::push()`
  ///
  /// Returns a new row with an extra element at the end.
  ///
  /// Like other mutation functions, there are three overloads. One takes a
  /// single value, and the resulting new row element type is `auto`-deduced.
  /// One takes `best::bind`, and the resulting new row element type will be
  /// a reference. One takes `best::types<T>`, and resulting value is `T`. To
  /// construct with multiple arguments, use a `best::args`.
  ///
  /// All other values are copied/moved as appropriate from this row.
  // clang-format off
  constexpr auto push(auto&&) const&;
  constexpr auto push(auto&&) &;
  constexpr auto push(auto&&) const&&;
  constexpr auto push(auto&&) &&;
  constexpr auto push(best::bind_t, auto&&) const&;
  constexpr auto push(best::bind_t, auto&&) &;
  constexpr auto push(best::bind_t, auto&&) const&&;
  constexpr auto push(best::bind_t, auto&&) &&;
  template <typename T> constexpr auto push(best::tlist<T>, auto&&) const&;
  template <typename T> constexpr auto push(best::tlist<T>, auto&&) &;
  template <typename T> constexpr auto push(best::tlist<T>, auto&&) const&&;
  template <typename T> constexpr auto push(best::tlist<T>, auto&&) &&;
  // clang-format on

  /// # `row::insert()`
  ///
  /// Returns a new row with an extra element inserted at index `n`.
  ///
  /// This function has the same overload set as `row::push()`.
  // clang-format off
  template <size_t n> constexpr auto insert(auto&&, best::index_t<n> = {}) const&;
  template <size_t n> constexpr auto insert(auto&&, best::index_t<n> = {}) &;
  template <size_t n> constexpr auto insert(auto&&, best::index_t<n> = {}) const&&;
  template <size_t n> constexpr auto insert(auto&&, best::index_t<n> = {}) &&;
  template <size_t n> constexpr auto insert(best::bind_t, auto&&, best::index_t<n> = {}) const&;
  template <size_t n> constexpr auto insert(best::bind_t, auto&&, best::index_t<n> = {}) &;
  template <size_t n> constexpr auto insert(best::bind_t, auto&&, best::index_t<n> = {}) const&&;
  template <size_t n> constexpr auto insert(best::bind_t, auto&&, best::index_t<n> = {}) &&;
  template <size_t n, typename T> constexpr auto insert(best::tlist<T>, auto&&, best::index_t<n> = {}) const&;
  template <size_t n, typename T> constexpr auto insert(best::tlist<T>, auto&&, best::index_t<n> = {}) &;
  template <size_t n, typename T> constexpr auto insert(best::tlist<T>, auto&&, best::index_t<n> = {}) const&&;
  template <size_t n, typename T> constexpr auto insert(best::tlist<T>, auto&&, best::index_t<n> = {}) &&;
  // clang-format on

  /// # `row::update()`
  ///
  /// Functionally equivalent to `row::insert()`, except it replaces the `n`th
  /// element instead of inserting into the `n`th slot.
  // clang-format off
  template <size_t n> constexpr auto update(auto&&, best::index_t<n> = {}) const&;
  template <size_t n> constexpr auto update(auto&&, best::index_t<n> = {}) &;
  template <size_t n> constexpr auto update(auto&&, best::index_t<n> = {}) const&&;
  template <size_t n> constexpr auto update(auto&&, best::index_t<n> = {}) &&;
  template <size_t n> constexpr auto update(best::bind_t, auto&&, best::index_t<n> = {}) const&;
  template <size_t n> constexpr auto update(best::bind_t, auto&&, best::index_t<n> = {}) &;
  template <size_t n> constexpr auto update(best::bind_t, auto&&, best::index_t<n> = {}) const&&;
  template <size_t n> constexpr auto update(best::bind_t, auto&&, best::index_t<n> = {}) &&;
  template <size_t n, typename T> constexpr auto update(best::tlist<T>, auto&&, best::index_t<n> = {}) const&;
  template <size_t n, typename T> constexpr auto update(best::tlist<T>, auto&&, best::index_t<n> = {}) &;
  template <size_t n, typename T> constexpr auto update(best::tlist<T>, auto&&, best::index_t<n> = {}) const&&;
  template <size_t n, typename T> constexpr auto update(best::tlist<T>, auto&&, best::index_t<n> = {}) &&;
  // clang-format on

  /// # `row::splice()`
  ///
  /// Returns a new row with a specific range replaced by the given row.
  /// Values are moved/copied out of this row, and the spliced-in row, as
  /// appropriate.
  ///
  /// A separate set of overloads takes an extra `best::types<...>` argument,
  /// which specifies what the spliced-in types should be independent of the
  /// argument. This list should have the same size as the input row.
  template <best::bounds range>
  constexpr auto splice(best::is_row auto&&, best::vlist<range> = {}) const&;
  template <best::bounds range>
  constexpr auto splice(best::is_row auto&&, best::vlist<range> = {}) &;
  template <best::bounds range>
  constexpr auto splice(best::is_row auto&&, best::vlist<range> = {}) const&&;
  template <best::bounds range>
  constexpr auto splice(best::is_row auto&&, best::vlist<range> = {}) &&;

  template <best::bounds range, typename... Those>
  constexpr auto splice(best::tlist<Those...>, best::is_row auto&&,
                        best::vlist<range> = {}) const&;
  template <best::bounds range, typename... Those>
  constexpr auto splice(best::tlist<Those...>, best::is_row auto&&,
                        best::vlist<range> = {}) &;
  template <best::bounds range, typename... Those>
  constexpr auto splice(best::tlist<Those...>, best::is_row auto&&,
                        best::vlist<range> = {}) const&&;
  template <best::bounds range, typename... Those>
  constexpr auto splice(best::tlist<Those...>, best::is_row auto&&,
                        best::vlist<range> = {}) &&;

  /// # `row::remove()`
  ///
  /// Returns a new row with the `n`th element removed. All other elements are
  /// moved/copied out of this row.
  // clang-format off
  template <size_t n> constexpr auto remove(best::index_t<n> = {}) const&;
  template <size_t n> constexpr auto remove(best::index_t<n> = {}) &;
  template <size_t n> constexpr auto remove(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr auto remove(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `row::erase()`
  ///
  /// Returns a new row with the given range. All other elements are
  /// moved/copied out of this row.
  // clang-format off
  template <best::bounds range> constexpr auto erase(best::vlist<range> = {}) const&;
  template <best::bounds range> constexpr auto erase(best::vlist<range> = {}) &;
  template <best::bounds range> constexpr auto erase(best::vlist<range> = {}) const&&;
  template <best::bounds range> constexpr auto erase(best::vlist<range> = {}) &&;
  // clang-format on

  /// # `row::gather()`
  ///
  /// Selects elements of this row by index to form a new row, by copying/moving
  /// as necessary.
  // clang-format off
  template <size_t... n> constexpr auto gather(best::vlist<n...> = {}) const&;
  template <size_t... n> constexpr auto gather(best::vlist<n...> = {}) &;
  template <size_t... n> constexpr auto gather(best::vlist<n...> = {}) const&&;
  template <size_t... n> constexpr auto gather(best::vlist<n...> = {}) &&;
  // clang-format on

  /// # `row::scatter()`
  ///
  /// Updates elements of this row by index to form a new row, by copying/moving
  /// from this row, and the one provided, as necessary.
  ///
  /// Like `row::splice()`, there is a second overload that allows specifying
  /// explicitly what the scattered types are.
  // clang-format off
  template <size_t... n> constexpr auto scatter(best::is_row auto&&, best::vlist<n...> = {}) const&;
  template <size_t... n> constexpr auto scatter(best::is_row auto&&, best::vlist<n...> = {}) &;
  template <size_t... n> constexpr auto scatter(best::is_row auto&&, best::vlist<n...> = {}) const&&;
  template <size_t... n> constexpr auto scatter(best::is_row auto&&, best::vlist<n...> = {}) &&;
  template <size_t... n, typename... Ts> constexpr auto scatter(best::tlist<Ts...>, best::is_row auto&&, best::vlist<n...> = {}) const&;
  template <size_t... n, typename... Ts> constexpr auto scatter(best::tlist<Ts...>, best::is_row auto&&, best::vlist<n...> = {}) &;
  template <size_t... n, typename... Ts> constexpr auto scatter(best::tlist<Ts...>, best::is_row auto&&, best::vlist<n...> = {}) const&&;
  template <size_t... n, typename... Ts> constexpr auto scatter(best::tlist<Ts...>, best::is_row auto&&, best::vlist<n...> = {}) &&;
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

  /// # `row::as_args()`
  ///
  /// Constructs a corresponding `best::args()` for this row. The elements of
  /// the resulting row will be the result of calling `get()`: references,
  /// except if an element is of void type, a best::empty value instead.
  constexpr auto as_args() const&;
  constexpr auto as_args() &;
  constexpr auto as_args() const&&;
  constexpr auto as_args() &&;

  friend void BestFmt(auto& fmt, const row& row)
    requires requires(best::object<Elems>... els) { (fmt.format(els), ...); }
  {
    auto tup = fmt.tuple();
    return indices.apply([&]<typename... I> {
      return (tup.entry(row.object(best::index<I::value>)), ...);
    });
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
  constexpr auto operator<=>(const choice<Us...>& that) const
    requires(best::comparable<Elems, Us> && ...)
  {
    using Output = best::common_ord<best::order_type<Elems, Us>...>;
    return indices.apply([&]<typename... I> {
      Output result = Output::equivalent;
      return (..., (result == 0 ? result = at(best::index<I::value>) <=>
                                           that.at(best::index<I::value>)
                                : result));
    });
  }
};

template <typename... Elems>
row(Elems&&...) -> row<best::as_auto<Elems>...>;
template <typename... Elems>
row(best::bind_t, Elems&&...) -> row<Elems&&...>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename... A>
constexpr auto row<A...>::as_args() const& {
  return apply([](auto&&... args) {
    return best::args<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::as_args() & {
  return apply([](auto&&... args) {
    return best::args<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::as_args() const&& {
  return BEST_MOVE(*this).apply([](auto&&... args) {
    return best::args<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}
template <typename... A>
constexpr auto row<A...>::as_args() && {
  return BEST_MOVE(*this).apply([](auto&&... args) {
    return best::args<decltype(args)...>{
        row<decltype(args)...>(BEST_FWD(args)...)};
  });
}

template <typename... A>
constexpr best::row<best::as_ref<const A>...> row<A...>::as_ref() const& {
  apply([](auto&&... args) {
    best::row<best::as_ref<const A>...>(BEST_FWD(args)...);
  });
}
template <typename... A>
constexpr best::row<best::as_ref<A>...> row<A...>::as_ref() & {
  apply(
      [](auto&&... args) { best::row<best::as_ref<A>...>(BEST_FWD(args)...); });
}
template <typename... A>
constexpr best::row<best::as_rref<const A>...> row<A...>::as_ref() const&& {
  BEST_MOVE(*this).apply([](auto&&... args) {
    best::row<best::as_rref<const A>...>(BEST_FWD(args)...);
  });
}
template <typename... A>
constexpr best::row<best::as_rref<A>...> row<A...>::as_ref() && {
  BEST_MOVE(*this).apply([](auto&&... args) {
    best::row<best::as_rref<A>...>(BEST_FWD(args)...);
  });
}

template <typename... A>
template <size_t n>
constexpr row<A...>::cref<n> row<A...>::operator[](
    best::index_t<n> idx) const& {
  return *this->get_impl(idx);
}
template <typename... A>
template <size_t n>
constexpr row<A...>::ref<n> row<A...>::operator[](best::index_t<n> idx) & {
  return *this->get_impl(idx);
}
template <typename... A>
template <size_t n>
constexpr row<A...>::crref<n> row<A...>::operator[](
    best::index_t<n> idx) const&& {
  return *BEST_MOVE(this->get_impl(idx));
}
template <typename... A>
template <size_t n>
constexpr row<A...>::rref<n> row<A...>::operator[](best::index_t<n> idx) && {
  return *BEST_MOVE(this->get_impl(idx));
}

template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::operator[](best::vlist<range>) const& {
  return row_internal::slice<range, A...>(*this);
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::operator[](best::vlist<range>) & {
  return row_internal::slice<range, A...>(*this);
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::operator[](best::vlist<range>) const&& {
  return row_internal::slice<range, A...>(BEST_MOVE(*this));
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::operator[](best::vlist<range>) && {
  return row_internal::slice<range, A...>(BEST_MOVE(*this));
}

template <typename... A>
template <size_t n>
constexpr row<A...>::cref<n> row<A...>::at(best::index_t<n> idx) const& {
  return *this->get_impl(idx);
}
template <typename... A>
template <size_t n>
constexpr row<A...>::ref<n> row<A...>::at(best::index_t<n> idx) & {
  return *this->get_impl(idx);
}
template <typename... A>
template <size_t n>
constexpr row<A...>::crref<n> row<A...>::at(best::index_t<n> idx) const&& {
  return *BEST_MOVE(this->get_impl(idx));
}
template <typename... A>
template <size_t n>
constexpr row<A...>::rref<n> row<A...>::at(best::index_t<n> idx) && {
  return *BEST_MOVE(this->get_impl(idx));
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::at(best::vlist<range>) const& {
  return row_internal::slice<range, A...>(*this);
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::at(best::vlist<range>) & {
  return row_internal::slice<range, A...>(*this);
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::at(best::vlist<range>) const&& {
  return row_internal::slice<range, A...>(BEST_MOVE(*this));
}
template <typename... A>
template <best::bounds range>
constexpr auto row<A...>::at(best::vlist<range>) && {
  return row_internal::slice<range, A...>(BEST_MOVE(*this));
}
template <typename... A>
template <size_t n>
constexpr const best::object<typename row<A...>::template type<n>>&
row<A...>::object(best::index_t<n> i) const& {
  return this->get_impl(i);
}
template <typename... A>
template <size_t n>
constexpr best::object<typename row<A...>::template type<n>>& row<A...>::object(
    best::index_t<n> i) & {
  return this->get_impl(i);
}
template <typename... A>
template <size_t n>
constexpr const best::object<typename row<A...>::template type<n>>&&
row<A...>::object(best::index_t<n> i) const&& {
  return BEST_MOVE(this->get_impl(i));
}
template <typename... A>
template <size_t n>
constexpr best::object<typename row<A...>::template type<n>>&&
row<A...>::object(best::index_t<n> i) && {
  return BEST_MOVE(this->get_impl(i));
}

template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) const& {
  if constexpr (best::is_void<type<n>>) {
    return best::empty{};
  } else {
    return at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) & {
  if constexpr (best::is_void<type<n>>) {
    return best::empty{};
  } else {
    return at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) const&& {
  if constexpr (best::is_void<type<n>>) {
    return best::empty{};
  } else {
    return BEST_MOVE(*this).at(idx);
  }
}
template <typename... A>
template <size_t n>
constexpr decltype(auto) row<A...>::get(best::index_t<n> idx) && {
  if constexpr (best::is_void<type<n>>) {
    return best::empty{};
  } else {
    return BEST_MOVE(*this).at(idx);
  }
}

template <typename... A>
constexpr auto row<A...>::join(best::is_row auto&&... those) const& {
  return row_internal::join(*this, BEST_FWD(those)...);
}
template <typename... A>
constexpr auto row<A...>::join(best::is_row auto&&... those) & {
  return row_internal::join(*this, BEST_FWD(those)...);
}
template <typename... A>
constexpr auto row<A...>::join(best::is_row auto&&... those) const&& {
  return row_internal::join(BEST_MOVE(*this), BEST_FWD(those)...);
}
template <typename... A>
constexpr auto row<A...>::join(best::is_row auto&&... those) && {
  return row_internal::join(BEST_MOVE(*this), BEST_FWD(those)...);
}
template <typename... A>
constexpr auto row<A...>::operator+(best::is_row auto&& that) const& {
  return join(BEST_FWD(that));
}
template <typename... A>
constexpr auto row<A...>::operator+(best::is_row auto&& that) & {
  return join(BEST_FWD(that));
}
template <typename... A>
constexpr auto row<A...>::operator+(best::is_row auto&& that) const&& {
  return BEST_MOVE(*this).join(BEST_FWD(that));
}
template <typename... A>
constexpr auto row<A...>::operator+(best::is_row auto&& that) && {
  return BEST_MOVE(*this).join(BEST_FWD(that));
}

template <typename... A>
template <typename T>
constexpr auto row<A...>::select(best::tlist<T> idx) const& {
  return gather(select_indices(idx));
}
template <typename... A>
template <typename T>
constexpr auto row<A...>::select(best::tlist<T> idx) & {
  return gather(select_indices(idx));
}
template <typename... A>
template <typename T>
constexpr auto row<A...>::select(best::tlist<T> idx) const&& {
  return BEST_MOVE(*this).gather(select_indices(idx));
}
template <typename... A>
template <typename T>
constexpr auto row<A...>::select(best::tlist<T> idx) && {
  return BEST_MOVE(*this).gather(select_indices(idx));
}

template <typename... A>
template <typename T>
constexpr auto row<A...>::select_indices(best::tlist<T> idx) {
  return row_internal::do_lookup<T, A...>();
}

// XXX: This code tickles a clang-format bug.
template <typename... A>
constexpr decltype(auto) row<A...>::first() const&
  requires(!types.is_empty())
{
  return at(index<0>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::first() &
    requires(!types.is_empty()) {
      return at(index<0>);
    } template <typename... A>
    constexpr decltype(auto) row<A...>::first() const&&
      requires(!types.is_empty())
{
  return BEST_MOVE(*this).at(index<0>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::first() &&
    requires(!types.is_empty()) {
      return BEST_MOVE(*this).at(index<0>);
    } template <typename... A>
    constexpr decltype(auto) row<A...>::second() const&
      requires(types.size() >= 2)
{
  return at(index<1>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::second() &
    requires(types.size() >= 2) {
      return at(index<1>);
    } template <typename... A>
    constexpr decltype(auto) row<A...>::second() const&&
      requires(types.size() >= 2)
{
  return BEST_MOVE(*this).at(index<1>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::second() &&
    requires(types.size() >= 2) {
      return BEST_MOVE(*this).at(index<1>);
    } template <typename... A>
    constexpr decltype(auto) row<A...>::last() const&
      requires(!types.is_empty())
{
  return at(index<types.size() - 1>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::last() &
    requires(!types.is_empty()) {
      return at(index<types.size() - 1>);
    } template <typename... A>
    constexpr decltype(auto) row<A...>::last() const&&
      requires(!types.is_empty())
{
  return BEST_MOVE(*this).at(index<types.size() - 1>);
}
template <typename... A>
    constexpr decltype(auto) row<A...>::last() &&
    requires(!types.is_empty()) {
      return BEST_MOVE(*this).at(index<types.size() - 1>);
    }
#define BEST_ROW_MUST_USE(func_)                              \
  [[nodiscard("best::row::" #func_                            \
              "() does not mutate its argument; instead, it " \
              "returns a new best::row")]]

    template <typename... A>
    BEST_ROW_MUST_USE(push)
    constexpr auto row<A...>::push(auto&& that) const& {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(auto&& that) & {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(auto&& that) const&& {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(auto&& that) && {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = size()}>);
}

template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::bind_t, auto&& that) const& {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::bind_t, auto&& that) & {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::bind_t, auto&& that) const&& {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = size()}>);
}
template <typename... A>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::bind_t, auto&& that) && {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = size()}>);
}

template <typename... A>
template <typename T>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::tlist<T>, auto&& those) const& {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
template <typename T>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::tlist<T>, auto&& those) & {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = size()}>);
}
template <typename... A>
template <typename T>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::tlist<T>, auto&& those) const&& {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = size()}>);
}
template <typename... A>
template <typename T>
BEST_ROW_MUST_USE(push)
constexpr auto row<A...>::push(best::tlist<T>, auto&& those) && {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = size()}>);
}

template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(auto&& that, best::index_t<n>) const& {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(auto&& that, best::index_t<n>) & {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(auto&& that, best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(auto&& that, best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 0}>);
}

template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::bind_t, auto&& that,
                                 best::index_t<n>) const& {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::bind_t, auto&& that,
                                 best::index_t<n>) & {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::bind_t, auto&& that,
                                 best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::bind_t, auto&& that,
                                 best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 0}>);
}

template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::tlist<T>, auto&& those,
                                 best::index_t<n>) const& {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::tlist<T>, auto&& those,
                                 best::index_t<n>) & {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::tlist<T>, auto&& those,
                                 best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = n, .count = 0}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(insert)
constexpr auto row<A...>::insert(best::tlist<T>, auto&& those,
                                 best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = n, .count = 0}>);
}

template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(auto&& that, best::index_t<n>) const& {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(auto&& that, best::index_t<n>) & {
  return splice(best::types<best::as_auto<decltype(that)>>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(auto&& that, best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(auto&& that, best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<best::as_auto<decltype(that)>>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 1}>);
}

template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::bind_t, auto&& that,
                                 best::index_t<n>) const& {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::bind_t, auto&& that,
                                 best::index_t<n>) & {
  return splice(best::types<decltype(that)&&>,
                best::row(best::bind, BEST_FWD(that)),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::bind_t, auto&& that,
                                 best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::bind_t, auto&& that,
                                 best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<decltype(that)&&>,
                                 best::row(best::bind, BEST_FWD(that)),
                                 best::vals<bounds{.start = n, .count = 1}>);
}

template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::tlist<T>, auto&& those,
                                 best::index_t<n>) const& {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::tlist<T>, auto&& those,
                                 best::index_t<n>) & {
  return splice(best::types<T>, best::row(best::args(BEST_FWD(those))),
                best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::tlist<T>, auto&& those,
                                 best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n, typename T>
BEST_ROW_MUST_USE(update)
constexpr auto row<A...>::update(best::tlist<T>, auto&& those,
                                 best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::types<T>,
                                 best::row(best::args(BEST_FWD(those))),
                                 best::vals<bounds{.start = n, .count = 1}>);
}

template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::is_row auto&& those,
                                 best::vlist<range>) const& {
  return row_internal::splice<range, A...>(*this, those.types, BEST_FWD(those));
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::is_row auto&& those,
                                 best::vlist<range>) & {
  return row_internal::splice<range, A...>(*this, those.types, BEST_FWD(those));
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::is_row auto&& those,
                                 best::vlist<range>) const&& {
  return row_internal::splice<range, A...>(BEST_MOVE(*this), those.types,
                                           BEST_FWD(those));
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::is_row auto&& those,
                                 best::vlist<range>) && {
  return row_internal::splice<range, A...>(BEST_MOVE(*this), those.types,
                                           BEST_FWD(those));
}

template <typename... A>
template <best::bounds range, typename... Those>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::tlist<Those...> those_types,
                                 best::is_row auto&& those,
                                 best::vlist<range>) const& {
  return row_internal::splice<range, A...>(*this, those_types, BEST_FWD(those));
}
template <typename... A>
template <best::bounds range, typename... Those>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::tlist<Those...> those_types,
                                 best::is_row auto&& those,
                                 best::vlist<range>) & {
  return row_internal::splice<range, A...>(*this, those_types, BEST_FWD(those));
}
template <typename... A>
template <best::bounds range, typename... Those>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::tlist<Those...> those_types,
                                 best::is_row auto&& those,
                                 best::vlist<range>) const&& {
  return row_internal::splice<range, A...>(BEST_MOVE(*this), those_types,
                                           BEST_FWD(those));
}
template <typename... A>
template <best::bounds range, typename... Those>
BEST_ROW_MUST_USE(splice)
constexpr auto row<A...>::splice(best::tlist<Those...> those_types,
                                 best::is_row auto&& those,
                                 best::vlist<range>) && {
  return row_internal::splice<range, A...>(BEST_MOVE(*this), those_types,
                                           BEST_FWD(those));
}

template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(remove)
constexpr auto row<A...>::remove(best::index_t<n>) const& {
  return splice(best::row(), best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(remove)
constexpr auto row<A...>::remove(best::index_t<n>) & {
  return splice(best::row(), best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(remove)
constexpr auto row<A...>::remove(best::index_t<n>) const&& {
  return BEST_MOVE(*this).splice(best::row(),
                                 best::vals<bounds{.start = n, .count = 1}>);
}
template <typename... A>
template <size_t n>
BEST_ROW_MUST_USE(remove)
constexpr auto row<A...>::remove(best::index_t<n>) && {
  return BEST_MOVE(*this).splice(best::row(),
                                 best::vals<bounds{.start = n, .count = 1}>);
}

template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(erase)
constexpr auto row<A...>::erase(best::vlist<range>) const& {
  return splice(best::row(), best::vals<range>);
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(erase)
constexpr auto row<A...>::erase(best::vlist<range>) & {
  return splice(best::row(), best::vals<range>);
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(erase)
constexpr auto row<A...>::erase(best::vlist<range>) const&& {
  return BEST_MOVE(*this).splice(best::row(), best::vals<range>);
}
template <typename... A>
template <best::bounds range>
BEST_ROW_MUST_USE(erase)
constexpr auto row<A...>::erase(best::vlist<range>) && {
  return BEST_MOVE(*this).splice(best::row(), best::vals<range>);
}

template <typename... A>
template <size_t... n>
constexpr auto row<A...>::gather(best::vlist<n...>) const& {
  return row_internal::gather<n...>(*this);
}
template <typename... A>
template <size_t... n>
constexpr auto row<A...>::gather(best::vlist<n...>) & {
  return row_internal::gather<n...>(*this);
}
template <typename... A>
template <size_t... n>
constexpr auto row<A...>::gather(best::vlist<n...>) const&& {
  return row_internal::gather<n...>(BEST_MOVE(*this));
}
template <typename... A>
template <size_t... n>
constexpr auto row<A...>::gather(best::vlist<n...>) && {
  return row_internal::gather<n...>(BEST_MOVE(*this));
}

template <typename... A>
template <size_t... n>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::is_row auto&& those,
                                  best::vlist<n...>) const& {
  return row_internal::scatter<n...>(*this, those.types, BEST_FWD(those));
}

template <typename... A>
template <size_t... n>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::is_row auto&& those,
                                  best::vlist<n...>) & {
  return row_internal::scatter<n...>(*this, those.types, BEST_FWD(those));
}

template <typename... A>
template <size_t... n>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::is_row auto&& those,
                                  best::vlist<n...>) const&& {
  return row_internal::scatter<n...>(BEST_MOVE(*this), those.types,
                                     BEST_FWD(those));
}

template <typename... A>
template <size_t... n>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::is_row auto&& those,
                                  best::vlist<n...>) && {
  return row_internal::scatter<n...>(BEST_MOVE(*this), those.types,
                                     BEST_FWD(those));
}

template <typename... A>
template <size_t... n, typename... Ts>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::tlist<Ts...> those_types,
                                  best::is_row auto&& those,
                                  best::vlist<n...>) const& {
  return row_internal::scatter<n...>(*this, those.types, BEST_FWD(those));
}

template <typename... A>
template <size_t... n, typename... Ts>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::tlist<Ts...> those_types,
                                  best::is_row auto&& those,
                                  best::vlist<n...>) & {
  return row_internal::scatter<n...>(*this, those.types, BEST_FWD(those));
}

template <typename... A>
template <size_t... n, typename... Ts>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::tlist<Ts...> those_types,
                                  best::is_row auto&& those,
                                  best::vlist<n...>) const&& {
  return row_internal::scatter<n...>(BEST_MOVE(*this), those.types,
                                     BEST_FWD(those));
}

template <typename... A>
template <size_t... n, typename... Ts>
BEST_ROW_MUST_USE(scatter)
constexpr auto row<A...>::scatter(best::tlist<Ts...> those_types,
                                  best::is_row auto&& those,
                                  best::vlist<n...>) && {
  return row_internal::scatter<n...>(BEST_MOVE(*this), those.types,
                                     BEST_FWD(those));
}

#undef BEST_TLIST_MUST_USE

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
    return best::call(BEST_FWD(f),
                      BEST_MOVE(*this).get(best::index<I::value>)...);
  });
}
template <typename... A>
constexpr decltype(auto) row<A...>::apply(auto&& f) && {
  return indices.apply([&]<typename... I>() -> decltype(auto) {
    return best::call(BEST_FWD(f),
                      BEST_MOVE(*this).get(best::index<I::value>)...);
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
