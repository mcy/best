#ifndef BEST_CONTAINER_SPAN_H_
#define BEST_CONTAINER_SPAN_H_

#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

#include "best/base/port.h"
#include "best/container/internal/span.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/log/location.h"
#include "best/math/overflow.h"
#include "best/memory/bytes.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"
#include "best/meta/tlist.h"

//! Data spans.
//!
//! `best::span<T, n>` is a view into a contiguous array of `n` `T`s. The extent
//! `n` may be dynamic, represented as `best::span<T>`.
//!
//! This header also provides concepts and traits for working with contiguous
//! ranges, i.e., ranges that can be represented as spans.

namespace best {
/// # `best::contiguous`
///
/// Whether T is a contiguous range that can be converted into a span.
///
/// This is defined as a type that enables calls to std::data and std::size().
template <typename T>
concept contiguous = requires(T&& t) {
  { *std::data(t) } -> best::ref_type;
  { std::size(t) } -> std::convertible_to<size_t>;
};

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
/// The returned size must equal the unique size returned by std::size when
/// applied to this type, or best::none.
template <contiguous T>
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
concept is_span = best::same<std::remove_cvref_t<T>,
                             best::span<typename std::remove_cvref_t<T>::type,
                                        std::remove_cvref_t<T>::extent>>;

/// # `best::span_type<T>`
///
/// Given T = best::span<U, n>, returns U.
template <is_span T>
using span_type = typename std::remove_cvref_t<T>::type;

/// # `best::span_extent<T>`
///
/// Given T = best::span<U, n>, returns n.
template <is_span T>
inline constexpr best::option<size_t> span_extent =
    std::remove_cvref_t<T>::extent;

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
template <best::object_type T,
          // NOTE: This default is in the fwd decl in option.h.
          best::option<size_t> n /* = best::none */>
class span final {
 public:
  /// Helper type aliases.
  using type = T;
  using value_type = std::remove_cv_t<T>;
  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  /// # `span::extent`
  ///
  /// The extent of this span, if it is statically known.
  static constexpr best::option<size_t> extent = n;

  /// # `span::is_static`
  ///
  /// Whether this is a static span.
  static constexpr bool is_static = extent.has_value();

  /// # `span::is_dynamic`
  ///
  /// Whether this is a dynamic span.
  static constexpr bool is_dynamic = extent.is_empty();

  /// # `span::is_const`
  ///
  /// Whether it is possible to mutate through this span.
  static constexpr bool is_const = std::is_const_v<T>;

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
    requires(is_dynamic || extent == 0)
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
  constexpr explicit(is_static) span(best::object_ptr<T> data, size_t size,  //
                                     best::location loc = best::here) {
    repr_.data = data;
    if constexpr (is_static) {
      best::bounds bounds_check = {.start = *extent, .count = 0};
      bounds_check.compute_count(this->size(), loc);
    } else {
      repr_.size = size;
    }
  }

  /// # `span::span(contiguous)`
  ///
  /// Constructs a new span from a contiguous range.
  ///
  /// If this would construct a fixed-size span, and the list being constructed
  /// from has a different length, this constructor will crash. However, if
  /// this constructor can crash, it is explicit.
  template <contiguous Range>
  constexpr explicit(
      is_static &&
      // XXX: For some god-forsaken reason Clang 17 seems to think that this is
      // not a constant expression inside of this explicit(): `extent ==
      // best::static_size<Range>`, even though it's perfectly ok with it in the
      // requires() that follows. This seems to be a bug (either in the
      // compiler or in the standard). Either way, this ate an hour of my time.
      //
      // We work around it by inlining the optional equality here.
      !(extent.has_value() == best::static_size<Range>.has_value() &&
        extent.value_or() == best::static_size<Range>.value_or()))
      span(Range&& range,  //
           best::location loc = best::here)
    requires best::qualifies_to<best::as_deref<decltype(*std::data(range))>,
                                T> &&
             (is_dynamic || best::static_size<Range>.is_empty() ||
              extent == best::static_size<Range>)
      : span(std::data(range), std::size(range), loc) {}

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
      : span(std::data(il), std::size(il), loc) {}

  /// # `span::from_nul()`
  ///
  /// Constructs a new span pointing to a NUL-terminated string (i.e., a string
  /// of span elements, the last of which is zero).
  ///
  /// If `data` is null, returns an empty span.
  ///
  /// If this is a fixed-with span, this will perform the usual fatal bounds
  /// check upon construction.
  constexpr static span from_nul(T* data) {
    if (data == nullptr) return {data, 0};

    auto ptr = data;
    while (*ptr++ != T{0})
      ;

    return best::span(data, ptr - data - 1);
  }

