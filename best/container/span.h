#ifndef BEST_CONTAINER_SPAN_H_
#define BEST_CONTAINER_SPAN_H_

#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

#include "best/container/internal/span.h"
#include "best/container/object.h"
#include "best/container/option.h"
#include "best/log/location.h"
#include "best/meta/concepts.h"
#include "best/meta/ops.h"

//! Data spans.
//!
//! best::span<T, n> is a view into a contiguous array of n Ts. The extent n
//! may be dynamic, represented as best::span<T>.
//!
//! This header also provides concepts and traits for working with contiguous
//! ranges, i.e., ranges that can be represented as spans.

namespace best {
/// Whether T is a contiguous range that can be converted into a span.
///
/// This is defined as a type that enables calls to std::data and std::size().
template <typename T>
concept contiguous = requires(T&& t) {
  { *std::data(t) } -> best::ref_type;
  { std::size(t) } -> std::convertible_to<size_t>;
};

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

/// Whether T is a contiguous container of statically-known size.
template <typename T>
concept static_contiguous = static_size<T>.has_value();

/// Represents whether T is best::span<U, n> for some U, n.
template <typename T>
concept is_span =
    std::is_same_v<std::remove_cvref_t<T>,
                   best::span<typename std::remove_cvref_t<T>::type,
                              std::remove_cvref_t<T>::extent>>;

/// Given T = best::span<U, n>, returns U.
template <is_span T>
using span_type = typename std::remove_cvref_t<T>::type;

/// Given T = best::span<U, n>, returns n.
template <is_span T>
inline constexpr best::option<size_t> span_extent =
    std::remove_cvref_t<T>::extent;

/// A pointer and a length.
///
/// A span specifies a possibly cv-qualified element type and an optional
/// size. If the static size (given by span::extent) is best::none, the
/// span has _dynamic size_.
///
/// Spans are great when a function needs to take contiguous data as an
/// argument, since they can be constructed from any best::contiguous type,
/// including initializer lists.
///
/// Spans offer an API somewhat like std::span, but differs significantly when
/// constructing subspans. It also provides many helpers analogous to those on
/// Rust's slice type.
///
/// Like any other C++ array type, individual elements can be accessed with []:
///
///   best::span<int> sp = ...;
///   sp[5] *= 2;
///
/// Unlike C++, obtaining a subspan uses a best::bounds. For example, we can
/// write sp[{.start = 4, .end = 6}]. See bounds.h for more details on the
/// syntax. Also, all accesses to a span are always bounds-checked at runtime.
///
/// Spans are iterable; the iterator yields references of type `span::ref`.
/// All spans are comparable, even the mutable ones. They are compared in
/// lexicographic order.
template <typename T, best::option<size_t> n = best::none>
class span final {
 public:
  using type = T;
  using value_type = std::remove_cv_t<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  /// The extent of this span, if it is statically known.
  static constexpr best::option<size_t> extent = n;

  /// Whether this is a static span.
  static constexpr bool is_static = extent.has_value();
  /// Whether this is a dynamic span.
  static constexpr bool is_dynamic = extent.is_empty();

  /// Rebinds the extent of this span type.
  template <best::option<size_t> m>
  using with_extent = span<T, m>;

  /// Removes the static extent of this span type.
  using as_dynamic = with_extent<best::none>;

  /// Constructs a new empty span.
  ///
  /// This span's data pointer is always null.
  constexpr span()
    requires(is_dynamic || extent == 0)
  = default;

  /// Trivial to copy/move.
  constexpr span(const span&) = default;
  constexpr span& operator=(const span&) = default;
  constexpr span(span&&) = default;
  constexpr span& operator=(span&&) = default;

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

  /// Constructs a new span from a contiguous range.
  ///
  /// If this would construct a fixed-size span, and
  /// the list being constructed
  /// from has a different length, this constructor will crash.
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
    requires best::qualifies_to<
                 std::remove_reference_t<decltype(*std::data(range))>, T> &&
             (is_dynamic || best::static_size<Range>.is_empty() ||
              extent == best::static_size<Range>)
      : span(std::data(range), std::size(range), loc) {}

  /// Constructs a new span from an initializer list.
  ///
  /// If this would construct a fixed-size span, and the list being constructed
  /// from has a different length, this constructor will crash.
  constexpr explicit(is_static) span(std::initializer_list<value_type> il,  //
                                     best::location loc = best::here)
    requires std::is_const_v<T>
      : span(std::data(il), std::size(il), loc) {}

  /// Returns the data pointer for this span.
  constexpr best::object_ptr<T> data() const { return repr_.data; }

  /// Returns the size of this span.
  constexpr size_t size() const { return repr_.size; }

  /// Returns whether this span is empty.
  constexpr bool is_empty() const { return size() == 0; }

  /// Extracts a single element.
  ///
  /// Crashes if the requested index is out-of-bounds.
  constexpr ref operator[](best::track_location<size_t> idx) const {
    best::bounds{.start = idx, .count = 1}.compute_count(size(), idx);
    return data()[idx];
  }

  /// Extracts a single element at a statically-specified index.
  ///
  /// If the requested index is out-of-bounds, produces a compile time error
  /// instead of crashing if the span is of static extent.
  template <size_t idx>
  constexpr ref operator[](best::index_t<idx>) const
    requires((is_dynamic || idx < *extent))
  {
    return (*this)[idx];
  }

  /// Extracts a subspan.
  ///
  /// Crashes if the requested range is out-of-bounds, returns best::none.
  constexpr best::span<T> operator[](best::bounds::with_location range) const {
    auto count = range.compute_count(size());
    return as_dynamic{data() + range.start, count};
  }

