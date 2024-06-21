#ifndef BEST_META_EMPTY_H_
#define BEST_META_EMPTY_H_

#include <stddef.h>

#include <compare>
#include <type_traits>

#include "best/meta/init.h"
#include "best/meta/internal/init.h"
#include "best/meta/tags.h"
#include "best/meta/taxonomy.h"
#include "best/meta/traits.h"

//! Utilities for empty types.

namespace best {
/// An empty type with minimal dependencies.
struct empty final {
  constexpr bool operator==(const empty& that) const = default;
  constexpr std::strong_ordering operator<=>(const empty& that) const = default;
};

/// # `best::is_empty`
///
/// Whether this type counts as "empty", i.e., if it will trigger the empty
/// base class optimization. (Ish. Some compilers (MSVC) will lie about when
/// they will do the optimization.)
template <typename T>
concept is_empty = best::is_void<T> || std::is_empty_v<T>;

/// # `best::devoid<T>`
///
/// If `T` is void, produces `best::empty`. Otherwise produces `T`.
template <typename T>
using devoid = best::select<best::is_void<T>, empty, T>;

/// # `best::ebo`
///
/// A wrapper type over a `T` that is an empty type if `T` is empty. This allows
/// performing the empty base optimization even if `T` is `final`. However,
/// it must be trivially relocatable.
///
/// The "canonical" way to use this type is as follows:
///
/// ```
/// template <typename T>
/// class MyClass : best::ebo<T, MyClass> {
///   private:
///     using value_ = best::ebo<T, MyClass>
///
///   public:
///     void frob() { value_::get().frob(); }
/// }
/// ```
///
/// Doing a CTRP-type here ensures that this base is reasonably unique among
/// other potential bases of `MyClass`.
template <best::is_object T, typename Tag, auto ident = 0,
          bool empty = best::is_empty<T>, bool open = !std::is_final_v<T>,
          bool relo = best::relocatable<T, trivially>>
class ebo /* not final! */ {
 public:
  /// # `ebo::ebo(...)`
  ///
  /// Constructs a new `ebo` by calling the constructor of the wrapped type
  constexpr ebo(best::in_place_t, auto&&... args)
      : BEST_EBO_VALUE_(BEST_FWD(args)...) {}

  constexpr ebo() = default;
  constexpr ebo(const ebo&) = default;
  constexpr ebo& operator=(const ebo&) = default;
  constexpr ebo(ebo&&) = default;
  constexpr ebo& operator=(ebo&&) = default;

  /// # `ebo::get()`
  ///
  /// Returns the wrapped value.
  constexpr const T& get() const { return BEST_EBO_VALUE_; }
  constexpr T& get() { return BEST_EBO_VALUE_; }

 public:
  T BEST_EBO_VALUE_;
#define BEST_EBO_VALUE_ _private
};

template <best::is_object T, typename Tag, auto ident, bool relo>
class ebo<T, Tag, ident, true, true, relo> /* not final! */ : T {
 public:
  constexpr ebo(best::in_place_t, auto&&... args) : T(BEST_FWD(args)...) {}

  constexpr ebo()
    requires best::constructible<T>
  = default;
  constexpr ebo(const ebo&) = default;
  constexpr ebo& operator=(const ebo&) = default;
  constexpr ebo(ebo&&) = default;
  constexpr ebo& operator=(ebo&&) = default;

  constexpr const T& get() const { return *this; }
  constexpr T& get() { return *this; }
};

template <best::is_object T, typename Tag, auto ident>
class ebo<T, Tag, ident, true, false, true> /* not final! */ {
 public:
  constexpr ebo(best::in_place_t, auto&&... args) {
    (void)T(BEST_FWD(args)...);
  }

  constexpr ebo()
    requires best::constructible<T, trivially>
  = default;
  constexpr ebo()
    requires(best::constructible<T> && !best::constructible<T, trivially>)
  {
    (void)T();
  };
  constexpr ebo(const ebo&) = default;
  constexpr ebo& operator=(const ebo&) = default;
  constexpr ebo(ebo&&) = default;
  constexpr ebo& operator=(ebo&&) = default;

  constexpr const T& get() const { return value_; }
  constexpr T& get() { return value_; }

 private:
  inline static T value_;
};

}  // namespace best

#endif  // BEST_META_EMPTY_H_