#ifndef BEST_CONTAINER_OPTION_H_
#define BEST_CONTAINER_OPTION_H_

#include <initializer_list>

#include "best/base/fwd.h"
#include "best/container/choice.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/meta/init.h"
#include "best/meta/internal/init.h"
#include "best/meta/tags.h"

//! An optional type, like `std::optional`.
//!
//! `best::option` is the ideal type for dealing with "possibly missing" values.
//! Unlike `std::optional`, it is a complete replacement for raw pointers:
//! `best::option<T&>` has the same size and alignment as a pointer, while
//! offering safer options for manipulating it.
//!
//! `best::option`'s API is a mix of that of `std::optional` and Rust's
//! `Option`, trying to pick the `best` alternatives from each.

namespace best {
/// # `best::none`
///
/// A tag for constructing an empty `best::optional<T>`.
/// Analogous to `std::nullopt` and Rust's `None`.
///
/// ```
/// best::option<int> x = best::none;
/// ```
inline constexpr struct none_t {
  friend void BestFmt(auto& fmt, none_t) { fmt.write("none"); }
} none;

/// # `best::is_option`
///
/// Whether `T` is some `best::option<U>`.
template <typename T>
concept is_option =
    best::same<best::as_auto<T>, best::option<typename best::as_auto<T>::type>>;

/// # `best::option_type<T>`
///
/// Given `best::option<U>`, returns `U`.
template <is_option T>
using option_type = typename best::as_auto<T>::type;

/// # `best::option<T>`
///
/// An optional value.
///
/// ## Construction
///
/// Constructing an best::option is very similar to constructing a
/// `std::optional`. The default constructor produces an empty option:
///
/// ```
/// best::option<int> x;  // Empty, same as best::none.
/// ```
///
/// It can also be constructed by implicitly converting a value into an
/// option that wraps it:
///
/// ```
/// best::option<int> x = 42;
/// best::option<int&> r = *x;
/// ```
///
/// `best::option<void>` can be tricky to construct, so we provide the
/// `best::VoidOption` constant for explicitly constructing one.
///
/// `best::option` can also be constructed via conversion. For example, if
/// `T` is `best::convertible` from `const U&`, then `best::option<T>` is
/// convertible from `const best::option<U>&`. In particular, this means that
/// `best::option<T>&` will convert into `best::option<T&>`.
///
/// ## Other Operations
///
/// best::option provides the same accessors as `std::optional`: `has_value()`,
/// `value()`, `operator*`, `operator->`. Unlike `std::optional`, all accesses
/// perform a runtime check. `best::option` contextually converts to `bool`,
/// just like `std::optional`.
///
/// `best::option` is comparable. An empty option compares as lesser than a
/// nonempty option.
///
/// `best::option` is a structural type: it can be used as the type of a
/// non-type template parameter.
template <typename T>
class option final {
 private:
  template <typename U>
  static constexpr bool cannot_init_from =
      (!best::constructible<T, const U&> && !best::constructible<T, U&&>) ||
      best::is_void<T>;

  template <typename U>
  static constexpr bool not_forbidden_conversion =
      (!best::same<best::in_place_t, best::as_auto<U>>)&&  //
      (!best::same<option, best::as_auto<U>>)&&            //
      (!best::same<bool, best::unqual<T>>);

 public:
  /// Helper type aliases.
  using type = T;
  using value_type = best::as_auto<T>;

  using cref = best::as_ref<const type>;
  using ref = best::as_ref<type>;
  using crref = best::as_rref<const type>;
  using rref = best::as_rref<type>;
  using cptr = best::as_ptr<const type>;
  using ptr = best::as_ptr<type>;

  /// # `option::option()`
  ///
  /// This constructs an empty optional value regardless `T`. The default value
  /// of every option is empty.
  ///
  /// To construct a non-empty option containing the default value of `T`, if it
  /// has one, use `best::option<T>(best::in_place)`.
  constexpr option() : BEST_OPTION_IMPL_(best::index<0>) {}

  /// # `option::option(none)`
  ///
  /// Conversion from `best::none`.
  constexpr option(best::none_t) : option() {}
  constexpr option& operator=(best::none_t) {
    reset();
    return *this;
  }

