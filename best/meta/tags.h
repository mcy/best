#ifndef BEST_META_TAGS_H_
#define BEST_META_TAGS_H_

#include <stddef.h>

#include <compare>
#include <utility>

//! Commonly-used tag types.
//!
//! These overlap with some of the ones from the STL; where possible,
//! they are aliases!
///
/// See https://abseil.io/tips/198 to get to know tag types better.

namespace best {
/// An empty type with minimal dependencies.
struct empty {
  constexpr bool operator==(const empty& that) const = default;
  constexpr std::strong_ordering operator<=>(const empty& that) const = default;
};

/// A helper for ranked overloading. See https://abseil.io/tips/229.
template <size_t n>
struct rank : rank<n - 1> {};
template <>
struct rank<0> {};

/// An alias for std::in_place.
///
/// Use this to tag variadic constructors that represent constructing a value
/// in place, to
struct in_place_t : std::in_place_t {
  in_place_t() = default;
};
inline constexpr in_place_t in_place;

/// An alias for std::in_place_index.
///
/// Use this to tag things that want to take a size_t.
template <size_t n>
struct index_t : std::in_place_index_t<n> {
  index_t() = default;
  static constexpr size_t value = n;
};
template <size_t n>
inline constexpr index_t<n> index;

/// A tag for uninitialized values.
///
/// Use this to define a non-default constructor that produces some kind of
/// "uninitialized" value.
struct uninit_t {};
inline constexpr uninit_t uninit;

/// # `best::ftadle`
///
/// A tag used for ensuring that FTADLE implementations are actually
/// FTADLEs. With some exceptions, every FTADLE implementation must tolerate
/// being passed ANY type in an unevaluated context. This also ensures we have
/// *a* type to pass in that position, so that calling the FTADLE finds
/// overloads in the best:: namespace, too.
struct ftadle;
}  // namespace best

#endif  // BEST_META_TAGS_H_