#ifndef BEST_CONTAINER_OBJECT_H_
#define BEST_CONTAINER_OBJECT_H_

#include <stddef.h>

#include <compare>
#include <concepts>
#include <memory>
#include <type_traits>

#include "best/base/port.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

//! Objectification: turn any C++ type into a something you can use as a class
//! member.

namespace best {
template <typename>
class object;

/// Completely identical to std::invoke, except that if F returns void, it
/// returns best::empty instead.
template <typename F, typename... Args>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) invoke(F&& f, Args&&... args)
  requires std::invocable<F, Args...>
{
  if constexpr (best::void_type<std::invoke_result_t<F, Args...>>) {
    std::invoke(BEST_FWD(f), BEST_FWD(args)...);
    return best::empty{};
  } else {
    return std::invoke(BEST_FWD(f), BEST_FWD(args)...);
  }
}

/// A tag for constructing niche representations.
///
/// A niche representation of a type T is a best::object<T> that contains an
/// otherwise invalid value of T. No operations need to be valid for a niche
/// representation, not even the destructor.
///
/// If T has a niche representation, it must satisfy best::constructible<T,
/// niche> and best::equatable<T, niche>. Only the niche representation
/// (obtained only by constructing from a best::niche) must compare as equal
/// to best::niche.
///
/// Niche representations are used for compressing the layout of some types,
/// such as best::choice.
struct niche final {};

/// Whether T is a type with a niche.
template <typename T>
concept has_niche = best::ref_type<T> || (best::object_type<T> &&
                                          best::constructible<T, best::niche> &&
                                          best::equatable<T, best::niche>);

/// A pointer to a possibly non-object T.
///
/// This allows creating a handle to a possibly non-object type that can be
/// manipulated in a consistent manner.
///
/// The mapping is as follows:
///
///   best::object_type T          -> T*
///   best::ref_type T             -> const base::as_ptr<T>*
///   best::abominable_func_type T -> const base::as_ptr<T>*
///   best::void_type T            -> void*
///
/// Note the `const` for reference types: this reflects the fact that e.g.
/// `T& const` is just `T&`.
///
/// Note that a best::object_ptr<void> is *not* a void* in so far that it does
/// not represent a type-erased pointer.
template <typename T>
class object_ptr final {
 public:
  /// The wrapped type; object_ptr<T> is nominally a T*.
  using type = T;

  /// The underlying pointee type.
  using pointee = std::conditional_t<best::object_type<T> || best::void_type<T>,
                                     T, const best::as_ptr<T>>;

  using cref = best::as_ref<const T>;
  using ref = best::as_ref<T>;
  using crref = best::as_rref<const T>;
  using rref = best::as_rref<T>;
  using cptr = best::as_ptr<const T>;
  using ptr = best::as_ptr<T>;

  /// Constructs a null pointer.
  constexpr object_ptr() = default;
  constexpr object_ptr(std::nullptr_t){};

  constexpr object_ptr(const object_ptr&) = default;
  constexpr object_ptr& operator=(const object_ptr&) = default;
  constexpr object_ptr(object_ptr&&) = default;
  constexpr object_ptr& operator=(object_ptr&&) = default;

  /// Wraps an underlying pointer type.
  template <typename U>
  constexpr explicit(!best::qualifies_to<U, pointee> && !best::void_type<T>)
      object_ptr(U* ptr)
      : BEST_OBJECT_PTR_(ptr) {}

  template <typename U>
  constexpr explicit(!best::qualifies_to<U, pointee> && !best::void_type<T>)
      object_ptr(object_ptr<U> ptr)
      : BEST_OBJECT_PTR_(
            const_cast<std::remove_const_t<typename object_ptr<U>::pointee>*>(
                ptr.raw())) {}

  /// Returns a non-null but dangling pointer, which is unique for the choice of
  /// `T`.
  static constexpr object_ptr dangling() { return std::addressof(dangling_); }

  /// Pointers may be compared by address.
  template <typename U>
  constexpr bool operator==(object_ptr<U> that) const {
    return best::addr_eq(raw(), that.raw());
  };
  template <typename U>
  constexpr std::strong_ordering operator<=>(object_ptr<U> that) const {
    return best::addr_cmp(raw(), that.raw());
  };
  template <typename U>
  constexpr bool operator==(U* that) const {
    return best::addr_eq(raw(), that);
  };
  template <typename U>
  constexpr std::strong_ordering operator<=>(U* that) const {
    return best::addr_cmp(raw(), that);
  };
  constexpr operator pointee*() const { return raw(); }

