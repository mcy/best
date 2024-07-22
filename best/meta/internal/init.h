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

#ifndef BEST_META_INTERNAL_INIT_H_
#define BEST_META_INTERNAL_INIT_H_

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/meta/taxonomy.h"

//! Rule-based implementation of constructable/convertible/assignable, using
//! overload resolution as a reasonably fast lookup.
//!
//! Clang is usually able to build good lookup structures for doing overload
//! resolution fast, so this is going to be much faster than letting the
//! constexpr interpreter do it.

namespace best {
class trivially final {
 private:
  ~trivially();
};

namespace init_internal {
/// Helper type that is *not* tlist, since this is much cheaper to instantiate.
template <typename>
struct tag {};

// ========================================================================== //

template <best::is_object T, typename... Args>
void ctor(tag<T>, tag<Args>...) requires std::is_constructible_v<T, Args...>;

template <best::is_object T, typename... Args>
void ctor(tag<T>, tag<trivially>, tag<Args>...)
  requires std::is_trivially_constructible_v<T, Args...>;

template <best::is_object T, best::is_void V>
void ctor(tag<T>, tag<V>) requires std::is_constructible_v<T>;

template <best::is_object T, best::is_void V>
void ctor(tag<T>, tag<trivially>, tag<V>)
  requires std::is_trivially_constructible_v<T>;

// -------------------------------------------------------------------------- //

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<U[n]>) requires std::is_constructible_v<T, U>;

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<trivially>, tag<U[n]>)
  requires std::is_trivially_constructible_v<T, U>;

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<U (&)[n]>) requires std::is_constructible_v<T, U&>;

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<trivially>, tag<U (&)[n]>)
  requires std::is_trivially_constructible_v<T, U&>;

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<U (&&)[n]>) requires std::is_constructible_v<T, U&&>;

template <best::is_object T, best::is_object U, size_t n>
void ctor(tag<T[n]>, tag<trivially>, tag<U (&&)[n]>)
  requires std::is_trivially_constructible_v<T, U&&>;

// -------------------------------------------------------------------------- //

template <best::is_lref T, best::is_ref R>
void ctor(tag<T>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_lref T, best::is_ref R>
void ctor(tag<T>, tag<trivially>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_rref T, best::is_rref R>
void ctor(tag<T>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_rref T, best::is_rref R>
void ctor(tag<T>, tag<trivially>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

// -------------------------------------------------------------------------- //

template <best::is_func T, typename F>
void ctor(tag<T>, tag<F>) requires std::is_convertible_v<F, best::as_ref<T>>;

template <best::is_func T, typename F>
void ctor(tag<T>, tag<trivially>, tag<F>)
  requires std::is_convertible_v<F, best::as_ref<T>>;

// -------------------------------------------------------------------------- //

template <best::is_void T>
void ctor(tag<T>);

template <best::is_void T>
void ctor(tag<T>, tag<trivially>);

template <best::is_void T, typename U>
void ctor(tag<T>, tag<U>);

template <best::is_void T, typename U>
void ctor(tag<T>, tag<trivially>, tag<U>);

// -------------------------------------------------------------------------- //

template <typename T, typename... Args>
void ctor(tag<T>, tag<best::args<Args...>>)
  requires requires { ctor(tag<T>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<T>, tag<trivially>, tag<best::args<Args...>>)
  requires requires { ctor(tag<T>{}, tag<trivially>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<T>, tag<best::args<Args...>&&>)
  requires requires { ctor(tag<T>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<T>, tag<trivially>, tag<best::args<Args...>&&>)
  requires requires { ctor(tag<T>{}, tag<trivially>{}, tag<Args>{}...); };

// ========================================================================== //

template <best::is_object T, typename Arg>
void conv(tag<T>, tag<Arg>) requires std::is_convertible_v<Arg, T>;

template <best::is_object T, typename Arg>
void conv(tag<T>, tag<trivially>, tag<Arg>)
  requires std::is_convertible_v<Arg, T> &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<Arg>()); };

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<U[n]>) requires std::is_convertible_v<T, U>;

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<trivially>, tag<U[n]>)
  requires std::is_convertible_v<T, U> &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<U>()); };

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<U (&)[n]>) requires std::is_convertible_v<T, U&>;

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<trivially>, tag<U (&)[n]>)
  requires std::is_convertible_v<T, U> &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<U>()); };

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<U (&&)[n]>) requires std::is_convertible_v<T, U&&>;

