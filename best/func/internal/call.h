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
#include "best/meta/taxonomy.h"
#include "best/meta/traits.h"

namespace best::call_internal {
template <typename...>
struct tag {};

template <typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
    tag<>, best::is_func auto Class::*member, auto&& self, auto&&... args)
  requires requires { (self.*member)(BEST_FWD(args)...); }
{
  return (BEST_FWD(self).*member)(BEST_FWD(args)...);
}
template <typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
    tag<>, best::is_func auto Class::*member, auto* self, auto&&... args)
  requires requires { (self->*member)(BEST_FWD(args)...); }
{
  return (self->*member)(BEST_FWD(args)...);
}

template <typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
    tag<>, best::is_object auto Class::*member, auto&& self)
  requires requires { self.*member; }
{
  return BEST_FWD(self).*member;
}
template <typename Class>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(
    tag<>, best::is_object auto Class::*member, auto* self)
  requires requires { self->*member; }
{
  return self->*member;
}

BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<>, auto&& func,
                                                    auto&&... args)
  requires requires { func(BEST_FWD(args)...); }
{
  return BEST_FWD(func)(BEST_FWD(args)...);
}
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<Args...>, auto&& func,
                                                    auto&&... args)
  requires requires { func.template operator()<Args...>(BEST_FWD(args)...); } &&
           (sizeof...(Args) > 0)
{
  return BEST_FWD(func).template operator()<Args...>(BEST_FWD(args)...);
}

BEST_INLINE_SYNTHETIC constexpr void call(tag<>) {}

template <typename F, typename... TParams, typename R, typename... Args>
constexpr bool can_call(tag<TParams...>, R (*)(Args...))
  requires requires(F f, Args... args) {
    call_internal::call<TParams...>(tag<TParams...>{}, f, BEST_FWD(args)...);
  }
{
  return best::is_void<R> ||
         best::convertible<R, decltype(call_internal::call<TParams...>(
                                  tag<TParams...>{}, best::lie<F>,
                                  best::lie<Args>...))>;
}

template <typename F, typename... Args>
auto call_result(tag<Args...>)
    -> decltype(call_internal::call(tag<>{}, best::lie<F>, best::lie<Args>...));
template <typename F, best::is_void V>
auto call_result(tag<V>)
    -> decltype(call_internal::call(tag<>{}, best::lie<F>));
}  // namespace best::call_internal

#endif  // BEST_FUNC_INTERNAL_CALL_H_