 private:
  template <size_t m>
  static constexpr best::option<size_t> minus =
      n ? best::option(best::saturating_sub(*n, m)) : best::none;

 public:
  /// # `span::data()`
  ///
  /// Returns the data pointer for this span.
  constexpr best::object_ptr<T> data() const { return repr_.data; }

  /// # `span::size()`
  ///
  /// Returns the size (length) of this span.
  constexpr size_t size() const { return repr_.size; }

  /// # `span::is_empty()`
  ///
  /// Returns whether this span is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// # `span::first()`
  ///
  /// Returns the first, or first `m`, elements of this span, or `best::none` if
  /// there are not enough elements.
  constexpr best::option<T&> first() const { return at(0); }
  template <size_t m>
  constexpr best::option<best::span<T, m>> first(best::index_t<m> = {}) const {
    return at({.end = m}).map(best::ctor<best::span<T, m>>);
  }

  /// # `span::last()`
  ///
  /// Returns the last, or last `m`, elements of this span, or `best::none` if
  /// there are not enough elements.
  constexpr best::option<T&> last() const { return at(size() - 1); }
  template <size_t m>
  constexpr best::option<best::span<T, m>> last(best::index_t<m> = {}) const {
    return at({.start = size() - m}).map(best::ctor<best::span<T, m>>);
  }

  /// # `span::split_first()`
  ///
  /// Returns the first, or first `m`, elements of this span, and the remaining
  /// elements, or `best::none` if there are not enough elements.
  constexpr best::option<std::pair<T&, span<T, minus<1>>>> split_first() const {
    return split_first(best::index<1>).map([](auto pair) {
      return std::pair<T&, span<T, minus<1>>>{pair.first[0], pair.second};
    });
  }
  template <size_t m>
  constexpr best::option<std::pair<span<T, m>, span<T, minus<m>>>> split_first(
      best::index_t<m> = {}) const {
    return at({.end = m}).map(best::ctor<span<T, m>>).map([&](auto ch) {
      return std::pair{ch, span<T, minus<m>>(operator[]({.start = m}))};
    });
  }

  /// # `span::split_last()`
  ///
  /// Returns the last, or last `m`, elements of this span, and the remaining
  /// elements, or `best::none` if there are not enough elements.
  constexpr best::option<std::pair<T&, span<T, minus<1>>>> split_last() const {
    return split_last(best::index<1>).map([](auto pair) {
      return std::pair<T&, span<T, minus<1>>>{pair.first[0], pair.second};
    });
  }
  template <size_t m>
  constexpr best::option<std::pair<span<T, m>, span<T, minus<m>>>> split_last(
      best::index_t<m> = {}) const {
    return at({.start = size() - m})
        .map(best::ctor<span<T, m>>)
        .map([&](auto ch) {
          return std::pair{
              ch,
              span<T, minus<m>>(operator[]({.end = size() - m})),
          };
        });
  }

  /// # `span::take_first()`
  ///
  /// Splits this span at `m`; returns the prefix, and updates this to be the
  /// rest. If `m > size()`, returns best::none and leaves this span untouched
  constexpr best::option<best::span<T>> take_first(size_t m)
    requires is_dynamic
  {
    if (m > size()) return best::none;
    auto [prefix, rest] = *split_at(m);
    *this = rest;
    return prefix;
  }

  /// # `span::take_last()`
  ///
  /// Splits this span at `m`; returns the suffix, and updates this to be the
  /// rest. If `m > size()`, returns best::none.
  constexpr best::option<best::span<T>> take_last(size_t m)
    requires is_dynamic
  {
    if (m > size()) return best::none;
    auto [rest, suffix] = *split_at(size() - m);
    *this = rest;
    return suffix;

    return best::none;
  }

  /// # `span[idx]`
  ///
  /// Extracts a single element. Crashes if the requested index is
  /// out-of-bounds.
  constexpr T& operator[](best::track_location<size_t> idx) const {
    best::bounds{.start = idx, .count = 1}.compute_count(size(), idx);
    return data()[idx];
  }

  /// # `span[best::index<n>]`
  ///
  /// Extracts a single element at a statically-specified index. If the
  /// requested index is out-of-bounds, and `this->is_static`, produces a
  /// compile time error; otherwise crashes.
  template <size_t idx>
  constexpr T& operator[](best::index_t<idx>) const
    requires((is_dynamic || idx < *extent))
  {
    return (*this)[idx];
  }

