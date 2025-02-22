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

#ifndef BEST_FUNC_INTERNAL_FNREF2_H_
#define BEST_FUNC_INTERNAL_FNREF2_H_

#include <cstddef>

#include "best/func/call.h"
#include "best/func/dyn.h"
#include "best/meta/traits/funcs.h"
#include "best/meta/traits/refs.h"

namespace best::fn_internal {
template <typename Func, typename R, typename... Args>
class fn {
 public:
  constexpr R operator()(Args... args) const {
    return (*vt_)->call(data_, BEST_FWD(args)...);
  }
  constexpr R operator()(Args... args) requires (!best::is_const_func<Func>)
  {
    return (*vt_)->call(data_, BEST_FWD(args)...);
  }

  template <best::as_dyn<fn> F>
  static constexpr auto of(F&& BEST_value_) {
    return best::dyn<F>::of(BEST_FWD(BEST_value_));
  }

  struct BestFuncs final {
    best::vtable_binder<Func> call;
  };

  const best::vtable<fn>& vtable() const { return vt_; }

 private:
  friend best::access;

  template <typename F>
  static constexpr best::vtable<fn> lambda_impl{
    best::types<F>,
    {.call = [](best::select<best::is_const_func<Func>, const F, F>& f,
                Args... args) { return f(BEST_FWD(args)...); }}};

  template <best::callable<Func> F>
  constexpr friend const best::vtable<fn>& BestImplements(F*, fn*) {
    return lambda_impl<F>;
  }

  // static constexpr best::vtable<fn> fnptr_impl{
  //   best::types<best::tame<Func>>,
  //   {.call = [](best::tame<Func>* f, Args... args) {
  //     return f(BEST_FWD(args)...);
  //   }}};

  // constexpr friend const best::vtable<fn>& BestImplements(best::tame<Func>*,
  // fn*) {
  //   return fnptr_impl;
  // }

  constexpr fn(void* data, const best::vtable<fn>* vt) : data_(data), vt_(vt) {}

  void* data_;
  const best::vtable<fn>* vt_;
};
}  // namespace best::fn_internal

#endif  // BEST_FUNC_INTERNAL_FNREF_H_
