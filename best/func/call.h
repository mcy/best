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

#ifndef BEST_FUNC_CALL_H_
#define BEST_FUNC_CALL_H_

#include <stddef.h>

#include "best/base/fwd.h"
#include "best/base/hint.h"
#include "best/func/internal/call.h"
#include "best/meta/empty.h"
#include "best/meta/taxonomy.h"

//! Highly generic function-calling functions.

namespace best {
/// # `best::call()`
///
/// Calls a function.
///
/// This is a highly generic operation: it emulates the behavior of std::invoke,
/// which treats T C::* as a C -> T function; it also allows passing a
/// best::tlist as the first argument, which will pass explicit template
/// parameters to the underlying operator().
///
/// Additionally, any type parameters passed to this function will be forwarded
/// to `call`.
template <typename... TParams>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call(auto &&...args)
  requires requires {
    call_internal::call(call_internal::tag<TParams...>{}, BEST_FWD(args)...);
  }
{
  return call_internal::call(call_internal::tag<TParams...>{},
                             BEST_FWD(args)...);
}

/// # `best::call_devoid()`
///
/// `best::call`s a function, and replaces a `void` return with a `best::empty`.
template <typename... TParams>
BEST_INLINE_SYNTHETIC constexpr decltype(auto) call_devoid(auto &&...args)
  requires requires {
    call_internal::call(call_internal::tag<TParams...>{}, BEST_FWD(args)...);
  }
{
  using Out = decltype(call_internal::call(call_internal::tag<TParams...>{},
                                           BEST_FWD(args)...));
  if constexpr (best::is_void<Out>) {
    call_internal::call(call_internal::tag<TParams...>{}, BEST_FWD(args)...);
    return best::empty{};
  } else {
    return call_internal::call(call_internal::tag<TParams...>{},
                               BEST_FWD(args)...);
  }
}

/// # `BEST_CALLABLE()`
///
/// Constructs a generic lambda that will call the function named by
/// `path`.
///
/// This is useful for capturing a "function pointer" to an overloaded
/// function.
#define BEST_CALLABLE(path_) \
  [](auto &&...args) -> decltype(auto) { return path_(BEST_FWD(args)...); }

/// # `best::callable`
///
/// Whether `F` is a callable function with the requested signature.
///
/// Use as `best::callable<F, int(char*, size_t, size_t)>`. This will validate
/// that `F` can be called with the arguments `char*, size_t, size_t`, and that
/// the result is convertible to `int`. If the result type is specified to be
/// `void`, no constraints are placed on F's result type.
///
/// Arguments after the function signature are explicit template parameters for
/// `operator()`.
template <typename F, typename Signature, typename... TParams>
concept callable = call_internal::can_call<F>(call_internal::tag<TParams...>{},
                                              (Signature *)nullptr);

/// # `best::call_result`.
///
/// Returns the result of calling F with no template parameters and some
/// arguments.
///
/// If `Args` is exactly one void type, this instead simulates calling with zero
/// arguments.
template <typename F, typename... Args>
using call_result =
    decltype(call_internal::call_result<F>(call_internal::tag<Args...>{}));

}  // namespace best

#endif  // BEST_FUNC_CALL_H_
