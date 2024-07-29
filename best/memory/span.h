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

#ifndef BEST_MEMORY_SPAN_H_
#define BEST_MEMORY_SPAN_H_

#include <cstddef>
#include <initializer_list>
#include <type_traits>

#include "best/base/ord.h"
#include "best/base/port.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/container/result.h"
#include "best/iter/iter.h"
#include "best/log/location.h"
#include "best/math/overflow.h"
#include "best/memory/internal/bytes.h"
#include "best/meta/init.h"
#include "best/meta/tlist.h"

//! Data spans.
//!
//! `best::span<T, n>` is a view into a contiguous array of `n` `T`s. The extent
//! `n` may be dynamic, represented as `best::span<T>`.
//!
//! This header also provides concepts and traits for working with contiguous
//! ranges, i.e., ranges that can be represented as spans.

namespace best {
/// # `best::data()`
///
/// Returns the data pointer of a contiguous range.
constexpr auto data(auto&& range) requires requires { range.data(); }
{
  return BEST_FWD(range).data();
}
template <typename T, size_t n>
constexpr T* data(T (&range)[n]) {
  return range;
}
template <typename T>
constexpr const T* data(const std::initializer_list<T>& il) {
  return il.begin();
}

/// # `best::size()`
///
/// Returns the size of a contiguous range.
constexpr size_t size(const auto& range) requires requires { range.size(); }
{
  return range.size();
}
template <size_t n>
constexpr size_t size(const auto (&)[n]) {
  return n;
}

/// # `best::contiguous`
///
/// Whether T is a contiguous range that can be converted into a span.
///
/// This is defined as a type that enables calls to best::data and best::size().
template <typename T>
concept contiguous = requires(const T& ct, T&& t) {
  best::size(ct);
  { *best::data(ct) } -> best::is_ref;
  { *best::data(t) } -> best::is_ref;
};

/// # `best::data_type<T>`
///
/// Extracts the referent type of a contiguous range. For example,
/// `best::data_type<int[4]>` is `int`.
template <best::contiguous R>
using data_type = best::un_ref<decltype(*best::data(best::lie<R>))>;

/// # `best::static_size<T>`
///
/// The static size of a best::contiguous type, if it has one.
///
/// This is a FTADLE (https://abseil.io/tips/218). To make your best::contiguous
/// type be best::static_contiguous, implement this function as a constexpr
/// friend named BestStaticSize of the following form:
///
/// friend constexpr best::option<size_t> BestStaticSize(auto, MyType*);
///
/// The returned size must equal the unique size returned by best::size when
/// applied to this type, or best::none.
template <best::contiguous T>
inline constexpr best::option<size_t> static_size =
  BestStaticSize(best::types<T>, best::as_raw_ptr<T>{});

/// # `best::static_contiguous`
///
/// Whether T is a contiguous container of statically-known size.
template <typename T>
concept static_contiguous = static_size<T>.has_value();

/// # `best::is_span`
///
/// Represents whether T is best::span<U, n> for some U, n.
template <typename T>
concept is_span =
  best::same<best::as_auto<T>, best::span<data_type<T>, static_size<T>>>;

/// # `best::from_nul`
///
/// Wraps a pointer to a NUL (i.e., `T{0}`) terminated string in a `best::span`.
/// This function is essentially a safer `strlen()`.
template <typename T>
constexpr best::span<T> from_nul(T* ptr) {
  return best::span<T>::from_nul(ptr);
}

/// # `best::from_static`
///
/// Constructs the best possible static span pointing to `range`. If `range`
/// does not have a static size, this returns a dynamic span.
///
/// Notably, this is not the behavior of `best::span`'s deduction guides:
/// `best::span(range)` will never deduce a static extent, because doing this by
/// default turns out to be very annoying.
template <best::contiguous R>
constexpr best::span<best::data_type<R>, best::static_size<R>> from_static(
  R&& range) {
  return {BEST_FWD(range)};
}

/// # `best::span<T, n>`
///
/// A pointer and a length.
///
/// A span specifies a possibly cv-qualified element type and an optional
/// size. If the static size (given by `span::extent`) is `best::none`, the
/// span has _dynamic size_.
///
/// Spans are great when a function needs to take contiguous data as an
/// argument, since they can be constructed from any best::contiguous type,
/// including initializer lists.
///
/// ## Indexing and Iterating
///
/// Spans offer an API somewhat like std::span, but differs significantly when
/// constructing subspans. It also provides many helpers analogous to those on
/// Rust's slice type.
///
/// Like any other C++ array type, individual elements can be accessed with []:
///
/// ```
/// best::span<int> sp = ...;
/// sp[5] *= 2;
/// ```
///
/// Unlike C++, obtaining a subspan uses a best::bounds. For example, we can
/// write sp[{.start = 4, .end = 6}]. See bounds.h for more details on the
/// syntax. Also, all accesses to a span are always bounds-checked at runtime.
///
/// Spans are iterable; the iterator yields references of type `T&`.
/// All spans are comparable, even the mutable ones. They are compared in
/// lexicographic order.
///
/// Unfortunately, it is not possible to make `best::span<T>` work when `T` is
/// not an object type.
template <best::is_object T,
          // NOTE: This default is in the fwd decl in option.h.
          best::option<best::dependent<size_t, T>> n /* = best::none */>
class span final {
 public:
  /// Helper type aliases.
  using type = T;
  using value_type = best::un_qual<T>;
  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_raw_ptr<const type>;
  using ptr = best::as_raw_ptr<type>;