  /// Extracts a fixed-size subspan.
  ///
  /// If the requested index is out-of-bounds, produces a compile time error
  /// instead of crashing if the span is of static extent.
  template <best::bounds range>
  constexpr auto operator[](best::vlist<range>) const
    requires(range.try_compute_count(extent).has_value())
  {
    constexpr auto count = range.try_compute_count(extent);
    return with_extent<count>(data() + range.start, *count);
  }

  /// Extracts a single element.
  ///
  /// If the requested index is out-of-bounds, returns best::none.
  constexpr best::option<ref> at(size_t idx) const {
    if (idx < size()) {
      return data()[idx];
    }
    return best::none;
  }

  /// Extracts a subspan.
  ///
  /// If the requested range is out-of-bounds, returns best::none.
  constexpr best::option<best::span<T>> at(best::bounds range) const {
    if (auto count = range.try_compute_count(size())) {
      return span<T>{data() + range.start, *count};
    }
    return best::none;
  }

  /// Splits this span at idx and returns both halves.
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

  /// Splits this span at `m`; returns the prefix, and updates this to be the
  /// rest.
  ///
  /// If `m > size()`, returns best::none.
  constexpr best::option<best::span<T>> take_first(size_t m) {
    if (auto split = split_at(m)) {
      auto [prefix, rest] = *split;
      *this = rest;
      return prefix;
    }

    return best::none;
  }

  /// Splits this span at `m`; returns the suffix, and updates this to be the
  /// rest.
  ///
  /// If `m > size()`, returns best::none.
  constexpr best::option<best::span<T>> take_last(size_t m) {
    if (auto split = split_at(size() - m)) {
      auto [rest, suffix] = *split;
      *this = rest;
      return suffix;
    }

    return best::none;
  }

  /// A span iterator.
  struct iter final {
    constexpr iter() = default;

    constexpr bool operator==(const iter& that) const = default;

    constexpr decltype(auto) operator*() const { return (*this)[0]; }
    constexpr best::as_ptr<T> operator->() const { return ptr_.operator->(); }
    constexpr decltype(auto) operator[](size_t idx) const {
      if constexpr (best::void_type<T>)
        return best::empty{};
      else {
        return ptr_[idx];
      }
    }

    constexpr iter& operator++() {
      if constexpr (best::void_type<T>) --idx_;
      ++ptr_;
      return *this;
    }
    constexpr iter operator++(int) {
      auto prev = *this;
      ++*this;
      return prev;
    }

    constexpr iter& operator--() {
      if constexpr (best::void_type<T>) ++idx_;
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
    constexpr iter(best::object_ptr<T> ptr, size_t idx) : ptr_(ptr) {
      if constexpr (best::void_type<T>) idx_ = idx;
    }
    best::object_ptr<T> ptr_;
    [[no_unique_address]] std::conditional_t<best::void_type<T>, size_t,
                                             best::empty>
        idx_{};
  };

  /// Spans are iterable ranges.
  constexpr iter begin() const { return {data(), size()}; }
  constexpr iter end() const { return {data() + size(), 0}; }

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, span sp) {
    os << "[";
    bool first = true;
    for (auto&& value : sp) {
      if (!std::exchange(first, false)) {
        os << ", ";
      }
      if constexpr (best::void_type<T>) {
        os << "void";
      } else {
        os << value;
      }
    }
    return os << "]";
  }

  // All spans are comparable.
  template <typename U, best::option<size_t> m>
  constexpr bool operator==(best::span<U, m> that) const
    requires best::equatable<T, U>
  {
    if constexpr (best::can_memcmp<T> && best::can_memcmp<U> &&
                  best::same<const T, const U>) {
      return size() == that.size() &&
             (data() == that.data() ||  // Optimize for the case where we are
                                        // comparing a span to itself!
              std::memcmp(data().raw(), that.data().raw(), size()) == 0);
    }

    if (size() != that.size()) {
      return false;
    }

    if constexpr (!best::void_type<T>) {
      for (size_t i = 0; i < size(); ++i) {
        if (data()[i] != that.data()[i]) {
          return false;
        }
      }
    }

    return true;
  }

  template <contiguous R>
  constexpr bool operator==(const R& range) const
    requires best::equatable<T, decltype(*std::data(range))> && (!is_span<R>)
  {
    return *this == best::span(range);
  }

  template <typename U, best::option<size_t> m>
  constexpr best::order_type<T, U> operator<=>(best::span<U, m> that) const {
    if constexpr (best::can_memcmp<T> && best::can_memcmp<U> &&
                  best::same<const T, const U>) {
      if (data() == that.data()) {
        return size() <=> that.size();
      }

      size_t prefix = std::min({size(), that.size()});
      int result =
          std::memcmp(data().raw(), that.data().raw(), prefix * sizeof(T));
      if (result < 0) {
        return std::strong_ordering::less;
      } else if (result > 0) {
        return std::strong_ordering::greater;
      } else {
        return size() <=> that.size();
      }
    }

    if constexpr (!best::void_type<T>) {
      size_t prefix = std::min({size(), that.size()});

      for (size_t i = 0; i < prefix; ++i) {
        if (auto result = data()[i] <=> that.data()[i]; result != 0) {
          return result;
        }
      }
    }

    return size() <=> that.size();
  }

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

template <typename T>
span(T*, size_t) -> span<T>;
template <typename T>
span(std::initializer_list<T>) -> span<const T>;
template <contiguous R>
span(R&& r) -> span<std::remove_reference_t<decltype(*std::data(r))>,
                    best::static_size<R>>;
}  // namespace best

#endif  // BEST_CONTAINER_SPAN_H_