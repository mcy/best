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

#ifndef BEST_CONTAINER_SPAN_H_
#define BEST_CONTAINER_SPAN_H_

#include <cstddef>
#include <initializer_list>
#include <type_traits>

#include "best/base/port.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/log/location.h"
#include "best/math/overflow.h"
#include "best/memory/bytes.h"
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
constexpr auto data(auto&& range)
  requires requires { range.data(); }
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
constexpr size_t size(const auto& range)
  requires requires { range.size(); }
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
using data_type = best::unref<decltype(*best::data(best::lie<R>))>;

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
    BestStaticSize(best::types<T>, best::as_ptr<T>{});

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
          best::option<size_t> n /* = best::none */>
class span final {
 public:
  /// Helper type aliases.
  using type = T;
  using value_type = best::unqual<T>;
  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

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
  constexpr span()
    requires is_dynamic || (n == 0)
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
  constexpr explicit(is_static) span(best::object_ptr<T> data, size_t size,
                                     best::location loc = best::here);

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
    requires best::qualifies_to<best::unref<best::data_type<R>>, T> &&
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
  constexpr explicit(is_static) span(std::initializer_list<value_type> il,
                                     best::location loc = best::here)
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
  constexpr static span from_nul(T* data);

 private:
  template <size_t m>
  static constexpr best::option<size_t> minus =
      n ? best::option(best::saturating_sub(*n, m)) : best::none;

 public:
  /// # `span::data()`
  ///
  /// Returns the data pointer for this span.
  constexpr best::object_ptr<T> data() const { return data_; }

  /// # `span::size()`
  ///
  /// Returns the size (length) of this span.
  constexpr static size_t size()
    requires is_static
  {
    return *n;
  }
  constexpr size_t size() const
    requires is_dynamic
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
  constexpr best::option<best::span<T>> take_first(size_t m)
    requires is_dynamic;

  /// # `span::take_last()`
  ///
  /// Splits this span at `m`; returns the suffix, and updates this to be the
  /// rest. If `m > size()`, returns best::none.
  constexpr best::option<best::span<T>> take_last(size_t m)
    requires is_dynamic;

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
    requires(range.try_compute_count(best::static_size<span>).has_value());

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
  constexpr T& at(unsafe, size_t idx) const { return data()[idx]; }

  /// # `span::at(unsafe, {.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds,
  /// Undefined Behavior.
  constexpr best::span<T> at(unsafe, best::bounds range) const;

  /// # `span.swap()`
  ///
  /// Swaps the elements at indices `a` and `b`.
  constexpr void swap(size_t a, size_t b, best::location loc = best::here) const
    requires(!is_const);

  /// # `span::reverse()`
  ///
  /// Reverses the order of the elements in this span, in-place.
  constexpr void reverse() const
    requires(!is_const);

  /// # `span::split_at()`
  ///
  /// Splits this span at `idx` and returns both halves.
  constexpr best::option<std::array<best::span<T>, 2>> split_at(size_t) const;

  /// # `span::contains()`
  ///
  /// Performs a linear search for a matching element or subspan and returns
  /// whether it was found.
  template <best::equatable<T> U = const T>
  constexpr bool contains(best::span<U> needle) const;
  constexpr bool contains(const best::equatable<T> auto& needle) const;

  /// # `span::find()`
  ///
  /// Performs a linear search for a matching element or subspan and returns its
  /// index.
  template <best::equatable<T> U>
  constexpr best::option<size_t> find(best::span<U> needle) const;
  constexpr best::option<size_t> find(
      const best::equatable<T> auto& needle) const;

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
  constexpr bool consume_prefix(best::span<U> prefix)
    requires is_dynamic;
  template <best::equatable<T> U = const T>
  constexpr bool consume_suffix(best::span<U> suffix)
    requires is_dynamic;

