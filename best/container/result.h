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

#ifndef BEST_CONTAINER_RESULT_H_
#define BEST_CONTAINER_RESULT_H_

#include "best/container/choice.h"
#include "best/container/row.h"
#include "best/meta/tags.h"

//! A result type, like `std::expected`.
//!
//! `best::result` is intended for representing fallible operations. It's like
//! `std::expected`, but carries all the usability of `best::option`.
//!
//! `best::result`'s API is a close replication of Rust's `Result` type.

namespace best {
/// # `best::ok`
///
/// Helper for constructing a `best::result`. It constructs a success value.
///
/// ```
/// best::result<int, error> compute() {
///   return best::ok(42);
/// }
/// ```
template <typename... Args>
struct ok {
  constexpr ok(Args... args) : row(BEST_FWD(args)...) {}
  best::row<Args...> row;

  friend void BestFmt(auto& fmt, const ok& ok)
    requires requires { fmt.format(ok.row); }
  {
    fmt.write("ok");
    fmt.format(ok.row);
  }
  friend constexpr void BestFmtQuery(auto& query, ok*) {
    query = query.template of<best::row<Args...>>;
  }
};
template <typename... Args>
ok(Args&&...) -> ok<Args&&...>;

/// # `best::err`
///
/// Helper for constructing a `best::result`. It constructs an error value.
///
/// ```
/// best::result<int, error> compute() {
///   return best::err("not yet implemented");
/// }
/// ```
template <typename... Args>
struct err {
  constexpr err(Args... args) : row(BEST_FWD(args)...) {}
  best::row<Args...> row;