  /// # `option::option(option)`
  ///
  /// Copy/move are trivial if they are trivial for `T`.
  constexpr option(const option&) = default;
  constexpr option& operator=(const option&) = default;
  constexpr option(option&&) = default;
  constexpr option& operator=(option&&) = default;

  /// # `option::option(T)`
  ///
  /// This converts a value into a non-empty option containing it.
  template <typename U>
  constexpr explicit(!best::convertible<T, U&&>) option(U&& arg)
    requires not_forbidden_conversion<U> && best::constructible<T, U&&> &&
             (!best::copyable<U, trivially> || !best::is_object<T>)
      : option(best::in_place, BEST_FWD(arg)) {}
  constexpr option(best::devoid<T> arg)
    requires best::moveable<T>
      : option(best::in_place, BEST_MOVE(arg)) {}
  template <typename U>
  constexpr explicit(!best::convertible<T, U&&>) option(best::bind_t, U&& arg)
    requires not_forbidden_conversion<U> && best::constructible<T, U&&>
      : option(best::in_place, BEST_FWD(arg)) {}
  constexpr option(best::bind_t, best::devoid<T> arg)
    requires best::moveable<T>
      : option(best::in_place, BEST_MOVE(arg)) {}

  // This constructor exists because otherwise, converting e.g. int to long
  // makes the compiler think there's a reference to temporary being bound that
  // doesn't need to be.
  template <typename U>
  constexpr explicit(!best::convertible<T, U>) option(U arg)
    requires not_forbidden_conversion<U> && best::constructible<T, U> &&
             best::copyable<U, trivially>
      : option(best::in_place, BEST_FWD(arg)) {}

  /// # `option::option(option<U>&)`
  ///
  /// Constructs a `best::option<T>` by converting from a reference to
  /// `best::option<U>` by copy or move. In particular, this enables
  ///
  /// ```
  /// best::option<int> x = 42;
  /// best::option<int&> y = x;
  /// ```
  ///
  /// Conditionally explicit: only available when `T` is implicitly convertible
  /// from `U`.
  template <is_option U>
  constexpr explicit(
      !best::convertible<T, best::refcopy<best::option_type<U>, U&&>>)
      option(U&& that)
    requires best::constructible<T, best::refcopy<best::option_type<U>, U&&>> &&
             cannot_init_from<U>
      : option() {
    *this = BEST_FWD(that);
  }
  template <is_option U>
  constexpr option& operator=(U&& that)
    requires best::constructible<T, best::refcopy<best::option_type<U>, U&&>> &&
             cannot_init_from<U>
  {
    if (that.has_value()) {
      if constexpr (best::is_void<best::option_type<U>>) {
        emplace();
      } else {
        emplace(*BEST_FWD(that));
      }
    } else {
      reset();
    }
    return *this;
  }

  /// # `option::option(T*)`
  ///
  /// Constructs a best::option<T&> out of a T*. nullptr is mapped to
  /// an empty optional.
  constexpr option(ptr that)
    requires best::is_ref<T>
      : option() {
    *this = that;
  }

  constexpr option& operator=(ptr that)
    requires best::is_ref<T>
  {
    if (that != nullptr) {
      emplace(*that);
    } else {
      reset();
    }
    return *this;
  }

  /// # `option::option(in_place, ...)`
  ///
  /// Constructs a `best::option<T>` in-place with the given arguments. E.g.
  ///
  /// ```
  /// best::option<int> my_opt(best::in_place, 42);
  /// ```
  ///
  /// This constructor is intended for constructing options that contain
  /// values with troublesome constructors.
  template <typename... Args>
  constexpr explicit option(best::in_place_t, Args&&... args)
    requires best::constructible<T, Args&&...>
      : BEST_OPTION_IMPL_(best::index<1>, BEST_FWD(args)...) {}

  template <typename E, typename... Args>
  constexpr explicit option(best::in_place_t, std::initializer_list<E> il,
                            Args&&... args)
    requires best::constructible<T, std::initializer_list<E>, Args&&...>
      : BEST_OPTION_IMPL_(best::index<1>, il, BEST_FWD(args)...) {}

  /// # `option::is_empty()`
  ///
  /// Returns whether this option is empty.
  // FIXME(mcyoung): is_none, is_some?
  constexpr bool is_empty() const { return impl().which() == 0; }