  /// # `span::is_static`
  ///
  /// Whether this is a static span.
  static constexpr bool is_static = n.has_value();

  /// # `span::is_dynamic`
  ///
  /// Whether this is a dynamic span.
  static constexpr bool is_dynamic = n.is_empty();

  /// # `span::is_const`
  ///
  /// Whether it is possible to mutate through this span.
  static constexpr bool is_const = best::is_const<T>;

  /// # `span::with_extent`
  ///
  /// Rebinds the extent of this span type.
  template <best::option<size_t> m>
  using with_extent = span<T, m>;

  /// # `span::as_dynamic`
  ///
  /// Removes the static extent of this span type.
  using as_dynamic = with_extent<best::none>;

  /// # `span()`
  ///
  /// Constructs a new empty span.
  ///
  /// This span's data pointer is always null.
  constexpr span() requires is_dynamic || (n == 0)
  = default;

  /// # `span::span(span)`
  ///
  /// Trivial to copy/move.
  constexpr span(const span&) = default;
  constexpr span& operator=(const span&) = default;
  constexpr span(span&&) = default;
  constexpr span& operator=(span&&) = default;

  /// # `span::span(T*, size_t)`
  ///
  /// Constructs a new span from a pointer and length.
  ///
  /// If this span has a statically-known size, this constructor will crash if
  /// len is not that size.
  constexpr explicit(is_static)
    span(best::ptr<T> data, size_t size, best::location loc = best::here);

  /// # `span::span(range)`
  ///
  /// Constructs a new span from a contiguous range.
  ///
  /// If this would construct a fixed-size span, and the list being constructed
  /// from has a different length, this constructor will crash. However, if
  /// this constructor can crash, it is explicit.
  template <best::contiguous R>
  constexpr explicit(is_static && n != static_size<R>)
    span(R&& range, best::location loc = best::here)
      requires best::qualifies_to<best::un_ref<best::data_type<R>>, T> &&
               (is_dynamic ||                       //
                best::static_size<R>.is_empty() ||  //
                best::static_size<span> == best::static_size<R>)
    : span(best::data(range), best::size(range), loc) {}

  /// # `span::span{...}`
  ///
  /// Constructs a new span from an initializer list. This enable the
  /// syntax `best::span{1, 2, 3, 4}`. Note that such spans dangle at the end
  /// of the full expression that contains them.
  ///
  /// If this would construct a fixed-size span, and the list being constructed
  /// from has a different length, this constructor will crash.
  constexpr explicit(is_static)
    span(std::initializer_list<value_type> il, best::location loc = best::here)
      requires is_const
    : span(best::data(il), best::size(il), loc) {}

  /// # `span::from_nul()`
  ///
  /// Constructs a new span pointing to a NUL-terminated string (i.e., a string
  /// of span elements, the last of which is zero).
  ///
  /// If `data` is null, returns an empty span.
  ///
  /// If this is a fixed-with span, this will perform the usual fatal bounds
  /// check upon construction.
  static constexpr span from_nul(T* data);

 private:
  template <size_t m>
  static constexpr best::option<size_t> minus =
    n ? best::option(best::saturating_sub(*n, m)) : best::none;

 public:
  /// # `span::data()`
  ///
  /// Returns the data pointer for this span.
  constexpr best::ptr<T> data() const { return data_; }

  /// # `span::size()`
  ///
  /// Returns the size (length) of this span.
  static constexpr size_t size() requires is_static
  {
    return *n;
  }
  constexpr size_t size() const requires is_dynamic
  {
    return size_;
  }

  /// # `span::is_empty()`
  ///
  /// Returns whether this span is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// # `span::first()`
  ///
  /// Returns the first, or first `m`, elements of this span, or `best::none` if
  /// there are not enough elements.
  constexpr best::option<T&> first() const;
  template <size_t m>
  constexpr best::option<best::span<T, m>> first(best::index_t<m> = {}) const;

  /// # `span::last()`
  ///
  /// Returns the last, or last `m`, elements of this span, or `best::none` if
  /// there are not enough elements.
  constexpr best::option<T&> last() const;
  template <size_t m>
  constexpr best::option<best::span<T, m>> last(best::index_t<m> = {}) const;

  /// # `span::split_first()`
  ///
  /// Returns the first, or first `m`, elements of this span, and the remaining
  /// elements, or `best::none` if there are not enough elements.
  constexpr auto split_first() const
    -> best::option<best::row<T&, span<T, minus<1>>>>;
  template <size_t m>
  constexpr auto split_first(best::index_t<m> = {}) const
    -> best::option<best::row<span<T, m>, span<T, minus<m>>>>;

