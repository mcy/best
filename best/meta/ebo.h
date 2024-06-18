#ifndef BEST_META_EBO_H_
#define BEST_META_EBO_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/internal/init.h"
#include "best/meta/tags.h"

//! A helper for the empty base class optimization.

namespace best {
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
template <best::object_type T, typename Tag, auto ident = 0,
          bool compressed =
              std::is_empty_v<T> &&
              // Use our intrinsic instead of the full init.h machinery, because
              // this type gets hammered by every best::object!
              best::init_internal::trivially_relocatable<T>>
class ebo /* not final! */ {
 public:
  /// # `ebo::ebo(...)`
  ///
  /// Constructs a new `ebo` by calling the constructor of the wrapped type
  constexpr ebo(best::in_place_t, auto&&... args)
      : BEST_EBO_VALUE_(BEST_FWD(args)...) {}

  constexpr ebo()
    requires best::constructible<T>
  = default;
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

template <best::object_type T, typename Tag, auto ident>
class ebo<T, Tag, ident, true> /* not final! */ {
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

#endif  // BEST_META_EBO_H_