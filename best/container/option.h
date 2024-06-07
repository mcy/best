#ifndef BEST_CONTAINER_OPTION_H_
#define BEST_CONTAINER_OPTION_H_

#include <compare>
#include <initializer_list>
#include <type_traits>

#include "best/base/fwd.h"
#include "best/container/choice.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

//! An optional type, like std::optional.
//!
//! best::option is the ideal type for dealing with "possibly missing" values.
//! Unlike std::optional, it is a complete replacement for raw pointers:
//! best::option<T&> has the same size and alignment as a pointer, while
//! offering safer options for manipulating it.
//!
//! best::option's API is a mix of that of std::optional and Rust's Option,
//! trying to pick the best alternatives from each.

namespace best {
/// A tag for constructing an empty best::optional<T>.
/// Analogous to std::nullopt and Rust's None.
///
///   best::option<int> x = best::none;
inline constexpr struct none_t {
} none;

/// Whether T is best::option<U> for some U.
template <typename T>
concept is_option =
    std::is_same_v<std::remove_cvref_t<T>,
                   best::option<typename std::remove_cvref_t<T>::type>>;

/// Given T = best::option<U>, returns U.
template <is_option T>
using option_type = typename std::remove_cvref_t<T>::type;

/// An optional value.
///
/// Constructing an best::option is very similar to constructing a
/// std::optional. The default constructor produces an empty option:
///
///   best::option<int> x;  // Empty, same as best::none.
///
/// It can also be constructed by implicitly converting a value into an
/// option that wraps it:
///
///   best::option<int> x = 42;
///   best::option<int&> r = *x;
///
/// best::option<void> can be tricky to construct, so we provide the
/// best::VoidOption constant for explicitly constructing one.
///
/// best::option can also be constructed via conversion. For example, if
/// T is convertible from const U&, then best::option<T> is convertible
/// from const best::option<U>&. In particular, this means that best::option<T>&
/// will convert into best::option<T&>.
///
/// best::option provides the same accessors as std::optional: has_value(),
/// value(), operator*, operator->. Unlike std::optional, all accesses perform
/// a runtime check.
///
/// best::option is comparable. An empty option compares as lesser than a
/// nonempty option.
///
/// best::option is a structural type: it can be used as the type of a non-type
/// template parameter.
template <typename T>
class option final {
 private:
  template <typename U>
  static constexpr bool cannot_init_from =
      !best::constructible<T, const U&> && !best::constructible<T, U&&> &&
      !best::constructible<T, const U&>;

  template <typename U>
  static constexpr bool not_forbidden_conversion =
      (!std::is_same_v<best::in_place_t, std::remove_cvref_t<U>>)&&(
          !std::is_same_v<
              option,
              std::remove_cvref_t<U>>)&&(!std::is_same_v<bool,
                                                         std::remove_cv_t<T>>);

 public:
  using type = T;
  using value_type = std::remove_cvref_t<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  /// Empty option constructors.
  ///
  /// This constructs an empty optional value regardless `T`. The default value
  /// of every option is empty.
  ///
  /// To construct a non-empty option containing the default value of T, if it
  /// has one, use `best::option<T>(best::in_place)`.
  constexpr option() : BEST_OPTION_IMPL_(best::index<0>) {}
  constexpr option(best::none_t) : option() {}
  constexpr option& operator=(best::none_t) {
    reset();
    return *this;
  }

  /// Copy/move constructors.
  ///
  /// best::optional calls the copy constructors of T when nonempty.
  constexpr option(const option&) = default;
  constexpr option& operator=(const option&) = default;
  constexpr option(option&&) = default;
  constexpr option& operator=(option&&) = default;

  /// Nonempty option constructor.
  ///
  /// This converts a value into an option containing it.
  template <typename U = T>
  constexpr option(U&& arg)
    requires not_forbidden_conversion<U> && best::constructible<T, U> &&
             (!best::moveable<std::remove_cvref_t<U>> || std::is_reference_v<T>)
      : option(best::in_place, BEST_FWD(arg)) {}
  template <typename U = T>
  constexpr option(U arg)
    requires not_forbidden_conversion<U> && best::constructible<T, U> &&
             best::moveable<U> && (!std::is_reference_v<T>)
      : option(best::in_place, std::move(arg)) {}