  /// # `span::sort()`
  ///
  /// Sorts the underlying span. There are three overloads. One takes no
  /// arguments, and uses the intrinsic ordering of the type. One takes an
  /// unary function, which must produce a reference to an ordered type to use
  /// as a key. Finally, one takes a binary function, which must return a
  /// partial ordering for any pair of elements of this type.
  ///
  /// Because this is implemented using the <algorithm> header, which would
  /// pull in a completely unacceptable amount of this, the implementations of
  /// these functions live in `//best/container/span_sort.h`, which must be
  /// included separately.
  void sort() const
    requires best::comparable<T> && (!is_const);
  void sort(best::callable<void(const T&)> auto&&) const
    requires(!is_const);
  void sort(best::callable<best::partial_ord(const T&, const T&)> auto&&) const
    requires(!is_const);

  /// # `span::stable_sort()`
  ///
  /// Identical to `sort()`, but uses a stable sort which guarantees that equal
  /// items are not reordered past each other. This usually means the algorithm
  /// is slower.
  void stable_sort() const
    requires best::comparable<T> && (!is_const);
  void stable_sort(best::callable<void(const T&)> auto&&) const
    requires(!is_const);
  void stable_sort(
      best::callable<best::partial_ord(const T&, const T&)> auto&&) const
    requires(!is_const);

  /// # `span::copy_from()`, `span::emplace_from()`
  ///
  /// Copies values from src. This has the same semantics as Go's `copy()`
  /// builtin: if the lengths are not equal, only the overlapping part is
  /// copied.
  ///
  /// `emplace_from()` additionally assumes the destination is uninitialized, so
  /// it will call a constructor rather than assignment.
  template <typename U = const T>
  constexpr void copy_from(best::span<U> src) const
    requires(!is_const);
  template <typename U = const T>
  constexpr void emplace_from(best::span<U> src) const
    requires(!is_const);

  /// # `span::iter`, `span::begin()`, `span::end()`.
  ///
  /// Spans are iterable exactly how you'd expect.
  struct iter;
  constexpr iter begin() const { return {data(), size()}; }
  constexpr iter end() const { return {data() + size(), 0}; }

  // TODO(mcyoung): Various other iterators, including:
  // struct chunk_iter, struct window_iter, struct split_iter, struct ptr_iter.

  /// # `span::destroy_in_place()`
  ///
  /// Destroys the elements of this span, in-place. This does not affect
  /// whatever the underlying storage for the span is.
  constexpr void destroy_in_place() const
    requires(!is_const);

  /// # `vec::shift_within()`
  ///
  /// Performs an internal `memmove()`. This relocates `count` elements starting
  /// at `src` to `dst`.
  ///
  /// NOTE! This function assumes that the destination range is uninitialized,
  /// *and* that the source range is initialized.
  constexpr void shift_within(unsafe u, size_t dst, size_t src,
                              size_t count) const
    requires(!is_const) && is_static
  {
    as_dynamic(*this).shift_within(u, dst, src, count);
  }
  constexpr void shift_within(unsafe, size_t dst, size_t src,
                              size_t count) const
    requires(!is_const) && is_dynamic;

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
  best::object_ptr<T> data_ = nullptr;
  [[no_unique_address]] best::select<n.has_value(), best::empty, size_t>
      size_{};
};

template <typename T>
span(T*, size_t) -> span<T>;
template <typename T>
span(best::object_ptr<T>, size_t) -> span<T>;
template <typename T>
span(std::initializer_list<T>) -> span<const T>;
template <contiguous R>
span(R&& r) -> span<best::data_type<R>>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <best::is_object T, best::option<size_t> n>
struct span<T, n>::iter final {
  constexpr iter() = default;

  constexpr bool operator==(const iter& that) const = default;

  constexpr decltype(auto) operator*() const { return (*this)[0]; }
  constexpr best::as_ptr<T> operator->() const { return ptr_.operator->(); }
  constexpr decltype(auto) operator[](size_t idx) const { return ptr_[idx]; }