template <best::is_object T, best::is_object U, size_t n>
void conv(tag<T[n]>, tag<trivially>, tag<U (&&)[n]>)
  requires std::is_convertible_v<T, U> &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<U>()); };

template <typename T, typename Arg>
void conv(tag<T>, tag<Arg>)
  requires (!best::is_object<T> && !best::is_rref<T>) &&
           requires { ctor(tag<T>{}, tag<Arg>()); };

template <typename T, typename Arg>
void conv(tag<T>, tag<trivially>, tag<Arg>)
  requires (!best::is_object<T> && !best::is_rref<T>) &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<Arg>()); };

// ========================================================================== //

template <best::is_object T, typename Arg>
void assign(tag<T>, tag<Arg>) requires std::is_assignable_v<T&, Arg>;

template <best::is_object T, typename Arg>
void assign(tag<T>, tag<trivially>, tag<Arg>)
  requires std::is_trivially_assignable_v<T&, Arg>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<U[n]>) requires std::is_assignable_v<T&, U>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<trivially>, tag<U[n]>)
  requires std::is_trivially_assignable_v<T&, U>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<U (&)[n]>) requires std::is_assignable_v<T&, U&>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<trivially>, tag<U (&)[n]>)
  requires std::is_trivially_assignable_v<T&, U&>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<U (&&)[n]>) requires std::is_assignable_v<T&, U&&>;

template <best::is_object T, best::is_object U, size_t n>
void assign(tag<T[n]>, tag<trivially>, tag<U (&&)[n]>)
  requires std::is_trivially_assignable_v<T&, U&&>;

// -------------------------------------------------------------------------- //

template <typename T, typename Arg>
void assign(tag<T>, tag<Arg>)
  requires (!best::is_object<T>) && requires { ctor(tag<T>{}, tag<Arg>{}); };

template <typename T, typename Arg>
void assign(tag<T>, tag<trivially>, tag<Arg>)
  requires (!best::is_object<T>) &&
           requires { ctor(tag<T>{}, tag<trivially>{}, tag<Arg>{}); };

// -------------------------------------------------------------------------- //

template <best::is_void T>
void assign(tag<T>);

template <best::is_void T>
void assign(tag<T>, tag<trivially>);

// -------------------------------------------------------------------------- //

template <typename T, typename... Args>
void assign(tag<T>, tag<best::args<Args...>>)
  requires requires { assign(tag<T>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<T>, tag<trivially>, tag<best::args<Args...>>)
  requires requires { assign(tag<T>{}, tag<trivially>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<T>, tag<best::args<Args...>&&>)
  requires requires { assign(tag<T>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<T>, tag<trivially>, tag<best::args<Args...>&&>)
  requires requires { assign(tag<T>{}, tag<trivially>{}, tag<Args>{}...); };

// ========================================================================== //

void triv();
void triv(tag<trivially>);
template <typename... Args>
concept only_trivial = requires { triv(tag<Args>{}...); };

void is_triv(tag<trivially>);
template <typename... Args>
concept is_trivial = requires { is_triv(tag<Args>{}...); };

template <typename T>
concept trivially_relocatable =
#if BEST_HAS_BUILTIN(__is_trivially_relocatable)
  __is_trivially_relocatable(T);
#else
  std::is_trivial_v<T>;
#endif

}  // namespace init_internal
}  // namespace best

#endif  // BEST_META_INTERNAL_INIT_H_
