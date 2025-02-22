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

#ifndef BEST_FUNC_INTERNAL_CALL_H_
#define BEST_FUNC_INTERNAL_CALL_H_

#include <stddef.h>

#include "best/base/hint.h"
#include "best/meta/init.h"
#include "best/meta/traits/empty.h"
#include "best/meta/traits/funcs.h"
#include "best/meta/traits/objects.h"
#include "best/meta/traits/types.h"

namespace best::call_internal {
template <typename...>
struct tag {};

template <typename... Ts, typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
  best::is_func auto Class::*member, auto&& self, auto&&... args)
  requires requires {
    requires sizeof...(Ts) == 0;
    (self.*member)(BEST_FWD(args)...);
  }
{
  return (BEST_FWD(self).*member)(BEST_FWD(args)...);
}
template <typename... Ts, typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
  best::is_func auto Class::*member, auto* self, auto&&... args)
  requires requires {
    requires sizeof...(Ts) == 0;
    (self->*member)(BEST_FWD(args)...);
  }
{
  return (self->*member)(BEST_FWD(args)...);
}

template <typename... Ts, typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
  best::is_object auto Class::*member, auto&& self) requires requires {
  requires sizeof...(Ts) == 0;
  self.*member;
}
{
  return BEST_FWD(self).*member;
}
template <typename... Ts, typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
  best::is_object auto Class::*member, auto* self) requires requires {
  requires sizeof...(Ts) == 0;
  self->*member;
}
{
  return self->*member;
}

template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(auto&& func, auto&&... args)
  requires requires {
    requires sizeof...(Args) == 0;
    func(BEST_FWD(args)...);
  }
{
  return BEST_FWD(func)(BEST_FWD(args)...);
}
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(auto&& func, auto&&... args)
  requires requires {
    requires sizeof...(Args) > 0;
    func.template operator()<Args...>(BEST_FWD(args)...);
  }
{
  return BEST_FWD(func).template operator()<Args...>(BEST_FWD(args)...);
}

template <auto... targs>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(auto&& func, auto&&... args)
  requires requires {
    requires sizeof...(targs) > 0;
    func.template operator()<targs...>(BEST_FWD(args)...);
  }
{
  return BEST_FWD(func).template operator()<targs...>(BEST_FWD(args)...);
}

BEST_INLINE_SYNTHETIC constexpr void call() {}

template <typename F, typename... TParams, typename R, typename... Args>
constexpr bool can_call(tag<R(Args...), TParams...>)
  requires requires(F f, Args... args) {
    call_internal::call<TParams...>(f, BEST_FWD(args)...);
  }
{
  return best::is_void<R> ||
         best::convertible<R, decltype(call_internal::call<TParams...>(
                                best::lie<F>, best::lie<Args>...))>;
}

template <typename F, typename... TParams, typename R, typename... Args>
constexpr bool can_call(tag<R(Args...) const, TParams...>)
  requires requires(const F f, Args... args) {
    call_internal::call<TParams...>(f, BEST_FWD(args)...);
  }
{
  return best::is_void<R> ||
         best::convertible<R, decltype(call_internal::call<TParams...>(
                                best::lie<const F>, best::lie<Args>...))>;
}

template <typename F, typename... Args>
auto call_result(tag<Args...>)
  -> decltype(call_internal::call(best::lie<F>, best::lie<Args>...));
template <typename F, best::is_void V>
auto call_result(tag<V>) -> decltype(call_internal::call(best::lie<F>));
}  // namespace best::call_internal

#endif  // BEST_FUNC_INTERNAL_CALL_H_