  /// # `span::split_last()`
  ///
  /// Returns the last, or last `m`, elements of this span, and the remaining
  /// elements, or `best::none` if there are not enough elements.
  constexpr auto split_last() const
    -> best::option<best::row<T&, span<T, minus<1>>>>;
  template <size_t m>
  constexpr auto split_last(best::index_t<m> = {}) const
    -> best::option<best::row<span<T, m>, span<T, minus<m>>>>;

  /// # `span::take_first()`
  ///
  /// Splits this span at `m`; returns the prefix, and updates this to be the
  /// rest. If `m > size()`, returns best::none and leaves this span untouched
  constexpr best::option<best::span<T>> take_first(size_t m) requires is_dynamic
  ;

  /// # `span::take_last()`
  ///
  /// Splits this span at `m`; returns the suffix, and updates this to be the
  /// rest. If `m > size()`, returns best::none.
  constexpr best::option<best::span<T>> take_last(size_t m) requires is_dynamic;

  /// # `span[idx]`
  ///
  /// Extracts a single element. Crashes if the requested index is
  /// out-of-bounds.
  constexpr T& operator[](best::track_location<size_t> idx) const;

  /// # `span[best::index<n>]`
  ///
  /// Extracts a single element at a statically-specified index. If the
  /// requested index is out-of-bounds, and `this->is_static`, produces a
  /// compile time error; otherwise crashes.
  template <size_t idx>
  constexpr T& operator[](best::index_t<idx>) const
    requires is_dynamic || ((idx < size()));

  /// # `span[{.start = ...}]`
  ///
  /// Extracts a subspan. Crashes if the requested range is out-of-bounds.
  constexpr best::span<T> operator[](best::bounds::with_location range) const;

  /// # `span[best::vals<bounds{.start = ...}>]`
  ///
  /// Extracts a fixed-size subspan. If the requested index is out-of-bounds,
  /// and `this->is_static`, produces a compile time error; otherwise crashes.
  template <best::bounds range>
  constexpr auto operator[](best::vlist<range>) const
    requires (range.try_compute_count(best::static_size<span>).has_value());

  /// # `span::at(idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// returns best::none.
  constexpr best::option<T&> at(size_t idx) const;

  /// # `span::at({.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds, returns
  /// best::none.
  constexpr best::option<best::span<T>> at(best::bounds range) const;

  /// # `span::at(unsafe, idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// Undefined Behavior.
  constexpr T& at(unsafe, size_t idx) const {
    return data().offset(idx).deref();
  }

  /// # `span::at(unsafe, {.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds,
  /// Undefined Behavior.
  constexpr best::span<T> at(unsafe, best::bounds range) const;

  /// # `span::iterator`
  ///
  /// This span's iterator type.
  class iter_impl;
  using iterator = best::iter<iter_impl>;

  /// # `span::iter()`, `span::begin()`, `span::end()`.
  ///
  /// Spans are iterable exactly how you'd expect.
  constexpr iterator iter() const {
    return iterator(iter_impl(data(), size()));
  }
  constexpr auto begin() const { return iter().into_range(); }
  constexpr auto end() const { return best::iter_range_end{}; }

  /// # `span.swap()`
  ///
  /// Swaps the elements at indices `a` and `b`.
  constexpr void swap(size_t a, size_t b, best::location loc = best::here) const
    requires (!is_const);

  /// # `span::reverse()`
  ///
  /// Reverses the order of the elements in this span, in-place.
  constexpr void reverse() const requires (!is_const);

  /// # `span::split_at()`
  ///
  /// Splits this span at `idx` and returns both halves.
  constexpr best::option<std::array<best::span<T>, 2>> split_at(size_t) const;

  /// # `span::find()`.
  ///
  /// Finds the first occurrence of a pattern by linear search, and returns its
  /// position.
  ///
  /// A pattern may be:
  ///
  /// - Some value that is `best::equatable<T>`.
  /// - Some range whose elements are `best::equatable<T>`.
  /// - A predicate on `const T&`.
  ///
  /// Where possible, this function will automatically call vectorized
  /// implementations of e.g. `memchr` and `memcmp` for finding the desired
  /// pattern. Therefore, when possible, prefer to provide a needle by value.
  template <best::equatable<T> U = T>
  constexpr best::option<size_t> find(const U& needle) const;
  template <best::contiguous R = best::span<const T>>
  constexpr best::option<size_t> find(const R& needle) const
    requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>);
  constexpr best::option<size_t> find(
    best::callable<void(const T&)> auto&& pred) const;