  /// Converting copy/move constructors.
  ///
  /// Constructs a best::option<T> by converting from a reference to
  /// best::option<U> by copy or move. It will also construct e.g. a
  /// best::option<T&> from a best::option<T>&.
  ///
  /// Conditionally explicit: only available when T is implicitly convertible
  /// from U.
  template <is_option U>
  constexpr explicit(
      !best::convertible<T, best::copy_ref<best::option_type<U>, U&&>>)
      option(U&& that)
    requires best::constructible<T,
                                 best::copy_ref<best::option_type<U>, U&&>> &&
             cannot_init_from<U>
      : option() {
    *this = BEST_FWD(that);
  }
  template <is_option U>
  constexpr option& operator=(U&& that)
    requires best::constructible<T,
                                 best::copy_ref<best::option_type<U>, U&&>> &&
             cannot_init_from<U>
  {
    if (that.has_value()) {
      if constexpr (best::void_type<best::option_type<U>>) {
        emplace();
      } else {
        emplace(*BEST_FWD(that));
      }
    } else {
      reset();
    }
    return *this;
  }

  /// Pointer constructors.
  ///
  /// Constructs a best::option<T&> out of a T*. nullptr is mapped to
  /// an empty optional.
  constexpr option(ptr that)
    requires std::is_reference_v<T>
      : option() {
    *this = that;
  }

  constexpr option& operator=(ptr that)
    requires std::is_reference_v<T>
  {
    if (that != nullptr) {
      emplace(*that);
    } else {
      reset();
    }
    return *this;
  }

  /// In-place constructors.
  ///
  /// Constructs a best::option<T> in-place with the given arguments. Use as
  ///
  ///   best::optional<T> my_opt(best::in_place, arg1, arg2, ...);
  ///
  /// This constructor is intended for constructing optionals that contain
  /// values that cannot be copied or moved.
  template <typename... Args>
  constexpr explicit option(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args...>
      : BEST_OPTION_IMPL_(best::index<1>, BEST_FWD(args)...) {}

  template <typename E, typename... Args>
  constexpr explicit option(best::in_place_t, std::initializer_list<E> il,
                            Args&&... args)
    requires best::constructible<T, std::initializer_list<E>, Args...>
      : BEST_OPTION_IMPL_(best::index<1>, il, BEST_FWD(args)...) {}

  /// Returns whether this option is empty.
  constexpr bool is_empty() const { return impl().which() == 0; }

  /// Returns whether this option contains a value.
  constexpr bool has_value() const { return !is_empty(); }
  constexpr explicit operator bool() const { return has_value(); }

  /// Resets this option.
  ///
  /// Equivalent to `my_opt = best::none;`.
  constexpr void reset() { impl().emplace(index<0>); }

  /// Constructs a version of this option that does not contain a reference.
  ///
  /// In other words, if this is an option<T&>, it constructs an option<T> by
  /// calling the copy constructor; if this is an option<T&&>, it calls the
  /// move constructor.
  constexpr best::option<value_type> copy() const& {
    return best::option<value_type>(*this);
  }
  constexpr best::option<value_type> copy() && {
    return best::option<value_type>(std::move(*this));
  }

  /// Constructs a version of this option that contains an rvalue reference.
  constexpr best::option<rref> move() {
    return best::option<rref>(std::move(*this));
  }

  /// Extracts the value of this option.
  ///
  /// Crashes if `is_empty()` is true.
  constexpr cref value(best::location loc = best::here) const& {
    return check_ok(loc), value(best::unsafe);
  }
  constexpr ref value(best::location loc = best::here) & {
    return check_ok(loc), value(best::unsafe);
  }
  constexpr crref value(best::location loc = best::here) const&& {
    return check_ok(loc), moved().value(best::unsafe);
  }
  constexpr rref value(best::location loc = best::here) && {
    return check_ok(loc), moved().value(best::unsafe);
  }

  /// Extracts the value of this option without checking.
  ///
  /// Undefined behavior if `is_empty()` is true.
  constexpr cref value(best::unsafe_t) const& {
    return impl().at(best::unsafe, index<1>);
  }
  constexpr ref value(best::unsafe_t) & {
    return impl().at(best::unsafe, index<1>);
  }
  constexpr crref value(best::unsafe_t) const&& {
    return moved().impl().at(best::unsafe, index<1>);
  }
  constexpr rref value(best::unsafe_t) && {
    return moved().impl().at(best::unsafe, index<1>);
  }

  /// Extracts the value of this option as a pointer.
  ///
  /// Returns nullptr if `is_empty()` is true.
  constexpr cptr as_ptr() const { return impl().as_ptr(index<1>); }
  constexpr ptr as_ptr() { return impl().as_ptr(index<1>); }

  /// Extracts the value of this option, or constructs a default with the given
  /// arguments and returns that.
  template <typename... Args>
  constexpr value_type value_or(Args&&... args) const& {
    return has_value() ? value() : value_type(BEST_FWD(args)...);
  }
  template <typename... Args>
  constexpr value_type value_or(Args&&... args) && {
    return has_value() ? moved().value() : value_type(BEST_FWD(args)...);
  }

  /// Constructs a new value in place.
  ///
  /// Returns a reference to the newly constructed value.
  template <typename... Args>
  constexpr ref emplace(Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    return impl().emplace(index<1>, BEST_FWD(args)...);
  }

  // This makes best::option into a smart pointer.
  constexpr cref operator*() const& { return value(); }
  constexpr ref operator*() & { return value(); }
  constexpr crref operator*() const&& { return moved().value(); }
  constexpr rref operator*() && { return moved().value(); }
  constexpr cptr operator->() const& {
    return check_ok(), impl().as_ptr(index<1>);
  }
  constexpr ptr operator->() & { return check_ok(), impl().as_ptr(index<1>); }

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, option opt) {
    if (!opt.has_value()) {
      return os << "none";
    } else if constexpr (std::is_void_v<T>) {
      return os << "option()";
    } else {
      return os << "option(" << *opt << ")";
    }
  }