  /// # `option::has_value()`
  ///
  /// Returns whether this option contains a value.
  ///
  /// If passed a predicate argument, this will return whether this option has
  /// a value *and* it satisfies the predicate.
  constexpr bool has_value() const { return !is_empty(); }
  constexpr bool has_value(auto&& p) const {
    return impl().index_match([&](best::index_t<0>) { return false; },
                              [&](best::index_t<1>, auto&&... args) {
                                return best::call(BEST_FWD(p),
                                                  BEST_FWD(args)...);
                              });
  }
  constexpr explicit operator bool() const { return has_value(); }

  /// # `option::reset()`
  ///
  /// Resets this option.@
  ///
  /// Equivalent to `my_opt = best::none;`.
  constexpr void reset() { impl().emplace(index<0>); }

  /// # `option::copy()`.
  ///
  /// Constructs a version of this option that does not contain a reference.
  ///
  /// In other words, if this is an `option<T&>`, it constructs an `option<T>`
  /// by calling the copy constructor; if this is an `option<T&&>`, it calls the
  /// move constructor.
  constexpr best::option<value_type> copy() const& {
    return best::option<value_type>(*this);
  }
  constexpr best::option<value_type> copy() && {
    return best::option<value_type>(BEST_MOVE(*this));
  }

  /// # `option::as_ref()`.
  ///
  /// Constructs a version of this option that contains an appropriate `T&`.
  constexpr best::option<cref> as_ref() const& { return *this; }
  constexpr best::option<ref> as_ref() & { return *this; }
  constexpr best::option<crref> as_ref() const&& {
    return best::option<crref>(BEST_MOVE(*this));
  }
  constexpr best::option<rref> as_ref() && {
    return best::option<rref>(BEST_MOVE(*this));
  }

  /// # `option::as_object()`.
  ///
  /// Constructs a version of this option that contains a reference to the
  /// corresponding `best::object` type.
  constexpr best::option<const object<T>&> as_object() const& {
    return impl().object(best::index<1>);
  }
  constexpr best::option<object<T>&> as_object() & {
    return impl().object(best::index<1>);
  }
  constexpr best::option<const object<T>&&> as_object() const&& {
    return BEST_MOVE(*this).impl().object(best::index<1>);
  }
  constexpr best::option<object<T>&&> as_object() && {
    return BEST_MOVE(*this).impl().object(best::index<1>);
  }

  // TODO: expect.

  /// # `option::value()`.
  ///
  /// Extracts the value of this option. Crashes if `this->is_empty()`.
  constexpr cref value(best::location loc = best::here) const&;
  constexpr ref value(best::location loc = best::here) &;
  constexpr crref value(best::location loc = best::here) const&&;
  constexpr rref value(best::location loc = best::here) &&;

  /// # `option::value_or(...)`
  ///
  /// Extracts the value of this option by copy/move, or constructs a default
  /// with the given arguments and returns that.
  template <typename... Args>
  constexpr best::dependent<value_type, Args...> value_or(
      Args&&... args) const& {
    return has_value() ? value() : value_type(BEST_FWD(args)...);
  }
  template <typename... Args>
  constexpr best::dependent<value_type, Args...> value_or(Args&&... args) && {
    return has_value() ? BEST_MOVE(*this).value()
                       : value_type(BEST_FWD(args)...);
  }

  /// # `option::value_or([] { ... })`
  ///
  /// Extracts the value of this option by copy/move, or constructs a default
  /// with the given callback.
  constexpr auto value_or(best::callable<value_type()> auto&& or_else)
      const& -> best::dependent<value_type, decltype(or_else)> {
    return has_value() ? value() : best::call(BEST_FWD(or_else));
  }
  constexpr auto value_or(
      best::callable<value_type()> auto&&
          or_else) && -> best::dependent<value_type, decltype(or_else)> {
    return has_value() ? BEST_MOVE(*this).value()
                       : best::call(BEST_FWD(or_else));
  }