  /// # `span::contains()`
  ///
  /// Determines whether an element exists that matches some pattern.
  ///
  /// A pattern for a separator may be as in `span::find()`.
  template <best::equatable<T> U = T>
  constexpr bool contains(const U& needle) const;
  template <best::contiguous R = best::span<const T>>
  constexpr bool contains(const R& needle) const
    requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>);
  constexpr bool contains(best::callable<void(const T&)> auto&& pred) const;

  /// # `span::split_once()`.
  ///
  /// Calls `span::find()` to find the first occurrence of some pattern, and
  /// if found, returns the subspans before and after the separator.
  ///
  /// A pattern for a separator may be as in `span::find()`.
  template <best::equatable<T> U = T>
  constexpr best::option<best::row<span<T>, span<T>>> split_once(
    const U& needle) const;
  template <best::contiguous R = best::span<const T>>
  constexpr best::option<best::row<span<T>, span<T>>> split_once(
    const R& needle) const
    requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>);
  constexpr best::option<best::row<span<T>, span<T>>> split_once(
    best::callable<void(const T&)> auto&& pred) const;

  /// # `span::split()`.
  ///
  /// Returns an iterator over subspans separated by some pattern. Internally,
  /// it calls `span::split_once()` until it is out of span.
  ///
  /// A pattern for a separator may be as in `span::find()`.
  template <best::equatable<T> U = T>
  constexpr auto split(const U& needle) const;
  template <best::contiguous R = best::span<const T>>
  constexpr auto split(const R& needle) const
    requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>);
  constexpr auto split(best::callable<void(const T&)> auto&& pred) const;

  // This overload saves people from passing a temporary into split and doing
  // something like `auto splits = span.split(4);`
  template <best::equatable<T> U = T>
  constexpr auto split(U&& needle) const;

 private:
  template <typename P>
  class split_impl;
  template <typename P>
  using split_iter = best::iter<split_impl<P>>;

 public:
  /// # `span::starts_with()`, `span::ends_with()`
  ///
  /// Checks if this span starts or ends with a particular pattern.
  template <best::equatable<T> U = const T>
  constexpr bool starts_with(best::span<U> needle) const;
  template <best::equatable<T> U = const T>
  constexpr bool ends_with(best::span<U> needle) const;

  /// # `span::strip_prefix()`, `span::strip_suffix()`
  ///
  /// If this span starts with `prefix` (or ends with `suffix`), removes it and
  /// returns the rest; otherwise returns `best::none`.
  template <best::equatable<T> U = const T>
  constexpr best::option<best::span<T>> strip_prefix(
    best::span<U> prefix) const;
  template <best::equatable<T> U = const T>
  constexpr best::option<best::span<T>> strip_suffix(
    best::span<U> suffix) const;

  /// # `span::consume_prefix()`, `span::consume_suffix()`
  ///
  /// Like `strip_prefix()`/`strip_suffix()`, but returns a bool and updates the
  /// span in-place.
  template <best::equatable<T> U = const T>
  constexpr bool consume_prefix(best::span<U> prefix) requires is_dynamic;
  template <best::equatable<T> U = const T>
  constexpr bool consume_suffix(best::span<U> suffix) requires is_dynamic;

  /// # `span::sort()`
  ///
  /// Sorts the underlying span.
  ///
  /// There are three overloads. One takes no arguments, and uses the intrinsic
  /// ordering of the type. One takes an unary function, which must produce a
  /// reference to an ordered type to use as a key. Finally, one takes a binary
  /// function, which must return a partial ordering for any pair of elements of
  /// this type.
  ///
  /// Because this is implemented using the <algorithm> header, which would
  /// pull in a completely unacceptable amount of stuff, the implementations of
  /// these functions live in `//best/memory/span_sort.h`, which must be
  /// included separately.
  constexpr void sort() const requires best::comparable<T> && (!is_const);
  constexpr void sort(best::callable<void(const T&)> auto&&) const
    requires (!is_const);
  constexpr void sort(
    best::callable<best::partial_ord(const T&, const T&)> auto&&) const
    requires (!is_const);

  /// # `span::stable_sort()`
  ///
  /// Identical to `sort()`, but uses a stable sort which guarantees that equal
  /// items are not reordered past each other. This usually means the algorithm
  /// is slower.
  constexpr void stable_sort() const requires best::comparable<T> && (!is_const)
  ;
  constexpr void stable_sort(best::callable<void(const T&)> auto&&) const
    requires (!is_const);
  constexpr void stable_sort(
    best::callable<best::partial_ord(const T&, const T&)> auto&&) const
    requires (!is_const);

  /// # `span::bisect()`
  ///
  /// Performs binary search on this span
  ///
  /// There are three overloads. One takes the value to search for, which must
  /// be comparable with the elements of this span. One takes the value to
  /// search for, and a callback for extracting a "key" from each element to
  /// compare with the sought value. Finally, one simply takes a callback that
  /// takes a single argument, and returns how that element compares with the
  /// sought value (which need not actually be provided).
  ///
  /// If the value is found, its index is returned in `best::ok()`. If not,
  /// a `best::err()` containing where it should be inserted is returned.
  constexpr best::result<size_t, size_t> bisect(
    const best::comparable<T> auto& sought) const;
  constexpr best::result<size_t, size_t> bisect(
    const auto& sought, best::callable<void(const T&)> auto&&) const;
  constexpr best::result<size_t, size_t> bisect(
    best::callable<best::partial_ord(const T&)> auto&&) const;

  /// # `span::copy_from()`, `span::emplace_from()`
  ///
  /// Copies values from src. This has the same semantics as Go's `copy()`
  /// builtin: if the lengths are not equal, only the overlapping part is
  /// copied.
  ///
  /// `emplace_from()` additionally assumes the destination is uninitialized, so
  /// it will call a constructor rather than assignment.
  template <typename U = const T>
  constexpr void copy_from(best::span<U> src) const requires (!is_const);
  template <typename U = const T>
  constexpr void emplace_from(best::span<U> src) const requires (!is_const);

  // TODO(mcyoung): Various other iterators, including:
  // struct chunk_iter, struct window_iter, struct ptr_iter.

  /// # `span::destroy()`
  ///
  /// Destroys the elements of this span, in-place. This does not affect
  /// whatever the underlying storage for the span is.
  constexpr void destroy() const requires (!is_const);

  /// # `vec::shift_within()`
  ///
  /// Performs an internal `memmove()`. This relocates `count` elements starting
  /// at `src` to `dst`.
  ///
  /// NOTE! This function assumes that the destination range is uninitialized,
  /// *and* that the source range is initialized, except where they overlap.
  constexpr void shift_within(unsafe u, size_t dst, size_t src,
                              size_t count) const
    requires (!is_const) && is_static
  {
    as_dynamic(*this).shift_within(u, dst, src, count);
  }
  constexpr void shift_within(unsafe, size_t dst, size_t src,
                              size_t count) const
    requires (!is_const) && is_dynamic;

  /// # `span::has_subarray`
  ///
  /// Returns whether this span contains `that` as a subarray by pointer
  /// identity. In other words, this function checks if the pointed-to range
  /// of `that` is within the pointed-to range of `this`.
  constexpr bool has_subarray(const best::contiguous auto& that) const {
    auto start0 = data();
    auto end0 = start0 + size();

    auto start1 = best::data(that);
    auto end1 = start1 + best::size(that);

    return best::compare(start0, start1) <= 0 && best::compare(end1, end0) <= 0;
  }

  // All spans are comparable.
  template <best::is_object U, best::option<size_t> m>
  constexpr bool operator==(best::span<U, m> that) const
    requires best::equatable<T, U>;
  template <contiguous R>
  constexpr bool operator==(const R& range) const
    requires best::equatable<T, best::data_type<R>> && (!is_span<R>)
  {
    return *this == best::span(range);
  }

  template <best::is_object U, best::option<size_t> m>
  constexpr auto operator<=>(best::span<U, m> that) const
    requires best::comparable<T, U>;
  template <contiguous R>
  constexpr auto operator<=>(const R& range) const
    requires best::comparable<T, best::data_type<R>> && (!is_span<R>)
  {
    return *this <=> best::span(range);
  }

  constexpr friend best::option<size_t> BestStaticSize(auto, span*) {
    return n;
  }

 private:
  best::ptr<T> data_ = nullptr;
  [[no_unique_address]] best::select<n.has_value(), best::empty, size_t>
    size_{};
};