  // Conversions w/ simple_option.
 private:
  using objT = std::conditional_t<best::object_type<T>, T, best::empty>;

 public:
  constexpr option(const container_internal::option<objT>& opt)
    requires best::object_type<T>
      : option() {
    if (opt) emplace(*opt);
  }
  constexpr option(container_internal::option<objT>&& opt)
    requires best::object_type<T>
      : option() {
    if (opt) emplace(std::move(*opt));
  }
  constexpr operator container_internal::option<objT>() const&
    requires best::object_type<T>
  {
    return is_empty() ? container_internal::option<objT>() : value();
  }
  constexpr operator container_internal::option<objT>() &&
    requires best::object_type<T>
  {
    return is_empty() ? container_internal::option<objT>() : moved().value();
  }

  // Comparisons.

  template <typename U>
  constexpr bool operator==(const best::option<U>& that) const
    requires best::equatable<T, U>
  {
    return impl() == that.impl();
  }
  template <typename U>
  constexpr bool operator==(const U& u) const
    requires best::equatable<T, U>
  {
    return operator==(best::option<const U&>(u));
  }
  template <typename U>
  constexpr bool operator==(const U* u) const

    requires best::equatable<T, U> && std::is_reference_v<T>
  {
    return operator==(best::option<const U&>(u));
  }
  constexpr bool operator==(const best::none_t&) const { return is_empty(); }

  template <typename U>
  constexpr best::order_type<T, U> operator<=>(
      const best::option<U>& that) const {
    return impl() <=> that.impl();
  }
  template <typename U>
  constexpr best::order_type<T, U> operator<=>(const U& u) const {
    return operator<=>(best::option<const U&>(u));
  }
  template <typename U>
  constexpr best::order_type<T, U> operator<=>(const U* u) const
    requires std::is_reference_v<T>
  {
    return operator<=>(best::option<const U&>(u));
  }
  constexpr std::strong_ordering operator<=>(const best::none_t&) const {
    return has_value() <=> false;
  }

 private:
  template <typename U>
  friend class option;

  /// Constructs an option from the corresponding best::choice type.
  template <typename Choice>
  constexpr explicit option(best::tlist<best::choice<void, T>>, Choice&& choice)
    requires best::same<best::choice<void, T>, std::remove_cvref_t<Choice>>
      : BEST_OPTION_IMPL_(BEST_FWD(choice)) {}

  constexpr const option&& moved() const {
    return static_cast<const option&&>(*this);
  }
  constexpr option&& moved() { return static_cast<option&&>(*this); }

  constexpr void check_ok(best::location loc = best::here) const {
    if (best::unlikely(is_empty())) {
      crash_internal::crash({"attempted access of empty best::option", loc});
    }
  }

  constexpr const auto& impl() const& { return BEST_OPTION_IMPL_; }
  constexpr auto& impl() & { return BEST_OPTION_IMPL_; }
  constexpr const auto&& impl() const&& {
    return static_cast<const best::choice<void, T>&&>(BEST_OPTION_IMPL_);
  }
  constexpr auto&& impl() && {
    return static_cast<best::choice<void, T>&&>(BEST_OPTION_IMPL_);
  }

 public:
  best::choice<void, T> BEST_OPTION_IMPL_;

  // This effectively makes the above field private by making it impossible to
  // name.
#define BEST_OPTION_IMPL_ private_
};

template <typename T>
option(T&&) -> option<std::remove_cvref_t<T>>;
option(best::none_t) -> option<void>;

// TODO: BestFmt
template <typename Os>
Os& operator<<(Os& os, none_t opt) {
  return os << "none";
}

inline constexpr best::option<void> VoidOption{best::in_place};

// Forward declare span as soon as possible.
template <typename, best::option<size_t>>
class span;
}  // namespace best

#endif  // BEST_CONTAINER_OPTION_H_