  /// Dereferences this pointer.
  ///
  /// For example, this will dereference the wrapping pointer of a T&, so
  /// if T = U&, then raw = U** and this dereferences that twice.
  constexpr ref operator*() const {
    if constexpr (best::object_type<T>) {
      return *raw();
    } else if constexpr (best::void_type<T>) {
      return;
    } else {
      return **raw();
    }
  }

  /// Performs raw pointer arithmetic.
  constexpr ref operator[](size_t idx) const { return *(*this + idx); }

  constexpr object_ptr operator+(ptrdiff_t idx) const {
    if constexpr (std::is_void_v<T>) {
      return *this;
    } else {
      return object_ptr(raw() + idx);
    }
  }
  constexpr object_ptr operator+=(ptrdiff_t idx) {
    *this = *this + idx;
    return *this;
  }
  constexpr object_ptr operator++() { return *this += 1; }
  constexpr object_ptr operator++(int) {
    auto prev = *this;
    ++*this;
    return *this;
  }

  constexpr object_ptr operator-(ptrdiff_t idx) const {
    if constexpr (std::is_void_v<T>) {
      return *this;
    } else {
      return object_ptr(raw() - idx);
    }
  }
  constexpr object_ptr operator-=(ptrdiff_t idx) {
    *this = *this - idx;
    return *this;
  }
  constexpr object_ptr operator--() { return *this -= 1; }
  constexpr object_ptr operator--(int) {
    auto prev = *this;
    --*this;
    return *this;
  }

  /// Converts a reference to as_object<T> to as_ptr<T>.
  ///
  /// For example, this will return the wrapping pointer of a T&, or take
  /// the address of an object.
  BEST_INLINE_SYNTHETIC constexpr ptr operator->() const {
    if constexpr (best::object_type<T> || best::void_type<T>) {
      return raw();
    } else {
      return *raw();
    }
  }

  /// Constructs in place according to the constructors valid per
  /// best::constructible.
  ///
  /// Note that this function implicitly casts away an untracked const if called
  /// on an object_ptr<T&> or object_ptr<T()>.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct_in_place(Args&&... args) const
    requires best::constructible<T, Args&&...>
  {
    if constexpr (best::object_type<T>) {
      std::construct_at(raw(), BEST_FWD(args)...);
    } else if constexpr (best::ref_type<T>) {
      *const_cast<std::remove_const_t<pointee>*>(raw()) =
          std::addressof(args...);
    } else if constexpr (best::abominable_func_type<T>) {
      *const_cast<std::remove_const_t<pointee>*>(raw()) = (args, ...);
    }
  }

  /// Constructs a niche representation in-place.
  BEST_INLINE_SYNTHETIC constexpr void construct_in_place(niche) const
    requires best::has_niche<T>
  {
    if constexpr (best::object_type<T>) {
      std::construct_at(raw(), niche{});
    } else if constexpr (best::ref_type<T>) {
      *const_cast<std::remove_const_t<pointee>*>(raw()) = nullptr;
    }
  }

  /// Destroys the pointed to object in place.
  BEST_INLINE_SYNTHETIC constexpr void destroy_in_place() const
    requires best::destructible<T>
  {
    if constexpr (best::object_type<T>) {
      std::destroy_at(raw());
      if (!std::is_constant_evaluated() && best::is_debug()) {
        std::memset(raw(), 0xcd, sizeof(pointee));
      }
    }
  }

  /// Constructs in place according to the constructors valid per
  /// best::constructible.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(Args&&... args) const
    requires best::assignable<T, Args&&...>
  {
    if constexpr (best::object_type<T>) {
      **this = (BEST_FWD(args), ...);
    } else {
      construct_in_place(BEST_FWD(args)...);
    }
  }

  /// Constructs a copy of `that`'s pointee.
  ///
  /// `is_init` specifies whether to construct or assign in-place.
  BEST_INLINE_SYNTHETIC constexpr void copy_from(object_ptr<const T> that,
                                                 bool is_init) const
    requires best::copyable<T>
  {
    if constexpr (!best::void_type<T>) {
      if (is_init) {
        assign(*that);
      } else {
        construct_in_place(*that);
      }
    }
  }

