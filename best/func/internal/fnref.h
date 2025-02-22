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

#ifndef BEST_FUNC_INTERNAL_FNREF_H_
#define BEST_FUNC_INTERNAL_FNREF_H_

#include <cstddef>

#include "best/base/unsafe.h"
#include "best/func/call.h"
#include "best/meta/traits/classes.h"
#include "best/meta/traits/funcs.h"
#include "best/meta/traits/ptrs.h"
#include "best/meta/traits/quals.h"
#include "best/meta/traits/refs.h"
#include "best/meta/traits/types.h"

namespace best::fnref_internal {
template <typename F>
concept no_captures = requires(F f) {
  requires best::is_class<F>;
  { +f } -> best::is_func_ptr;
};

template <typename P>
struct ptr_cast {
  template <typename T>
  constexpr operator T&() {
    return *static_cast<T*>(ptr);
  }
  P* ptr;
};

template <typename Func, typename T>
using copy_const =
  best::select<best::traits_internal::tame<Func>::c, best::as_const<T>, T>;

template <typename Func, typename R, typename... Args>
class impl {
 private:
  using ptr = best::select<best::is_const_func<Func>, const void*, void*>;

 public:
  using output = R;

  struct bound final {
   public:
    constexpr bound() = default;
    constexpr bound(std::nullptr_t){};

    constexpr bound(no_captures auto closure)
      : pc_(+[](ptr thiz, Args... args) {
          decltype(closure) closure;
          return closure(ptr_cast{.ptr = thiz}, BEST_FWD(args)...);
        }) {}

    constexpr bound(R (*fnptr)(ptr, Args...)) : pc_(fnptr) {}

    constexpr bool operator==(std::nullptr_t) const { return pc_ == nullptr; }
    constexpr explicit operator bool() const { return *this != nullptr; }

   private:
    friend impl;
    R (*pc_)(ptr, Args...) = nullptr;
  };

  using free = R (*)(Args...);

  constexpr impl() = delete;

  constexpr impl(best::unsafe, ptr ptr, bound fnptr)
    : ptr_(ptr), fnptr_{.bound = fnptr} {}

  constexpr impl(std::nullptr_t) : ptr_(nullptr), fnptr_{.free = nullptr} {}
  constexpr impl(R (*fn)(Args...)) : ptr_(nullptr), fnptr_{.free = fn} {}

  constexpr impl(const auto& fn)
    requires best::callable<decltype(fn), R(Args...)> &&
               (!no_captures<decltype(fn)>)
    : ptr_(best::addr(fn)),
      fnptr_{.bound = [](decltype(fn)& f, Args... args) -> R {
        if constexpr (best::is_void<R>) {
          best::call(f, BEST_FWD(args)...);
        } else {
          return best::call(f, BEST_FWD(args)...);
        }
      }} {}

  constexpr impl(best::callable<R(Args...)> auto&& fn)
    requires (!best::is_const_func<Func>) && (!no_captures<decltype(fn)>)
    : ptr_(best::addr(fn)),
      fnptr_{.bound = [](decltype(fn)& f, Args... args) -> R {
        if constexpr (best::is_void<R>) {
          best::call(f, BEST_FWD(args)...);
        } else {
          return best::call(f, BEST_FWD(args)...);
        }
      }} {}

  constexpr impl(best::callable<R(Args...)> auto&& fn)
    requires no_captures<decltype(fn)>
    : impl(+fn) {}

  constexpr R operator()(Args... args) const requires best::is_const_func<Func>
  {
    if (ptr_) {
      return fnptr_.bound.pc_(ptr_, BEST_FWD(args)...);
    } else {
      return fnptr_.free(BEST_FWD(args)...);
    }
  }

  constexpr R operator()(Args... args) {
    if (ptr_) {
      return fnptr_.bound.pc_(ptr_, BEST_FWD(args)...);
    } else {
      return fnptr_.free(BEST_FWD(args)...);
    }
  }

  constexpr bool operator==(std::nullptr_t) const {
    return ptr_ == nullptr && fnptr_.free == nullptr;
  }
  constexpr explicit operator bool() const { return *this != nullptr; }

 private:
  ptr ptr_;
  union {
    bound bound;
    free free;
  } fnptr_;
};
}  // namespace best::fnref_internal

#endif  // BEST_FUNC_INTERNAL_FNREF_H_
