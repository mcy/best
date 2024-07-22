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

#include "best/func/call.h"
#include "best/meta/taxonomy.h"

namespace best::fnref_internal {
template <typename F>
concept no_captures = requires(F f) {
  { +f } -> best::is_func_ptr;
};

template <typename Func, typename R, typename... Args>
class impl {
 public:
  using output = R;

  constexpr impl() = delete;

  constexpr impl(std::nullptr_t) : ptr_(nullptr), fnptr_(nullptr) {}
  constexpr impl(R (*fn)(Args...)) : ptr_(nullptr), fnptr_(fn) {}

  constexpr impl(const auto& fn)
    requires best::callable<decltype(fn), R(Args...)> &&
               (!no_captures<decltype(fn)>)
    : ptr_(best::addr(fn)),
      lambda_(+[](const void* captures, Args... args) -> R {
        if constexpr (best::is_void<R>) {
          best::call(
            *reinterpret_cast<const best::unref<decltype(fn)>*>(captures),
            BEST_FWD(args)...);
        } else {
          return best::call(
            *reinterpret_cast<const best::unref<decltype(fn)>*>(captures),
            BEST_FWD(args)...);
        }
      }) {}

  constexpr impl(best::callable<R(Args...)> auto&& fn)
    requires (!best::is_const_func<Func>) && (!no_captures<decltype(fn)>)
    : ptr_(best::addr(fn)),
      lambda_(+[](const void* captures, Args... args) -> R {
        if constexpr (best::is_void<R>) {
          best::call(*reinterpret_cast<best::unref<decltype(fn)>*>(
                       const_cast<void*>(captures)),
                     BEST_FWD(args)...);
        } else {
          return best::call(*reinterpret_cast<best::unref<decltype(fn)>*>(
                              const_cast<void*>(captures)),
                            BEST_FWD(args)...);
        }
      }) {}

  constexpr impl(best::callable<R(Args...)> auto&& fn)
    requires no_captures<decltype(fn)>
    : impl(+fn) {}

  constexpr R operator()(Args... args) const {
    if (ptr_) {
      return lambda_(ptr_, BEST_FWD(args)...);
    } else {
      return fnptr_(BEST_FWD(args)...);
    }
  }

  constexpr bool operator==(std::nullptr_t) const {
    return ptr_ == nullptr && fnptr_ == nullptr;
  }
  constexpr explicit operator bool() const { return *this != nullptr; }

 private:
  const void* ptr_;
  union {
    R (*lambda_)(const void*, Args...);
    R (*fnptr_)(Args...);
  };
};
}  // namespace best::fnref_internal

#endif  // BEST_FUNC_INTERNAL_FNREF_H_