  constexpr iter& operator++() {
    ++ptr_;
    return *this;
  }
  constexpr iter operator++(int) {
    auto prev = *this;
    ++*this;
    return prev;
  }

  constexpr iter& operator--() {
    --ptr_;
    return *this;
  }
  constexpr iter operator--(int) {
    auto prev = *this;
    --*this;
    return prev;
  }

 private:
  friend span;
  constexpr iter(best::object_ptr<T> ptr, size_t idx) : ptr_(ptr) {}
  best::object_ptr<T> ptr_;
};

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

template <best::is_object T, best::option<size_t> n>
constexpr span<T, n>::span(best::object_ptr<T> data, size_t size,
                           best::location loc)
    : data_(data) {
  if constexpr (is_static) {
    best::bounds bounds_check = {.start = this->size(), .count = 0};
    bounds_check.compute_count(this->size(), loc);
  } else {
    size_ = size;
  }
}

template <best::is_object T, best::option<size_t> n>
constexpr best::option<T&> span<T, n>::first() const {
  return at(0);
}
template <best::is_object T, best::option<size_t> n>
template <size_t m>
constexpr best::option<best::span<T, m>> span<T, n>::first(
    best::index_t<m>) const {
  return at({.end = m}).map(best::ctor<best::span<T, m>>);
}
template <best::is_object T, best::option<size_t> n>
constexpr best::option<T&> span<T, n>::last() const {
  return at(size() - 1);
}
template <best::is_object T, best::option<size_t> n>
template <size_t m>
constexpr best::option<best::span<T, m>> span<T, n>::last(
    best::index_t<m>) const {
  return at({.start = size() - m}).map(best::ctor<best::span<T, m>>);
}

template <best::is_object T, best::option<size_t> n>
constexpr auto span<T, n>::split_first() const
    -> best::option<best::row<T&, span<T, minus<1>>>> {
  return split_first(best::index<1>).map([](auto pair) {
    return best::row<T&, span<T, minus<1>>>{pair.first()[0], pair.second()};
  });
}
template <best::is_object T, best::option<size_t> n>
template <size_t m>
constexpr auto span<T, n>::split_first(best::index_t<m>) const
    -> best::option<best::row<span<T, m>, span<T, minus<m>>>> {
  return at({.end = m}).map(best::ctor<span<T, m>>).map([&](auto ch) {
    return best::row{ch, span<T, minus<m>>(operator[]({.start = m}))};
  });
}

template <best::is_object T, best::option<size_t> n>
constexpr auto span<T, n>::split_last() const
    -> best::option<best::row<T&, span<T, minus<1>>>> {
  return split_last(best::index<1>).map([](auto pair) {
    return best::row<T&, span<T, minus<1>>>{pair.first()[0], pair.second()};
  });
}
template <best::is_object T, best::option<size_t> n>
template <size_t m>
constexpr auto span<T, n>::split_last(best::index_t<m>) const
    -> best::option<best::row<span<T, m>, span<T, minus<m>>>> {
  return at({.start = size() - m})
      .map(best::ctor<span<T, m>>)
      .map([&](auto ch) {
        return best::row{ch,
                         span<T, minus<m>>(operator[]({.end = size() - m}))};
      });
}