  /// # `option::value(unsafe)`.
  ///
  /// Extracts the value of this option without checking. Undefined behavior if
  /// `this->is_empty()`.
  constexpr cref value(unsafe u) const& { return impl().at(u, index<1>); }
  constexpr ref value(unsafe u) & { return impl().at(u, index<1>); }
  constexpr crref value(unsafe u) const&& {
    return BEST_MOVE(*this).impl().at(u, index<1>);
  }
  constexpr rref value(unsafe u) && {
    return BEST_MOVE(*this).impl().at(u, index<1>);
  }

  /// # `option::operator*, option::operator->`
  ///
  /// `best::option`'s contents can be accessed with the smart pointer
  /// operators. These internally simply defer to `value()`.
  constexpr cref operator*() const& { return value(); }
  constexpr ref operator*() & { return value(); }
  constexpr crref operator*() const&& { return BEST_MOVE(*this).value(); }
  constexpr rref operator*() && { return BEST_MOVE(*this).value(); }
  constexpr cptr operator->() const {
    return check_ok(), impl().as_ptr(index<1>);
  }
  constexpr ptr operator->() { return check_ok(), impl().as_ptr(index<1>); }

  /// # `option::map()`
  ///
  /// Applies a function to the contents of this option, and returns a new
  /// option with the result; maps `best::none` to `best::none`.
  ///
  /// If two arguments are provided, the first is used as a default, with the
  /// same semantics as `value_or()`.
  constexpr auto map(auto&& f) const&;
  constexpr auto map(auto&& f) &;
  constexpr auto map(auto&& f) const&&;
  constexpr auto map(auto&& f) &&;
  constexpr auto map(auto&& d, auto&& f) const&;
  constexpr auto map(auto&& d, auto&& f) &;
  constexpr auto map(auto&& d, auto&& f) const&&;
  constexpr auto map(auto&& d, auto&& f) &&;

  /// # `option::inspect()`
  ///
  /// Applies a function to the contents of this option, discards the result,
  /// and returns the original option.
  constexpr const option& inspect(auto&& f) const&;
  constexpr option& inspect(auto&& f) &;
  constexpr const option&& inspect(auto&& f) const&&;
  constexpr option&& inspect(auto&& f) &&;

  /// # `option::operator&`
  ///
  /// Returns `best::none` if `this->is_empty()`, else returns `that`.
  /// Arguments are eagerly evaluated.
  constexpr option operator&(auto&& that) const {
    if (is_empty()) return best::none;
    return BEST_FWD(that);
  }

  /// # `option::operator|`
  ///
  /// Returns this option if `this->has_value()`, else returns `that`.
  /// Arguments are eagerly evaluated.
  constexpr option operator|(auto&& that) const& {
    if (has_value()) return *this;
    return BEST_FWD(that);
  }
  constexpr option operator|(auto&& that) && {
    if (has_value()) return BEST_MOVE(*this);
    return BEST_FWD(that);
  }

  /// # `option::operator^`
  ///
  /// Returns whichever one of `this` and `that` is nonempty, or `best::none`
  /// if both are or are not empty.
  constexpr option operator^(auto&& that) const& {
    if (has_value() == that.has_value()) return best::none;
    if (has_value()) return *this;
    return BEST_FWD(that);
  }
  constexpr option operator^(auto&& that) && {
    if (has_value() == that.has_value()) return best::none;
    if (has_value()) return BEST_MOVE(*this);
    return BEST_FWD(that);
  }

  /// # `option::then()`
  ///
  /// Returns `best::none` if `this->is_empty()`, else calls `f` with this
  /// option's contents and returns the result.
  ///
  /// Some languages call this operation `flatmap`, `>>=`, or `and_then`.
  constexpr auto then(auto&& f) const&;
  constexpr auto then(auto&& f) &;
  constexpr auto then(auto&& f) const&&;
  constexpr auto then(auto&& f) &&;

  /// # `option::filter()`
  ///
  /// Returns `this` if `this->has_value(p)`; else returns `best::none`.
  constexpr option filter(auto&& p) const& {
    return has_value(BEST_FWD(p)) ? *this : best::none;
  }
  constexpr option filter(auto&& p) && {
    return has_value(BEST_FWD(p)) ? BEST_MOVE(*this) : best::none;
  }

  /// # `option::emplace()`
  ///
  /// Constructs a new value in place.
  ///
  /// Returns a reference to the newly constructed value.
  template <typename... Args>
  constexpr ref emplace(Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    return impl().emplace(index<1>, BEST_FWD(args)...);
  }