  /// # `span[{.start = ...}]`
  ///
  /// Extracts a subspan. Crashes if the requested range is out-of-bounds.
  constexpr best::span<T> operator[](best::bounds::with_location range) const {
    auto count = range.compute_count(size());
    return as_dynamic{data() + range.start, count};
  }

  /// # `span[best::vals<bounds{.start = ...}>]`
  ///
  /// Extracts a fixed-size subspan. If the requested index is out-of-bounds,
  /// and `this->is_static`, produces a compile time error; otherwise crashes.
  template <best::bounds range>
  constexpr auto operator[](best::vlist<range>) const
    requires(range.try_compute_count(extent).has_value())
  {
    constexpr auto count = range.try_compute_count(extent);
    return with_extent<count>(data() + range.start, *count);
  }

  /// # `span::at(idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// returns best::none.
  constexpr best::option<T&> at(size_t idx) const {
    if (idx < size()) {
      return data()[idx];
    }
    return best::none;
  }

  /// # `span::at({.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds, returns
  /// best::none.
  constexpr best::option<best::span<T>> at(best::bounds range) const {
    if (auto count = range.try_compute_count(size())) {
      return span<T>{data() + range.start, *count};
    }
    return best::none;
  }

  /// # `span::at(unsafe, idx)`
  ///
  /// Extracts a single element. If the requested index is out-of-bounds,
  /// Undefined Behavior.
  constexpr best::option<T&> at(unsafe, size_t idx) const {
    return data()[idx];
  }

  /// # `span::at(unsafe, {.start = ...})`
  ///
  /// Extracts a subspan. If the requested range is out-of-bounds,
  /// Undefined Behavior.
  constexpr best::span<T> at(unsafe, best::bounds range) const {
    size_t count = size() - range.start;
    if (range.end) {
      count = *range.end - range.start;
    } else if (range.including_end) {
      count = *range.end - range.start + 1;
    } else if (range.count) {
      count = *range.count;
    }

    return best::span<T>(data() + range.start, count);
  }

  /// # `span.swap()`
  ///
  /// Swaps the elements at indices `a` and `b`.
  constexpr void swap(size_t a, size_t b, best::location loc = best::here) const
    requires(!is_const)
  {
    using ::std::swap;
    swap(operator[]({a, loc}), operator[]({b, loc}));
  }

  /// # `span::reverse()`
  ///
  /// Reverses the order of the elements in this span, in-place.
  constexpr void reverse() const
    requires(!is_const)
  {
    for (size_t i = 0; i < size() / 2; ++i) {
      swap(i, size() - i - 1);
    }
  }

  /// # `span::split_at()`
  ///
  /// Splits this span at `idx` and returns both halves.
  constexpr best::option<std::array<best::span<T>, 2>> split_at(
      size_t idx) const {
    if (auto prefix = at({.end = idx})) {
      auto rest = *this;
      rest.repr_.data += idx;
      rest.repr_.size -= idx;
      return {{*prefix, rest}};
    }

    return best::none;
  }

  /// # `span::contains()`
  ///
  /// Performs a linear search for a matching element.
  constexpr bool contains(const best::equatable<T> auto& needle) const {
    for (const auto& candidate : *this) {
      if (candidate == needle) return true;
    }
    return false;
  }

  /// # `span::starts_with()`
  ///
  /// Checks if this span starts with a particular pattern.
  template <best::equatable<T> U = T>
  constexpr bool starts_with(best::span<const U> needle) const {
    return at({.end = needle.size()}) == needle;
  }

  /// # `span::ends_with()`
  ///
  /// Checks if this span ends with a particular pattern.
  template <best::equatable<T> U = T>
  constexpr bool ends_with(best::span<const U> needle) const {
    return at({.start = size() - needle.size()}) == needle;
  }

  /// # `span::strip_prefix()`
  ///
  /// If this span starts with `prefix`, removes it and returns the rest;
  /// otherwise returns `best::none`.
  template <best::equatable<T> U = T>
  constexpr best::option<best::span<T>> strip_prefix(
      best::span<const U> prefix) const {
    if (!starts_with(prefix)) return best::none;
    return at({.start = prefix.size()});
  }

  /// # `span::strip_suffix()`
  ///
  /// If this span ends with `suffix`, removes it and returns the rest;
  /// otherwise returns `best::none`.
  template <best::equatable<T> U = T>
  constexpr best::option<best::span<T>> strip_suffix(
      best::span<const U> suffix) const {
    if (!ends_with(suffix)) return best::none;
    return at({.end = size() - suffix.size()});
  }