template <best::is_object T, best::option<size_t> n>
constexpr span<T, n> span<T, n>::from_nul(T* data) {
  if (data == nullptr) return {data, 0};

  if constexpr (best::constexpr_byte_comparable<T>) {
    if (BEST_CONSTEXPR_MEMCMP || !std::is_constant_evaluated()) {
      size_t len =
#if BEST_CONSTEXPR_MEMCMP
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

template <best::is_object T, best::option<size_t> n>
constexpr best::option<best::span<T>> span<T, n>::take_first(size_t m)
  requires is_dynamic
{
  if (m > size()) return best::none;
  auto [prefix, rest] = *split_at(m);
  *this = rest;
  return prefix;
}

template <best::is_object T, best::option<size_t> n>
constexpr best::option<best::span<T>> span<T, n>::take_last(size_t m)
  requires is_dynamic
{
  if (m > size()) return best::none;
  auto [rest, suffix] = *split_at(size() - m);
  *this = rest;
  return suffix;

  return best::none;
}

template <best::is_object T, best::option<size_t> n>
constexpr T& span<T, n>::operator[](best::track_location<size_t> idx) const {
  best::bounds{.start = idx, .count = 1}.compute_count(size(), idx);
  return data()[idx];
}

template <best::is_object T, best::option<size_t> n>
template <size_t idx>
constexpr T& span<T, n>::operator[](best::index_t<idx>) const
  requires is_dynamic || ((idx < size()))
{
  return (*this)[idx];
}

template <best::is_object T, best::option<size_t> n>
constexpr best::span<T> span<T, n>::operator[](
    best::bounds::with_location range) const {
  auto count = range.compute_count(size());
  return as_dynamic{data() + range.start, count};
}

template <best::is_object T, best::option<size_t> n>
template <best::bounds range>
constexpr auto span<T, n>::operator[](best::vlist<range>) const
  requires(range.try_compute_count(best::static_size<span>).has_value())
{
  constexpr auto count = range.try_compute_count(best::static_size<span>);
  return with_extent<count>(data() + range.start, *count);
}

template <best::is_object T, best::option<size_t> n>
constexpr best::option<T&> span<T, n>::at(size_t idx) const {
  if (idx < size()) {
    return data()[idx];
  }
  return best::none;
}

template <best::is_object T, best::option<size_t> n>
constexpr best::option<best::span<T>> span<T, n>::at(best::bounds range) const {
  if (auto count = range.try_compute_count(size())) {
    return {{data() + range.start, *count}};
  }
  return best::none;
}

template <best::is_object T, best::option<size_t> n>
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

template <best::is_object T, best::option<size_t> n>
constexpr void span<T, n>::swap(size_t a, size_t b, best::location loc) const
  requires(!is_const)
{
  using ::std::swap;
  swap(operator[]({a, loc}), operator[]({b, loc}));
}

template <best::is_object T, best::option<size_t> n>
constexpr void span<T, n>::reverse() const
  requires(!is_const)
{
  for (size_t i = 0; i < size() / 2; ++i) {
    swap(i, size() - i - 1);
  }
}

template <best::is_object T, best::option<size_t> n>
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

template <best::is_object T, best::option<size_t> n>
constexpr bool span<T, n>::contains(
    const best::equatable<T> auto& needle) const {
  return find(needle).has_value();
}
template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::contains(best::span<U> needle) const {
  return find(needle).has_value();
}

template <best::is_object T, best::option<size_t> n>
constexpr best::option<size_t> span<T, n>::find(
    const best::equatable<T> auto& needle) const {
  size_t idx = 0;
  for (auto& candidate : *this) {
    if (candidate == needle) return idx;
    ++idx;
  }
  return best::none;
}
template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr best::option<size_t> span<T, n>::find(best::span<U> needle) const {
  if (needle.is_empty()) return 0;

  if constexpr (best::constexpr_byte_comparable<T, U>) {
    return best::search_bytes(*this, needle);
  } else if (!std::is_constant_evaluated()) {
    if constexpr (best::byte_comparable<T, U>) {
      return best::search_bytes(*this, needle);
    }
  }

  auto haystack = *this;
  auto& first = needle[0];
  while (haystack.size() >= needle.size()) {
    // Skip to the next possible start.
    auto next = haystack.find(first);
    if (!next) return best::none;

    // Skip forward.
    haystack = at(unsafe("we just did a bounds check; avoid going through "
                         "best::option here for constexpr speed"),
                  {.start = *next});

    // Now check if we found the full needle.
    if (haystack.starts_with(needle)) return size() - haystack.size();
  }
  return false;
}

template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::starts_with(best::span<U> needle) const {
  return at({.end = needle.size()}) == needle;
}
template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::ends_with(best::span<U> needle) const {
  return at({.start = size() - needle.size()}) == needle;
}

template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr best::option<best::span<T>> span<T, n>::strip_prefix(
    best::span<U> prefix) const {
  if (!starts_with(prefix)) return best::none;
  return at({.start = prefix.size()});
}

template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr best::option<best::span<T>> span<T, n>::strip_suffix(
    best::span<U> suffix) const {
  if (!ends_with(suffix)) return best::none;
  return at({.end = size() - suffix.size()});
}

template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::consume_prefix(best::span<U> prefix)
  requires is_dynamic
{
  if (!starts_with(prefix)) return false;
  *this = *at({.start = prefix.size()});
  return true;
}

/// # `span::consume_suffix()`
///
/// Like `strip_suffix()`, but returns a bool and updates the span in-place.
template <best::is_object T, best::option<size_t> n>
template <best::equatable<T> U>
constexpr bool span<T, n>::consume_suffix(best::span<U> suffix)
  requires is_dynamic
{
  if (!ends_with(suffix)) return false;
  *this = *at({.end = size() - suffix.size()});
  return true;
}

template <best::is_object T, best::option<size_t> n>
template <typename U>
constexpr void span<T, n>::copy_from(best::span<U> src) const
  requires(!is_const)
{
  if (!std::is_constant_evaluated() && best::same<T, U> &&
      best::copyable<T, trivially>) {
    best::copy_bytes(*this, src);
    return;
  }

  unsafe u("already performed a bounds check in the loop latch");
  size_t to_copy = best::min(size(), src.size());
  for (size_t i = 0; i < to_copy; ++i) {
    at(u, i) = src.at(u, i);
  }
}
template <best::is_object T, best::option<size_t> n>
template <typename U>
constexpr void span<T, n>::emplace_from(best::span<U> src) const
  requires(!is_const)
{
  if (!std::is_constant_evaluated() && best::same<T, U> &&
      best::copyable<T, trivially>) {
    best::copy_bytes(*this, src);
    return;
  }

  size_t to_copy = best::min(size(), src.size());
  for (size_t i = 0; i < to_copy; ++i) {
    (data() + i).copy_from(src.data() + i, false);
  }
}

template <best::is_object T, best::option<size_t> n>
constexpr void span<T, n>::destroy_in_place() const
  requires(!is_const)
{
  if constexpr (!best::is_debug() && best::destructible<T, trivially>) return;
  for (size_t i = 0; i < size(); ++i) {
    (data() + i).destroy_in_place();
  }
}

template <best::is_object T, best::option<size_t> n>
template <best::is_object U, best::option<size_t> m>
constexpr bool span<T, n>::operator==(span<U, m> that) const
  requires best::equatable<T, U>
{
  if (std::is_constant_evaluated()) {
    if constexpr (best::constexpr_byte_comparable<T, U>) {
      return best::equate_bytes(best::span<T>(*this), best::span<U>(that));
    }
  } else {
    if constexpr (best::byte_comparable<T, U>) {
      return best::equate_bytes(best::span<T>(*this), best::span<U>(that));
    }
  }

  if (size() != that.size()) return false;
  for (size_t i = 0; i < size(); ++i) {
    if (data()[i] != that.data()[i]) {
      return false;
    }
  }

  return true;
}

template <best::is_object T, best::option<size_t> n>
template <best::is_object U, best::option<size_t> m>
constexpr auto span<T, n>::operator<=>(span<U, m> that) const
  requires best::comparable<T, U>
{
  if (std::is_constant_evaluated()) {
    if constexpr (best::constexpr_byte_comparable<T, U>) {
      return best::compare_bytes(best::span(*this), best::span(that));
    }
  } else {
    if constexpr (best::byte_comparable<T, U>) {
      return best::compare_bytes(best::span(*this), best::span(that));
    }
  }

  size_t prefix = best::min(size(), that.size());
  for (size_t i = 0; i < prefix; ++i) {
    if (auto result = data()[i] <=> that.data()[i]; result != 0) {
      return result;
    }
  }

  return best::order_type<T, U>(size() <=> that.size());
}

template <best::is_object T, option<size_t> n>
constexpr void span<T, n>::shift_within(unsafe u, size_t dst, size_t src,
                                        size_t count) const
  requires(!is_const) && is_dynamic
{
  if (dst == src) return;

  // We need to handle the following cases.
  //
  // Non-overlapping shift. Happens when src + count <= dst or dst + count <=
  // src, need to destroy {.start = src, .count = count }.
  // | xxxx | yyyyyyyyyyyy | xxxxxxxxxxxx | ------------ | xxxx |
  //        src            src + count    dst            dst + count
  //
  // Overlapping forward shift. Happens when src < dst < src + count, need to
  // destroy {.start = src, .end = dest }
  // | xxxx | yyyyyyyyyyyy | yyyyyy | ------------ | xxxx |
  //        src            dst      src + count    dst + count
  //
  // The moved part is subdivided as follows according to how it needs to be
  // moved:
  // | aaaa | bbbbbbbbbbbb | cccc |
  // src    src + overlap  dst    src + count
  //
  // Where overlap = src + count - dst. The c part is move-constructed
  // but not destroyed; then the b part is relocated, and the a part is
  // move-assigned and then destroyed.
  //
  // Overlapping backward shift. Happens when dst < src < dest + count, need to
  // destroy {.start = dst + count, .end = src + count }
  // | xxxx | ------------ | yyyyyy | yyyyyyyyyyyy | xxxx |
  //        dst            src      dst + count    src + count
  //
  // The moved part is divided in the analogous way, but the move/relocate/
  // assign regions are in the opposite order.

  if (src + count <= dst || dst + count <= src) {
    // Non-overlapping case.
    if constexpr (best::relocatable<T, trivially>) {
      best::copy_bytes(at(u, {.start = dst, .count = count}),
                       at(u, {.start = src, .count = count}));
      at(u, {.start = src, .count = count}).destroy_in_place();
      return;
    }

    for (size_t i = 0; i < count; ++i) {
      (data() + dst + i).relocate_from(data() + src + i, false);
    }
  } else if (src < dst && dst < src + count) {
    // Forward case.
    if constexpr (best::relocatable<T, trivially>) {
      best::copy_overlapping_bytes(at(u, {.start = dst, .count = count}),
                                   at(u, {.start = src, .count = count}));
      at(u, {.start = src, .end = dst}).destroy_in_place();
      return;
    }

    // Need to make the copies in backward order to avoid trampling.
    size_t overlap = src + count - dst;
    for (size_t j = count; j > 0; --j) {
      size_t i = j - 1;
      if (i < overlap) {
        (data() + dst + i).relocate_from(data() + src + i, true);
      } else if (i <= count - overlap) {
        (data() + dst + i).relocate_from(data() + src + i, false);
      } else {
        (data() + dst + i).move_from(data() + src + i, false);
      }
    }
  } else if (dst < src && src < dst + count) {
    // Backward case.
    if constexpr (best::relocatable<T, trivially>) {
      best::copy_overlapping_bytes(at(u, {.start = dst, .count = count}),
                                   at(u, {.start = src, .count = count}));
      at(u, {.start = dst + count, .end = src + count}).destroy_in_place();
      return;
    }

    size_t overlap = dst + count - src;
    for (size_t i = 0; i < count; ++i) {
      if (i < overlap) {
        (data() + dst + i).move_from(data() + src + i, false);
      } else if (i <= count - overlap) {
        (data() + dst + i).relocate_from(data() + src + i, false);
      } else {
        (data() + dst + i).relocate_from(data() + src + i, true);
      }
    }
  }
}
}  // namespace best

#endif  // BEST_CONTAINER_SPAN_H_