  /// Constructs a move of `that`'s pointee.
  ///
  /// `is_init` specifies whether to construct or assign in-place.
  BEST_INLINE_SYNTHETIC constexpr void move_from(object_ptr that,
                                                 bool is_init) const
    requires best::moveable<T>
  {
    if constexpr (!best::void_type<T>) {
      if (is_init) {
        assign(static_cast<rref>(*that));
      } else {
        construct_in_place(static_cast<rref>(*that));
      }
    }
  }

  /// Relocates `that`'s pointee. In other words, this moves from and then
  /// destroys *that.
  ///
  /// `is_init` specifies whether to construct or assign in-place.
  BEST_INLINE_SYNTHETIC constexpr void relocate_from(object_ptr that,
                                                     bool is_init) const
    requires best::relocatable<T>
  {
    move_from(that, is_init);
    that.destroy_in_place();
  }

  /// Whether this value is a niche representation.
  constexpr bool is_niche() const {
    if constexpr (best::has_niche<T>) {
      if constexpr (best::ref_type<T>) {
        return *raw() == nullptr;
      } else {
        return **this == niche{};
      }
    }
    return false;
  }

  /// Returns the raw underlying pointer.
  constexpr pointee* raw() const { return BEST_OBJECT_PTR_; }

 private:
  inline static std::conditional_t<best::void_type<T>, best::empty, pointee>
      dangling_;

 public:
  // Public for structural-ness.
  pointee* BEST_OBJECT_PTR_;
#define BEST_OBJECT_PTR_ _private
};

/// An "equivalent" object type for any type, intended primarily for generic
/// code.
///
/// This type wraps any T and reproduces its properties.
///
/// The wrapped T can be accessed via operator* and operator->.
template <typename T>
class object {
 public:
  /// The type we're wrapping internally.
  using wrapped_type = std::conditional_t<
      best::void_type<T>, best::empty,
      std::conditional_t<best::object_type<T>, std::remove_cv_t<T>,
                         best::as_ptr<T>>>;

  using type = T;
  using value_type = std::remove_cvref_t<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  constexpr object() = default;
  constexpr object(const object&) = default;
  constexpr object& operator=(const object&) = default;
  constexpr object(object&&) = default;
  constexpr object& operator=(object&&) = default;

  /// Constructs an object by deferring to the appropriate downstream
  /// constructor.
  template <typename... Args>
  constexpr explicit object(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...> && (!best::object_type<T>)
  {
    as_ptr().construct_in_place(BEST_FWD(args)...);
  }
  template <typename... Args>
  constexpr explicit object(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...> && best::object_type<T>
      : BEST_OBJECT_INNER_(BEST_FWD(args)...) {}

  constexpr explicit object(best::in_place_t, best::niche)
    requires best::ref_type<T>
      : BEST_OBJECT_INNER_(nullptr) {}

  template <typename Arg>
  constexpr object& operator=(Arg&& arg)
    requires best::assignable<T, Arg>
  {
    as_ptr().assign(BEST_FWD(arg));
    return *this;
  }

  /// Extracts an object_ptr pointing to this object.
  constexpr best::object_ptr<const T> as_ptr() const {
    return best::object_ptr<const T>(std::addressof(BEST_OBJECT_INNER_));
  }
  constexpr best::object_ptr<T> as_ptr() {
    return best::object_ptr<T>(std::addressof(BEST_OBJECT_INNER_));
  }

  // best::object is a smart pointer.
  constexpr cref operator*() const& { return *as_ptr(); }
  constexpr ref operator*() & { return *as_ptr(); }
  constexpr crref operator*() const&& { return static_cast<crref>(**this); }
  constexpr rref operator*() && { return static_cast<rref>(**this); }
  constexpr cptr operator->() const { return as_ptr().operator->(); }
  constexpr ptr operator->() { return as_ptr().operator->(); }

  constexpr bool operator==(const object& that) const
    requires best::equatable<T, T>
  {
    if constexpr (best::void_type<T>) {
      return true;
    } else {
      return **this == *that;
    }
  }

 public:
  // Public for structural-ness.
  [[no_unique_address]] wrapped_type BEST_OBJECT_INNER_;
#define BEST_OBJECT_INNER_ _private
};
}  // namespace best

#endif  // BEST_CONTAINER_OBJECT_H_