  /// # `span::consume_prefix()`
  ///
  /// Like `strip_prefix()`, but returns a bool and updates the span in-place.
  template <best::equatable<T> U = T>
  constexpr bool consume_prefix(best::span<const U> prefix)
    requires is_dynamic
  {
    if (!starts_with(prefix)) return false;
    *this = *at({.start = prefix.size()});
    return true;
  }

  /// # `span::consume_suffix()`
  ///
  /// Like `strip_suffix()`, but returns a bool and updates the span in-place.
  template <best::equatable<T> U = T>
  constexpr bool consume_suffix(best::span<const U> suffix)
    requires is_dynamic
  {
    if (!ends_with(suffix)) return false;
    *this = *at({.end = size() - suffix.size()});
    return true;
  }

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
  void sort(
      best::callable<std::partial_ordering(const T&, const T&)> auto&&) const
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
      best::callable<std::partial_ordering(const T&, const T&)> auto&&) const
    requires(!is_const);

  /// # `span::copy_from()`
  ///
  /// Copies values from src. This has the same semantics as Go's `copy()`
  /// builtin: if the lengths are not equal, only the overlapping part is
  /// copied.
  template <typename U = const T>
  constexpr void copy_from(best::span<U> src) const
    requires(!is_const)
  {
    if (!std::is_constant_evaluated() && best::same<T, U> &&
        best::copyable<T, trivially>) {
      best::copy_bytes(*this, src);
      return;
    }

    unsafe::in([&](auto u) {
      size_t to_copy = best::min(size(), src.size());
      for (size_t i = 0; i < to_copy; ++i) {
        at(u, i) = src.at(u, i);
      }
    });
  }

  /// # `span::emplace_from()`
  ///
  /// Like `copy_from()`, but assumes this span's elements are uninitialized.
  template <typename U = const T>
  constexpr void emplace_from(best::span<U> src) const
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
  constexpr void destroy_in_place()
    requires(!is_const)
  {
    if constexpr (!best::is_debug() && best::destructible<T, trivially>) return;
    for (size_t i = 0; i < size(); ++i) {
      (data() + i).destroy_in_place();
    }
  }

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
  template <object_type U, best::option<size_t> m>
  constexpr bool operator==(best::span<U, m> that) const
    requires best::equatable<T, U>;
  template <contiguous R>
  constexpr bool operator==(const R& range) const
    requires best::equatable<T, decltype(*std::data(range))> && (!is_span<R>)
  {
    return *this == best::span(range);
  }

  template <object_type U, best::option<size_t> m>
  constexpr auto operator<=>(best::span<U, m> that) const
    requires best::comparable<T, U>;
  template <contiguous R>
  constexpr auto operator<=>(const R& range) const
    requires best::comparable<T, decltype(*std::data(range))> && (!is_span<R>)
  {
    return *this <=> best::span(range);
  }

  constexpr friend best::option<size_t> BestStaticSize(auto, span*) {
    return extent;
  }

 private:
  span_internal::repr<T, n> repr_;
};

template <typename T>
span(T*, size_t) -> span<T>;
template <typename T>
span(best::object_ptr<T>, size_t) -> span<T>;
template <typename T>
span(std::initializer_list<T>) -> span<const T>;
template <contiguous R>
span(R&& r) -> span<std::remove_reference_t<decltype(*std::data(r))>,
                    best::static_size<R>>;

// --- IMPLEMENTATION DETAILS BELOW ---

template <best::object_type T, best::option<size_t> n>
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

template <object_type T, best::option<size_t> n>
template <object_type U, best::option<size_t> m>
constexpr bool span<T, n>::operator==(span<U, m> that) const
  requires best::equatable<T, U>
{
  if (size() != that.size()) return false;
  if constexpr (best::byte_comparable<T, U>) {
    if (!std::is_constant_evaluated()) {
      return best::equate_bytes(span<T>(*this), span<U>(that));
    }
  }

  for (size_t i = 0; i < size(); ++i) {
    if (data()[i] != that.data()[i]) {
      return false;
    }
  }

  return true;
}

template <object_type T, best::option<size_t> n>
template <object_type U, best::option<size_t> m>
constexpr auto span<T, n>::operator<=>(span<U, m> that) const
  requires best::comparable<T, U>
{
  if constexpr (best::byte_comparable<T, U>) {
    if (!std::is_constant_evaluated()) {
      return best::order_type<T, U>(
          best::compare_bytes(span<T>(*this), span<U>(that)));
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

template <object_type T, option<size_t> n>
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