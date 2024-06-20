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

template <typename Class, best::is_func F>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<>, F Class::*member,
                                                    auto&& self, auto&&... args)
  requires requires { (self.*member)(BEST_FWD(args)...); }
{
  return (BEST_FWD(self).*member)(BEST_FWD(args)...);
}
template <typename Class, best::is_func F>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<>, F Class::*member,
                                                    auto* self, auto&&... args)
  requires requires { (self->*member)(BEST_FWD(args)...); }
{
  return (self->*member)(BEST_FWD(args)...);
}

template <typename Class, best::is_object R>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<>, R Class::*member,
                                                    auto&& self)
  requires(!best::is_func<R>) && requires { self.*member; }
{
  return BEST_FWD(self).*member;
}
template <typename Class, typename R>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(tag<>, R Class::*member,
                                                    auto* self)
  requires(!best::is_func<R>) && requires { self->*member; }
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
BEST_INLINE_SYNTHETIC constexpr auto call(tag<Args...>, auto&& func,
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