  /// # `option::or_emplace()`
  ///
  /// Returns the current value, or constructs a new value in place.
  template <typename... Args>
  constexpr ref or_emplace(Args&&... args)
    requires best::constructible<T, Args&&...>
  {
    if (has_value()) return **this;
    return emplace(BEST_FWD(args)...);
  }

  /// # `option::as_ptr()`
  ///
  /// Extracts the value of this option as a pointer.
  ///
  /// Returns nullptr if `is_empty()` is true.
  constexpr cptr as_ptr() const { return impl().as_ptr(index<1>); }
  constexpr ptr as_ptr() { return impl().as_ptr(index<1>); }

  /// # `option::ok_or()`, `option::err_or()`
  ///
  /// If this option has a value, `ok_or()` returns a `best::ok`; otherwise,
  /// returns a `best::err` constructed with the given arguments. `err_or()`
  /// does the opposite.
  ///
  /// Each function has four groups of overloads: one that takes an explicit
  /// template parameter and a variable number of arguments, which performs
  /// in-place construction; a variant that takes one argument and no template
  /// parameter, which deduces E to be the type of that argument; and a variant
  /// that takes a callback to construct the alternate.
  //
  // NOTE: The below 24 (yes, 24!) functions apparently cannot be outlined
  // without crashing clang in some configurations.
  // clang-format off
  template <typename E>
  constexpr best::result<T, E> ok_or(auto&&... args) const&
  requires best::constructible<T, cref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return make_ok(); return best::err(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<T, E> ok_or(auto&&... args) &
  requires best::constructible<T, ref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return make_ok(); return best::err(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<T, E> ok_or(auto&&... args) const&&
  requires best::constructible<T, crref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<T, E> ok_or(auto&&... args) &&
  requires best::constructible<T, rref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(BEST_FWD(args)...);
  }
  template <int&... deduction_barrier>
  constexpr auto ok_or(auto&& arg) const& -> best::result<T, best::as_auto<decltype(arg)>>
  requires best::constructible<T, cref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return make_ok(); return best::err(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto ok_or(auto&& arg) const&& -> best::result<T, best::as_auto<decltype(arg)>>
  requires best::constructible<T, ref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return make_ok(); return best::err(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto ok_or(auto&& arg) & -> best::result<T, best::as_auto<decltype(arg)>>
  requires best::constructible<T, crref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto ok_or(auto&& arg) && -> best::result<T, best::as_auto<decltype(arg)>>
  requires best::constructible<T, rref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(BEST_FWD(arg));
  }
  constexpr auto ok_or(best::callable<void()> auto&& arg) const&
  -> best::result<T, decltype(best::call(BEST_FWD(arg)))>
  requires best::constructible<T, cref> {
    if (has_value()) return make_ok(); return best::err(best::call(BEST_FWD(arg)));
  }
  constexpr auto ok_or(best::callable<void()> auto&& arg) const&&
  -> best::result<T, decltype(best::call(BEST_FWD(arg)))>
  requires best::constructible<T, ref> {
    if (has_value()) return make_ok(); return best::err(best::call(BEST_FWD(arg)));
  }
  constexpr auto ok_or(best::callable<void()> auto&& arg) &
  -> best::result<T, decltype(best::call(BEST_FWD(arg)))>
  requires best::constructible<T, crref> {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(best::call(BEST_FWD(arg)));
  }
  constexpr auto ok_or(best::callable<void()> auto&& arg) &&
  -> best::result<T, decltype(best::call(BEST_FWD(arg)))>
  requires best::constructible<T, rref> {
    if (has_value()) return BEST_MOVE(*this).make_ok(); return best::err(best::call(BEST_FWD(arg)));
  }

  template <typename E>
  constexpr best::result<E, T> err_or(auto&&... args) const&
  requires best::constructible<T, cref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return make_err(); return best::ok(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<E, T> err_or(auto&&... args) &
  requires best::constructible<T, ref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return make_err(); return best::ok(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<E, T> err_or(auto&&... args) const&&
  requires best::constructible<T, crref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(BEST_FWD(args)...);
  }
  template <typename E>
  constexpr best::result<E, T> err_or(auto&&... args) &&
  requires best::constructible<T, rref> && best::constructible<E, decltype(args)...> {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(BEST_FWD(args)...);
  }
  template <int&... deduction_barrier>
  constexpr auto err_or(auto&& arg) const& 
  -> best::result<best::as_auto<decltype(arg)>, T>
  requires best::constructible<T, cref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return make_err(); return best::ok(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto err_or(auto&& arg) const&&
  -> best::result<best::as_auto<decltype(arg)>, T>
  requires best::constructible<T, ref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return make_err(); return best::ok(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto err_or(auto&& arg) &
  -> best::result<best::as_auto<decltype(arg)>, T>
  requires best::constructible<T, crref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(BEST_FWD(arg));
  }
  template <int&... deduction_barrier>
  constexpr auto err_or(auto&& arg) &&
  -> best::result<best::as_auto<decltype(arg)>, T>
  requires best::constructible<T, rref> && (!best::callable<decltype(arg), void()>) {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(BEST_FWD(arg));
  }
  constexpr auto err_or(best::callable<void()> auto&& arg) const&
  -> best::result<decltype(best::call(BEST_FWD(arg))), T>
  requires best::constructible<T, cref> {
    if (has_value()) return make_err(); return best::ok(best::call(BEST_FWD(arg)));
  }
  constexpr auto err_or(best::callable<void()> auto&& arg) const&&
  -> best::result<decltype(best::call(BEST_FWD(arg))), T>
  requires best::constructible<T, ref> {
    if (has_value()) return make_err(); return best::ok(best::call(BEST_FWD(arg)));
  }
  constexpr auto err_or(best::callable<void()> auto&& arg) &
  -> best::result<decltype(best::call(BEST_FWD(arg))), T>
  requires best::constructible<T, crref> {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(best::call(BEST_FWD(arg)));
  }
  constexpr auto err_or(best::callable<void()> auto&& arg) &&
  -> best::result<decltype(best::call(BEST_FWD(arg))), T>
  requires best::constructible<T, rref> {
    if (has_value()) return BEST_MOVE(*this).make_err(); return best::ok(best::call(BEST_FWD(arg)));
  }
  // clang-format on

  friend void BestFmt(auto& fmt, const option& opt)
    requires requires { fmt.format(*opt.as_object()); }
  {
    if (!opt.has_value()) {
      fmt.write("none");
    } else {
      fmt.format("option({:!})", *opt.as_object());
    }
  }

  friend constexpr void BestFmtQuery(auto& query, option*) {
    query = query.template of<T>;
    query.requires_debug = true;
  }

  // Conversions w/ simple_option.
 private:
  using objT = best::select<best::is_object<T>, T, best::empty>;

 public:
  constexpr option(const container_internal::option<objT>& opt)
    requires best::is_object<T>
      : option() {
    if (opt) emplace(*opt);
  }
  constexpr option(container_internal::option<objT>&& opt)
    requires best::is_object<T>
      : option() {
    if (opt) emplace(BEST_MOVE(*opt));
  }
  constexpr operator container_internal::option<objT>() const&
    requires best::is_object<T>
  {
    return is_empty() ? container_internal::option<objT>() : value();
  }
  constexpr operator container_internal::option<objT>() &&
    requires best::is_object<T>
  {
    return is_empty() ? container_internal::option<objT>()
                      : BEST_MOVE(*this).value();
  }

  // Comparisons.
  template <best::equatable<T> U>
  constexpr bool operator==(const best::option<U>& that) const {
    return impl() == that.impl();
  }
  template <best::equatable<T> U>
  constexpr bool operator==(const U& u) const {
    return operator==(best::option<const U&>(u));
  }
  template <best::equatable<T> U>
  constexpr bool operator==(const U* u) const
    requires best::is_ref<T>
  {
    return operator==(best::option<const U&>(u));
  }
  constexpr bool operator==(const best::none_t&) const { return is_empty(); }

  template <best::comparable<T> U>
  constexpr best::order_type<T, U> operator<=>(
      const best::option<U>& that) const {
    return impl() <=> that.impl();
  }
  template <best::comparable<T> U>
  constexpr best::order_type<T, U> operator<=>(const U& u) const {
    return operator<=>(best::option<const U&>(u));
  }
  template <best::comparable<T> U>
  constexpr best::order_type<T, U> operator<=>(const U* u) const
    requires best::is_ref<T>
  {
    return operator<=>(best::option<const U&>(u));
  }
  constexpr best::ord operator<=>(const best::none_t&) const {
    return has_value() <=> false;
  }

 private:
  template <typename U>
  friend class option;

  /// Constructs an option from the corresponding best::choice type.
  template <typename Choice>
  constexpr explicit option(best::tlist<best::choice<void, T>>, Choice&& choice)
    requires best::same<best::choice<void, T>, best::as_auto<Choice>>
      : BEST_OPTION_IMPL_(BEST_FWD(choice)) {}

  constexpr void check_ok(best::location loc = best::here) const {
    if (best::unlikely(is_empty())) {
      crash_internal::crash({"attempted access of empty best::option", loc});
    }
  }

  constexpr const auto& impl() const& { return BEST_OPTION_IMPL_; }
  constexpr auto& impl() & { return BEST_OPTION_IMPL_; }
  constexpr const auto&& impl() const&& { return BEST_MOVE(BEST_OPTION_IMPL_); }
  constexpr auto&& impl() && { return BEST_MOVE(BEST_OPTION_IMPL_); }

  template <typename... Args>
  static constexpr auto empty_ok() {
    return best::ok<Args...>();
  }
  constexpr auto make_ok() const& {
    if constexpr (best::is_void<T>) {
      return empty_ok<>();
    } else {
      return best::ok(**this);
    }
  }
  constexpr auto make_ok() & {
    if constexpr (best::is_void<T>) {
      return empty_ok<>();
    } else {
      return best::ok(**this);
    }
  }
  constexpr auto make_ok() const&& {
    if constexpr (best::is_void<T>) {
      return empty_ok<>();
    } else {
      return best::ok(*BEST_MOVE(*this));
    }
  }
  constexpr auto make_ok() && {
    if constexpr (best::is_void<T>) {
      return empty_ok<>();
    } else {
      return best::ok(*BEST_MOVE(*this));
    }
  }

  template <typename... Args>
  static constexpr auto empty_err() {
    return best::err<Args...>();
  }
  constexpr auto make_err() const& {
    if constexpr (best::is_void<T>) {
      return empty_err<>();
    } else {
      return best::err(**this);
    }
  }
  constexpr auto make_err() & {
    if constexpr (best::is_void<T>) {
      return empty_err<>();
    } else {
      return best::err(**this);
    }
  }
  constexpr auto make_err() const&& {
    if constexpr (best::is_void<T>) {
      return empty_err<>();
    } else {
      return best::err(*BEST_MOVE(*this));
    }
  }
  constexpr auto make_err() && {
    if constexpr (best::is_void<T>) {
      return empty_err<>();
    } else {
      return best::err(*BEST_MOVE(*this));
    }
  }

 public:
  best::choice<void, T> BEST_OPTION_IMPL_;

  // This effectively makes the above field private by making it impossible to
  // name.
#define BEST_OPTION_IMPL_ private_
};

template <typename T>
option(T&&) -> option<best::as_auto<T>>;
template <typename T>
option(best::bind_t, T&&) -> option<T&&>;
option(best::none_t) -> option<void>;

inline constexpr best::option<void> VoidOption{best::in_place};

// Forward declare span as soon as possible.
template <best::is_object, best::option<size_t> = best::none>
class span;

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T>
constexpr option<T>::cref option<T>::value(best::location loc) const& {
  return check_ok(loc), value(unsafe("check_ok() called before this"));
}
template <typename T>
constexpr option<T>::ref option<T>::value(best::location loc) & {
  return check_ok(loc), value(unsafe("check_ok() called before this"));
}
template <typename T>
constexpr option<T>::crref option<T>::value(best::location loc) const&& {
  return check_ok(loc),
         BEST_MOVE(*this).value(unsafe("check_ok() called before this"));
}
template <typename T>
constexpr option<T>::rref option<T>::value(best::location loc) && {
  return check_ok(loc),
         BEST_MOVE(*this).value(unsafe("check_ok() called before this"));
}

template <typename T>
constexpr auto option<T>::map(auto&& f) const& {
  using U = best::call_result<decltype(f), cref>;
  return impl().index_match(
      [&](best::index_t<0>) -> option<U> { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> option<U> {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}
template <typename T>
constexpr auto option<T>::map(auto&& f) & {
  using U = best::call_result<decltype(f), ref>;
  return impl().index_match(
      [&](best::index_t<0>) -> option<U> { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> option<U> {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}
template <typename T>
constexpr auto option<T>::map(auto&& f) const&& {
  using U = best::call_result<decltype(f), crref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>) -> option<U> { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> option<U> {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}
template <typename T>
constexpr auto option<T>::map(auto&& f) && {
  using U = best::call_result<decltype(f), rref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>) -> option<U> { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> option<U> {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}

template <typename T>
constexpr auto option<T>::map(auto&& d, auto&& f) const& {
  return map(BEST_FWD(f)).value_or(BEST_FWD(d));
}
template <typename T>
constexpr auto option<T>::map(auto&& d, auto&& f) & {
  return map(BEST_FWD(f)).value_or(BEST_FWD(d));
}
template <typename T>
constexpr auto option<T>::map(auto&& d, auto&& f) const&& {
  return BEST_MOVE(*this).map(BEST_FWD(f)).value_or(BEST_FWD(d));
}
template <typename T>
constexpr auto option<T>::map(auto&& d, auto&& f) && {
  return BEST_MOVE(*this).map(BEST_FWD(f)).value_or(BEST_FWD(d));
}

template <typename T>
constexpr const option<T>& option<T>::inspect(auto&& f) const& {
  impl().index_match([&](best::index_t<0>) {},
                     [&](best::index_t<1>, auto&&... args) {
                       best::call(BEST_FWD(f), BEST_FWD(args)...);
                     });
  return *this;
}
template <typename T>
constexpr option<T>& option<T>::inspect(auto&& f) & {
  impl().index_match([&](best::index_t<0>) {},
                     [&](best::index_t<1>, auto&&... args) {
                       best::call(BEST_FWD(f), BEST_FWD(args)...);
                     });
  return *this;
}
template <typename T>
constexpr const option<T>&& option<T>::inspect(auto&& f) const&& {
  BEST_MOVE(*this).impl().index_match([&](best::index_t<0>) {},
                                      [&](best::index_t<1>, auto&&... args) {
                                        best::call(BEST_FWD(f),
                                                   BEST_FWD(args)...);
                                      });
  return BEST_MOVE(*this);
}
template <typename T>
constexpr option<T>&& option<T>::inspect(auto&& f) && {
  BEST_MOVE(*this).impl().index_match([&](best::index_t<0>) {},
                                      [&](best::index_t<1>, auto&&... args) {
                                        best::call(BEST_FWD(f),
                                                   BEST_FWD(args)...);
                                      });
  return BEST_MOVE(*this);
}

template <typename T>
constexpr auto option<T>::then(auto&& f) const& {
  using U = best::unref<best::call_result<decltype(f), cref>>;
  return impl().index_match([&](best::index_t<0>) -> U { return best::none; },
                            [&](best::index_t<1>, auto&&... args) -> U {
                              return best::call(BEST_FWD(f), BEST_FWD(args)...);
                            });
}
template <typename T>
constexpr auto option<T>::then(auto&& f) & {
  using U = best::unref<best::call_result<decltype(f), ref>>;
  return impl().index_match([&](best::index_t<0>) -> U { return best::none; },
                            [&](best::index_t<1>, auto&&... args) -> U {
                              return best::call(BEST_FWD(f), BEST_FWD(args)...);
                            });
}
template <typename T>
constexpr auto option<T>::then(auto&& f) const&& {
  using U = best::unref<best::call_result<decltype(f), crref>>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>) -> U { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}
template <typename T>
constexpr auto option<T>::then(auto&& f) && {
  using U = best::unref<best::call_result<decltype(f), rref>>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>) -> U { return best::none; },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      });
}

constexpr auto BestGuardResidual(auto&&, is_option auto&&) { return none; }
constexpr auto BestGuardReturn(auto&&, is_option auto&&, auto&&) {
  return none;
}
}  // namespace best

#endif  // BEST_CONTAINER_OPTION_H_