/// # `best::span::iter_impl`
///
/// The iterator implementation for `best::span`.
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
class span<T, n>::iter_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr best::span<T> rest() const {
    return best::span(start_, end_ - start_);
  }

  using BestIterArrow = void;

 private:
  friend span;
  friend best::iter<iter_impl>;
  friend best::iter<iter_impl&>;

  constexpr iter_impl(best::ptr<T> ptr, size_t idx)
    : start_(ptr), end_(ptr + idx) {}

  constexpr best::option<T&> next() {
    if (start_ == end_) { return best::none; }
    return best::option<T&>(*start_++);
  }

  constexpr best::size_hint size_hint() const {
    return {end_ - start_, end_ - start_};
  }

  constexpr size_t count() && { return end_ - start_; }

  constexpr best::option<T&> last() && {
    if (start_ == end_) { return best::none; }
    return end_.offset(-1).deref();
  }

  best::ptr<T> start_, end_;
};

/// # `best::span::split_impl`
///
/// The iterator implementation for `best::span`.
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <typename P>
class span<T, n>::split_impl final {
 public:
  /// # `iter->rest()`
  ///
  /// Returns the content not yet yielded.
  constexpr best::span<T> rest() const { return span_; }

  using BestIterArrow = void;

 private:
  friend span;
  friend best::iter<split_impl>;
  friend best::iter<split_impl&>;

  constexpr explicit split_impl(auto&& pat, best::span<T> span)
    : pat_(best::in_place, BEST_FWD(pat)), span_(span) {}

  constexpr best::option<best::span<T>> next() {
    if (done_) { return best::none; }
    if (auto found = span_.split_once(*pat_)) {
      span_ = found->second();
      return found->first();
    }
    done_ = true;
    auto rest = span_;
    span_ = {};
    return rest;
  }

  constexpr best::size_hint size_hint() const {
    if (done_) { return {0, 0}; }
    return {1, span_.size() + 1};
  }

  [[no_unique_address]] best::object<P> pat_;
  best::span<T> span_;
  bool done_ = false;
};