  friend void BestFmt(auto& fmt, const err& err)
    requires requires { fmt.format(err.row); }
  {
    fmt.write("err");
    fmt.format(err.row);
  }
  friend constexpr void BestFmtQuery(auto& query, err*) {
    query = query.template of<best::row<Args...>>;
  }
};
template <typename... Args>
err(Args&&...) -> err<Args&&...>;

/// # `best::is_result`
///
/// Whether `T` is some `best::result<U, E>`.
template <typename T>
concept is_result =
    best::same<best::as_auto<T>,
               best::result<typename best::as_auto<T>::ok_type,
                            typename best::as_auto<T>::err_type>>;

/// # `best::ok_type<T>`
///
/// Given `best::result<U, E>`, returns `U`.
template <is_result T>
using ok_type = typename best::as_auto<T>::ok_type;

/// # `best::err_type<T>`
///
/// Given `best::result<U, E>`, returns `E`.
template <is_result T>
using err_type = typename best::as_auto<T>::err_type;

template <typename T, typename E>
class [[nodiscard(
    "best::results may contain an error and must be explicitly "
    "handled!")]] result final {
 private:
  template <typename U>
  static constexpr bool cannot_init_from =
      ((!best::constructible<T, const U&> && !best::constructible<T, U&&>) ||
       best::is_void<T>)&&((!best::constructible<E, const U&> &&
                            !best::constructible<E, U&&>) ||
                           best::is_void<E>);

 public:
  /// Helper type aliases.
  using ok_type = T;
  using ok_value = best::as_auto<T>;
  using ok_cref = best::as_ref<const ok_type>;
  using ok_ref = best::as_ref<ok_type>;
  using ok_crref = best::as_rref<const ok_type>;
  using ok_rref = best::as_rref<ok_type>;
  using ok_cptr = best::as_ptr<const ok_type>;
  using ok_ptr = best::as_ptr<ok_type>;

  using err_type = E;
  using err_value = best::as_auto<E>;
  using err_cref = best::as_ref<const err_type>;
  using err_ref = best::as_ref<err_type>;
  using err_crref = best::as_rref<const err_type>;
  using err_rref = best::as_rref<err_type>;
  using err_cptr = best::as_ptr<const err_type>;
  using err_ptr = best::as_ptr<err_type>;

  /// # `result::result()`
  ///
  /// Results are not default-constructible.
  constexpr result() = delete;

  /// # `result::result(result)`
  ///
  /// Copy/move are trivial if they are trivial for `T` and `E`.
  constexpr result(const result&) = default;
  constexpr result& operator=(const result&) = default;
  constexpr result(result&&) = default;
  constexpr result& operator=(result&&) = default;

  /// # `result::result(...)`
  ///
  /// This converts a value into a result, by selecting the unambiguous type it
  /// can convert into.
  constexpr result(auto&&... args)
    requires best::constructible<best::choice<T, E>, decltype(args)...> &&
             (sizeof...(args) > 0)
      : BEST_RESULT_IMPL_(BEST_FWD(args)...) {}

  /// # `result::result(ok(...))`
  ///
  /// Constructs an ok result from a `best::ok(...)`.
  template <typename... Args>
  constexpr result(best::ok<Args...> args)
    requires best::constructible<T, Args...>
      : BEST_RESULT_IMPL_(best::index<0>, std::move(args).row.forward()) {}

  /// # `result::result(err(...))`
  ///
  /// Constructs an error result from a `best::error(...)`.
  template <typename... Args>
  constexpr result(best::err<Args...> args)
    requires best::constructible<E, Args...>
      : BEST_RESULT_IMPL_(best::index<1>, std::move(args).row.forward()) {}

  /// # `result::result(result<U, F>&)`
  ///
  /// Constructs a `best::result<T, E>` by converting from a reference to
  /// `best::result<U, F>` by copy or move. In particular, this enables
  ///
  /// ```
  /// best::result<int, error> x = 42;
  /// best::result<int&, error&> y = x;
  /// ```
  ///
  /// Conditionally explicit: only available when `T` is implicitly convertible
  /// from `U`.
  template <is_result R>
  constexpr explicit(
      !best::convertible<T, best::refcopy<best::ok_type<R>, R&&>> ||
      !best::convertible<E, best::refcopy<best::err_type<R>, R&&>>)
      result(R&& that)
    requires best::constructible<T, best::refcopy<best::ok_type<R>, R&&>> &&
             best::constructible<E, best::refcopy<best::err_type<R>, R&&>> &&
             cannot_init_from<R>;
  template <is_result R>
  constexpr result& operator=(R&& that)
    requires best::constructible<T, best::refcopy<best::ok_type<R>, R&&>> &&
             best::constructible<E, best::refcopy<best::err_type<R>, R&&>> &&
             cannot_init_from<R>;

  /// # `result::ok()`
  ///
  /// Extracts the ok value of this result, if present, and returns it in an
  /// optional.
  constexpr best::option<ok_cref> ok() const&;
  constexpr best::option<ok_ref> ok() &;
  constexpr best::option<ok_crref> ok() const&&;
  constexpr best::option<ok_rref> ok() &&;
  constexpr explicit operator bool() const { return impl().which() == 0; }

  /// # `result::ok()`
  ///
  /// Extracts the ok value of this result, if present, and returns it in an
  /// optional.
  constexpr best::option<err_cref> err() const&;
  constexpr best::option<err_ref> err() &;
  constexpr best::option<err_crref> err() const&&;
  constexpr best::option<err_rref> err() &&;

  /// # `result::operator*, result::operator->`
  ///
  /// `best::result`'s ok contents can be accessed with the smart pointer
  /// operators. These internally simply defer to `*ok()`.
  ///
  // TODO: nicer formatting once we have BestFmt.
  constexpr ok_cref operator*() const& { return impl()[best::index<0>]; }
  constexpr ok_ref operator*() & { return impl()[best::index<0>]; }
  constexpr ok_crref operator*() const&& {
    return BEST_MOVE(*this).impl()[best::index<0>];
  }
  constexpr ok_rref operator*() && {
    return BEST_MOVE(*this).impl()[best::index<0>];
  }
  constexpr ok_cptr operator->() const {
    return *ok(), impl().as_ptr(index<0>);
  }
  constexpr ok_ptr operator->() { return *ok(), impl().as_ptr(index<0>); }

  /// # `option::map()`, `option::map_err()`
  ///
  /// Applies a function to the contents of this result, and returns a new
  /// option by applying the given callback to either the ok or error
  /// alternative.
  constexpr auto map(auto&& f) const&;
  constexpr auto map(auto&& f) &;
  constexpr auto map(auto&& f) const&&;
  constexpr auto map(auto&& f) &&;
  constexpr auto map_err(auto&& f) const&;
  constexpr auto map_err(auto&& f) &;
  constexpr auto map_err(auto&& f) const&&;
  constexpr auto map_err(auto&& f) &&;

  /// # `option::then()`
  ///
  /// Returns the error alternative if `this->err()`, else calls `f` with this
  /// option's contents and returns the result.
  ///
  /// Some languages call this operation `flatmap`, `>>=`, or `and_then`.
  constexpr auto then(auto&& f) const&;
  constexpr auto then(auto&& f) &;
  constexpr auto then(auto&& f) const&&;
  constexpr auto then(auto&& f) &&;

  // Comparisons.
  template <best::equatable<T> U, best::equatable<E> F>
  BEST_INLINE_SYNTHETIC constexpr bool operator==(
      const best::result<U, F>& that) const {
    return impl() == that.impl();
  }
  BEST_INLINE_SYNTHETIC constexpr bool operator==(
      const best::equatable<T> auto& u) const {
    return ok() == u;
  }
  BEST_INLINE_SYNTHETIC constexpr bool operator==(
      const best::equatable<E> auto& u) const {
    return err() == u;
  }
  template <best::equatable<T> U>
  BEST_INLINE_SYNTHETIC constexpr bool operator==(best::ok<U> u) const {
    return ok() == u.row[best::index<0>];
  }
  template <best::equatable<E> U>
  BEST_INLINE_SYNTHETIC constexpr bool operator==(best::err<U> u) const {
    return err() == u.row[best::index<0>];
  }

  BEST_INLINE_SYNTHETIC constexpr bool operator==(best::ok<> u) const {
    return ok().has_value();
  }
  BEST_INLINE_SYNTHETIC constexpr bool operator==(best::err<> u) const {
    return err().has_value();
  }

  template <best::comparable<T> U, best::comparable<E> F>
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(
      const best::result<U, F>& that) const {
    return impl() <=> that.impl();
  }
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(
      const best::comparable<T> auto& u) const {
    if (auto v = ok()) return v <=> u;
    return best::ord::less;
  }
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(
      const best::comparable<E> auto& u) const {
    if (auto v = err()) return v <=> u;
    return best::ord::greater;
  }
  template <best::comparable<T> U>
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(best::ok<U> u) const {
    if (auto v = ok()) return v <=> u.row[best::index<0>];
    return best::ord::less;
  }
  template <best::comparable<E> U>
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(best::err<U> u) const {
    if (auto v = ok()) return v <=> u.rpw[best::index<0>];
    return best::ord::less;
  }
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(best::ok<> u) const {
    if (ok()) return best::ord::equal;
    return best::ord::less;
  }
  BEST_INLINE_SYNTHETIC constexpr auto operator<=>(best::err<> u) const {
    if (err()) return best::ord::equal;
    return best::ord::greater;
  }

  friend void BestFmt(auto& fmt, const result& res)
    requires requires { fmt.format(res.ok()); } &&
             requires { fmt.format(res.err()); }
  {
    if (auto v = res.ok()) {
      fmt.format("ok({:!})", *v.as_object());
    } else if (auto v = res.err()) {
      fmt.format("err({:!})", *v.as_object());
    }
  }
  template <typename Q>
  friend constexpr void BestFmtQuery(Q& query, result*) {
    query.supports_width = query.template of<T>.supports_width ||
                           query.template of<E>.supports_width;
    query.supports_prec = query.template of<T>.supports_prec ||
                          query.template of<E>.supports_prec;
    query.uses_method = [](auto r) {
      return (Q::template of<T>.uses_method(r) &&
              Q::template of<E>.uses_method(r));
    };
  }

 private:
  template <typename, typename>
  friend class result;

  constexpr const auto& impl() const& { return BEST_RESULT_IMPL_; }
  constexpr auto& impl() & { return BEST_RESULT_IMPL_; }
  constexpr const auto&& impl() const&& { return BEST_MOVE(BEST_RESULT_IMPL_); }
  constexpr auto&& impl() && { return BEST_MOVE(BEST_RESULT_IMPL_); }

 public:
  best::choice<T, E> BEST_RESULT_IMPL_;
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T, typename E>
template <is_result R>
constexpr result<T, E>::result(R&& that)
  requires best::constructible<T, best::refcopy<best::ok_type<R>, R&&>> &&
           best::constructible<E, best::refcopy<best::err_type<R>, R&&>> &&
           cannot_init_from<R>
    : BEST_RESULT_IMPL_(best::uninit) {
  if (auto v = BEST_FWD(that).ok()) {
    if constexpr (best::is_void<best::ok_type<R>>) {
      std::construct_at(&impl(), best::index<0>);
    } else {
      std::construct_at(&impl(), best::index<0>, *std::move(v));
    }
  } else if (auto v = BEST_FWD(that).err()) {
    if constexpr (best::is_void<best::err_type<R>>) {
      std::construct_at(&impl(), best::index<1>);
    } else {
      std::construct_at(&impl(), best::index<1>, *std::move(v));
    }
  }
}

// This effectively makes the above field private by making it impossible to
// name.
#define BEST_RESULT_IMPL_ private_

template <typename T, typename E>
template <is_result R>
constexpr result<T, E>& result<T, E>::operator=(R&& that)
  requires best::constructible<T, best::refcopy<best::ok_type<R>, R&&>> &&
           best::constructible<E, best::refcopy<best::err_type<R>, R&&>> &&
           cannot_init_from<R>
{
  if (auto v = BEST_FWD(that).ok()) {
    if constexpr (best::is_void<best::ok_type<R>>) {
      *this = best::ok();
    } else {
      *this = best::ok(*std::move(v));
    }
  } else if (auto v = BEST_FWD(that).err()) {
    if constexpr (best::is_void<best::err_type<R>>) {
      *this = best::err();
    } else {
      *this = best::err(*std::move(v));
    }
  }
  return *this;
}

template <typename T, typename E>
constexpr auto result<T, E>::ok() const& -> option<ok_cref> {
  return impl().at(best::index<0>);
}
template <typename T, typename E>
constexpr auto result<T, E>::ok() & -> option<ok_ref> {
  return impl().at(best::index<0>);
}
template <typename T, typename E>
constexpr auto result<T, E>::ok() const&& -> option<ok_crref> {
  return BEST_MOVE(*this).impl().at(best::index<0>);
}
template <typename T, typename E>
constexpr auto result<T, E>::ok() && -> option<ok_rref> {
  return BEST_MOVE(*this).impl().at(best::index<0>);
}

template <typename T, typename E>
constexpr auto result<T, E>::err() const& -> option<err_cref> {
  return impl().at(best::index<1>);
}
template <typename T, typename E>
constexpr auto result<T, E>::err() & -> option<err_ref> {
  return impl().at(best::index<1>);
}
template <typename T, typename E>
constexpr auto result<T, E>::err() const&& -> option<err_crref> {
  return BEST_MOVE(*this).impl().at(best::index<1>);
}
template <typename T, typename E>
constexpr auto result<T, E>::err() && -> option<err_rref> {
  return BEST_MOVE(*this).impl().at(best::index<1>);
}

template <typename T, typename E>
constexpr auto result<T, E>::map(auto&& f) const& {
  using U = best::call_result<decltype(f), ok_cref>;
  return impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> result<U, E> {
        return best::ok(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<1>, auto&&... args) -> result<U, E> {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map(auto&& f) & {
  using U = best::call_result<decltype(f), ok_ref>;
  return impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> result<U, E> {
        return best::ok(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<1>, auto&&... args) -> result<U, E> {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map(auto&& f) const&& {
  using U = best::call_result<decltype(f), ok_crref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> result<U, E> {
        return best::ok(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<1>, auto&&... args) -> result<U, E> {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map(auto&& f) && {
  using U = best::call_result<decltype(f), ok_rref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> result<U, E> {
        return best::ok(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<1>, auto&&... args) -> result<U, E> {
        return best::err(BEST_FWD(args)...);
      });
}

template <typename T, typename E>
constexpr auto result<T, E>::map_err(auto&& f) const& {
  using U = best::call_result<decltype(f), err_cref>;
  return impl().index_match(
      [&](best::index_t<1>, auto&&... args) -> result<T, U> {
        return best::err(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<0>, auto&&... args) -> result<T, U> {
        return best::ok(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map_err(auto&& f) & {
  using U = best::call_result<decltype(f), err_ref>;
  return impl().index_match(
      [&](best::index_t<1>, auto&&... args) -> result<T, U> {
        return best::err(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<0>, auto&&... args) -> result<T, U> {
        return best::ok(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map_err(auto&& f) const&& {
  using U = best::call_result<decltype(f), err_crref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<1>, auto&&... args) -> result<T, U> {
        return best::err(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<0>, auto&&... args) -> result<T, U> {
        return best::ok(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::map_err(auto&& f) && {
  using U = best::call_result<decltype(f), err_rref>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<1>, auto&&... args) -> result<T, U> {
        return best::err(best::call(BEST_FWD(f), BEST_FWD(args)...));
      },
      [&](best::index_t<0>, auto&&... args) -> result<T, U> {
        return best::ok(BEST_FWD(args)...);
      });
}

template <typename T, typename E>
constexpr auto result<T, E>::then(auto&& f) const& {
  using U = best::unref<best::call_result<decltype(f), ok_cref>>;
  return impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::then(auto&& f) & {
  using U = best::unref<best::call_result<decltype(f), ok_ref>>;
  return impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::then(auto&& f) const&& {
  using U = best::unref<best::call_result<decltype(f), ok_crref>>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::err(BEST_FWD(args)...);
      });
}
template <typename T, typename E>
constexpr auto result<T, E>::then(auto&& f) && {
  using U = best::unref<best::call_result<decltype(f), ok_rref>>;
  return BEST_MOVE(*this).impl().index_match(
      [&](best::index_t<0>, auto&&... args) -> U {
        return best::call(BEST_FWD(f), BEST_FWD(args)...);
      },
      [&](best::index_t<1>, auto&&... args) -> U {
        return best::err(BEST_FWD(args)...);
      });
}

constexpr decltype(auto) BestGuardResidual(auto&&, is_result auto&& r) {
  return *r.err();
}
constexpr auto BestGuardReturn(auto&&, is_result auto&&, auto&& e) {
  return best::err(BEST_FWD(e));
}
}  // namespace best
#endif  // BEST_CONTAINER_RESULT_H_