template <typename T>
span(T*, size_t) -> span<T>;
template <typename T>
span(best::ptr<T>, size_t) -> span<T>;
template <typename T>
span(std::initializer_list<T>) -> span<const T>;
template <contiguous R>
span(R&& r) -> span<best::data_type<R>>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {

inline constexpr best::option<size_t> BestStaticSize(auto,
                                                     best::contiguous auto*) {
  return best::none;
}

template <size_t n>
inline constexpr size_t BestStaticSize(auto, auto (*)[n]) {
  return n;
}

template <typename T, size_t n>
inline constexpr size_t BestStaticSize(auto, std::array<T, n>*) {
  return n;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr span<T, n>::span(best::ptr<T> data, size_t size, best::location loc)
  : data_(data) {
  if constexpr (is_static) {
    best::bounds bounds_check = {.start = this->size(), .count = 0};
    bounds_check.compute_count(this->size(), loc);
  } else {
    size_ = size;
  }
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<T&> span<T, n>::first() const {
  return at(0);
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <size_t m>
constexpr best::option<best::span<T, m>> span<T, n>::first(
  best::index_t<m>) const {
  return at({.end = m}).map(best::ctor<best::span<T, m>>);
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<T&> span<T, n>::last() const {
  return at(size() - 1);
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <size_t m>
constexpr best::option<best::span<T, m>> span<T, n>::last(
  best::index_t<m>) const {
  return at({.start = size() - m}).map(best::ctor<best::span<T, m>>);
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr auto span<T, n>::split_first() const
  -> best::option<best::row<T&, span<T, minus<1>>>> {
  if (is_empty()) { return best::none; }
  return {{*data(), span<T, minus<1>>(data() + 1, size() - 1)}};
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <size_t m>
constexpr auto span<T, n>::split_first(best::index_t<m>) const
  -> best::option<best::row<span<T, m>, span<T, minus<m>>>> {
  return at({.end = m}).map(best::ctor<span<T, m>>).map([&](auto ch) {
    return best::row{ch, span<T, minus<m>>(operator[]({.start = m}))};
  });
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr auto span<T, n>::split_last() const
  -> best::option<best::row<T&, span<T, minus<1>>>> {
  if (is_empty()) { return best::none; }
  return {{
    at(unsafe(
         "size() > 0 because of the is_empty() above, so size() - 1 < size()"),
       size() - 1),
    span<T, minus<1>>(data(), size() - 1),
  }};
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <size_t m>
constexpr auto span<T, n>::split_last(best::index_t<m>) const
  -> best::option<best::row<span<T, m>, span<T, minus<m>>>> {
  return at({.start = size() - m})
    .map(best::ctor<span<T, m>>)
    .map([&](auto ch) {
      return best::row{ch, span<T, minus<m>>(operator[]({.end = size() - m}))};
    });
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr span<T, n> span<T, n>::from_nul(T* data) {
  if (data == nullptr) { return {data, 0}; }

  if constexpr (best::bytes_internal::constexpr_byte_comparable<T>) {
    if (BEST_CONSTEXPR_MEMCMP_ || !std::is_constant_evaluated()) {
      size_t len =
#if BEST_CONSTEXPR_MEMCMP_
        __builtin_strlen(data);
#else
        bytes_internal::strlen(data);
#endif
      return {data, len};
    }
  }

  auto ptr = data;
  while (*ptr++ != T{0})
    ;
  return best::span(data, ptr - data - 1);
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<best::span<T>> span<T, n>::take_first(size_t m)
  requires is_dynamic
{
  if (m > size()) { return best::none; }
  auto [prefix, rest] = *split_at(m);
  *this = rest;
  return prefix;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<best::span<T>> span<T, n>::take_last(size_t m)
  requires is_dynamic
{
  if (m > size()) { return best::none; }
  auto [rest, suffix] = *split_at(size() - m);
  *this = rest;
  return suffix;

  return best::none;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr T& span<T, n>::operator[](best::track_location<size_t> idx) const {
  best::bounds{.start = idx, .count = 1}.compute_count(size(), idx);
  return at(
    unsafe("compute_count() performs a bounds check and crashes if it fails"),
    idx);
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <size_t idx>
constexpr T& span<T, n>::operator[](best::index_t<idx>) const
  requires is_dynamic || ((idx < size()))
{
  return (*this)[idx];
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::span<T> span<T, n>::operator[](
  best::bounds::with_location range) const {
  auto count = range.compute_count(size());
  return as_dynamic{data() + range.start, count};
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::bounds range>
constexpr auto span<T, n>::operator[](best::vlist<range>) const
  requires (range.try_compute_count(best::static_size<span>).has_value())
{
  constexpr auto count = range.try_compute_count(best::static_size<span>);
  return with_extent<count>(data() + range.start, *count);
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<T&> span<T, n>::at(size_t idx) const {
  if (idx < size()) { return at(unsafe("bounds check on the same line"), idx); }
  return best::none;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<best::span<T>> span<T, n>::at(best::bounds range) const {
  if (auto count = range.try_compute_count(size())) {
    return {{data() + range.start, *count}};
  }
  return best::none;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::span<T> span<T, n>::at(unsafe, best::bounds range) const {
  size_t count = size() - range.start;
  if (range.end) {
    count = *range.end - range.start;
  } else if (range.including_end) {
    count = *range.end - range.start + 1;
  } else if (range.count) {
    count = *range.count;
  }

  return {data() + range.start, count};
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::swap(size_t a, size_t b, best::location loc) const
  requires (!is_const)
{
  using ::std::swap;
  swap(operator[]({a, loc}), operator[]({b, loc}));
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::reverse() const requires (!is_const)
{
  for (size_t i = 0; i < size() / 2; ++i) { swap(i, size() - i - 1); }
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<std::array<best::span<T>, 2>> span<T, n>::split_at(
  size_t idx) const {
  if (auto prefix = at({.end = idx})) {
    auto rest = *this;
    rest.data_ += idx;
    rest.size_ -= idx;
    return {{*prefix, rest}};
  }

  return best::none;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr best::option<size_t> span<T, n>::find(const U& needle) const {
  return find(best::span(best::addr(needle), 1));
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::contiguous R>
constexpr best::option<size_t> span<T, n>::find(const R& needle) const
  requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>)
{
  if (best::size(needle) == 0) { return 0; }

  if constexpr (best::is_span<R>) {
    using U = best::data_type<R>;
    if constexpr (best::bytes_internal::constexpr_byte_comparable<T, U>) {
      return best::bytes_internal::search(*this, needle);
    } else if (!std::is_constant_evaluated()) {
      if constexpr (best::bytes_internal::byte_comparable<T, U>) {
        return best::bytes_internal::search(*this, needle);
      }
    }

    auto haystack = *this;
    auto [first, rest] = *needle.split_first();
    while (haystack.size() >= needle.size()) {
      // Skip to the next possible start.
      size_t next = 0;
      for (const auto& x : haystack) {
        if (x == first) { break; }
        ++next;
      }
      if (next == haystack.size()) { return best::none; }

      // Skip forward.
      haystack = at(unsafe("we just did a bounds check; avoid going through "
                           "best::option here for constexpr speed"),
                    {// SKip the first element for a speedier comparison.
                     .start = next + 1});

      // Now check if we found the full needle.
      if (haystack.starts_with(rest)) { return size() - haystack.size() - 1; }
    }
    return best::none;
  } else {
    // Canonicalize by tail-call to the version that takes a span to minimize
    // code bloat.
    return find(best::span(needle));
  }
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<size_t> span<T, n>::find(
  best::callable<void(const T&)> auto&& pred) const {
  size_t idx = 0;
  for (const auto& x : *this) {
    if (pred(x)) { return idx; }
    ++idx;
  }
  return best::none;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::contains(const U& needle) const {
  return find(needle).has_value();
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::contiguous R>
constexpr bool span<T, n>::contains(const R& needle) const
  requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>)
{
  return find(needle).has_value();
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr bool span<T, n>::contains(
  best::callable<void(const T&)> auto&& pred) const {
  return find(BEST_FWD(pred)).has_value();
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr best::option<best::row<span<T>, span<T>>> span<T, n>::split_once(
  const U& needle) const {
  auto idx = find(needle);
  if (!idx) { return best::none; }
  return {{
    best::span(data(), *idx),
    best::span(data() + *idx + 1, size() - *idx - 1),
  }};
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::contiguous R>
constexpr best::option<best::row<span<T>, span<T>>> span<T, n>::split_once(
  const R& needle) const
  requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>)
{
  auto idx = find(needle);
  if (!idx) { return best::none; }
  return {{
    best::span(data(), *idx),
    best::span(data() + *idx + needle.size(), size() - *idx - needle.size()),
  }};
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::option<best::row<span<T>, span<T>>> span<T, n>::split_once(
  best::callable<void(const T&)> auto&& pred) const {
  auto idx = find(pred);
  if (!idx) { return best::none; }
  return {{
    best::span(data(), idx),
    best::span(data() + *idx + 1, size() - *idx - 1),
  }};
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr auto span<T, n>::split(const U& needle) const {
  return split_iter<const U&>(split_impl<const U&>(needle, *this));
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr auto span<T, n>::split(U&& needle) const {
  return split_iter<U>(split_impl<U>(BEST_MOVE(needle), *this));
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::contiguous R>
constexpr auto span<T, n>::split(const R& needle) const
  requires best::equatable<T, best::data_type<R>> && (!best::equatable<T, R>)
{
  return split_iter(split_impl<const R&>(needle, *this));
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr auto span<T, n>::split(
  best::callable<void(const T&)> auto&& pred) const {
  return split_iter<decltype(pred)>(
    split_impl<decltype(pred)>(BEST_FWD(pred), *this));
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::starts_with(best::span<U> needle) const {
  return at({.end = needle.size()}) == needle;
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::ends_with(best::span<U> needle) const {
  return at({.start = size() - needle.size()}) == needle;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr best::option<best::span<T>> span<T, n>::strip_prefix(
  best::span<U> prefix) const {
  if (!starts_with(prefix)) { return best::none; }
  return at({.start = prefix.size()});
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr best::option<best::span<T>> span<T, n>::strip_suffix(
  best::span<U> suffix) const {
  if (!ends_with(suffix)) { return best::none; }
  return at({.end = size() - suffix.size()});
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::consume_prefix(best::span<U> prefix)
  requires is_dynamic
{
  if (!starts_with(prefix)) { return false; }
  *this = *at({.start = prefix.size()});
  return true;
}

/// # `span::consume_suffix()`
///
/// Like `strip_suffix()`, but returns a bool and updates the span in-place.
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::consume_suffix(best::span<U> suffix)
  requires is_dynamic
{
  if (!ends_with(suffix)) { return false; }
  *this = *at({.end = size() - suffix.size()});
  return true;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::result<size_t, size_t> span<T, n>::bisect(
  const best::comparable<T> auto& sought) const {
  return bisect([&](auto& that) { return that <=> sought; });
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::result<size_t, size_t> span<T, n>::bisect(
  const auto& sought, best::callable<void(const T&)> auto&& key) const {
  return bisect(
    [&](auto& that) { return best::call(BEST_FWD(key), that) <=> sought; });
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr best::result<size_t, size_t> span<T, n>::bisect(
  best::callable<best::partial_ord(const T&)> auto&& comparator) const {
  // Taken from Rust's `<[T]>::binary_search_by()`.
  size_t size = this->size();
  size_t left = 0;
  size_t right = size;
  while (left < right) {
    size_t mid = left + size / 2;

    unsafe u(R"(
      Quoting the Rust implementation:
      > SAFETY: the while condition means `size` is strictly positive, so
      > `size/2 < size`. Thus `left + size/2 < left + size`, which
      > coupled with the `left + size <= self.len()` invariant means
      > we have `left + size/2 < self.len()`, and this is in-bounds.
      )");
    auto cmp = best::call(BEST_FWD(comparator), (const T&)(at(u, mid)));

    left = cmp < 0 ? mid + 1 : left;
    right = cmp > 0 ? mid : right;
    if (cmp == 0) {
      // We are done. This hints to the caller that a bounds check to access
      // the span with the returned index is ok.
      best::assume(mid < this->size());
      return best::ok(mid);
    }
    size = right - left;
  }
  return best::err(left);
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <typename U>
constexpr void span<T, n>::copy_from(best::span<U> src) const
  requires (!is_const)
{
  data().copy_assign(src.data(), best::min(size(), src.size()));
}
template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <typename U>
constexpr void span<T, n>::emplace_from(best::span<U> src) const
  requires (!is_const)
{
  data().copy(src.data(), best::min(size(), src.size()));
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::destroy() const requires (!is_const)
{
  if constexpr (!best::is_debug() && best::destructible<T, trivially>) {
    return;
  }
  for (size_t i = 0; i < size(); ++i) { (data() + i).destroy(); }
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::is_object U, best::option<size_t> m>
constexpr bool span<T, n>::operator==(span<U, m> that) const
  requires best::equatable<T, U>
{
  if (std::is_constant_evaluated()) {
    if constexpr (best::bytes_internal::constexpr_byte_comparable<T, U>) {
      return best::bytes_internal::equate(best::span<T>(*this),
                                          best::span<U>(that));
    }
  } else {
    if constexpr (best::bytes_internal::byte_comparable<T, U>) {
      return best::bytes_internal::equate(best::span<T>(*this),
                                          best::span<U>(that));
    }
  }

  if (size() != that.size()) { return false; }
  unsafe u("the loop below performs a bounds check");
  for (size_t i = 0; i < size(); ++i) {
    if (at(u, i) != that.at(u, i)) { return false; }
  }

  return true;
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
template <best::is_object U, best::option<size_t> m>
constexpr auto span<T, n>::operator<=>(span<U, m> that) const
  requires best::comparable<T, U>
{
  if (std::is_constant_evaluated()) {
    if constexpr (best::bytes_internal::constexpr_byte_comparable<T, U>) {
      return best::bytes_internal::compare(best::span(*this), best::span(that));
    }
  } else {
    if constexpr (best::bytes_internal::byte_comparable<T, U>) {
      return best::bytes_internal::compare(best::span(*this), best::span(that));
    }
  }

  size_t prefix = best::min(size(), that.size());
  unsafe u("the loop below performs a bounds check");
  for (size_t i = 0; i < prefix; ++i) {
    if (auto result = at(u, i) <=> that.at(u, i); result != 0) {
      return result;
    }
  }

  return best::order_type<T, U>(size() <=> that.size());
}

template <best::is_object T, best::option<best::dependent<size_t, T>> n>
constexpr void span<T, n>::shift_within(unsafe u, size_t dst, size_t src,
                                        size_t count) const
  requires (!is_const) && is_dynamic
{
  (data() + dst).relo_overlapping(data() + src, count);
}
}  // namespace best

#endif  // BEST_MEMORY_SPAN_H_
