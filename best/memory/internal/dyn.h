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

#ifndef BEST_MEMORY_INTERNAL_DYN_H_
#define BEST_MEMORY_INTERNAL_DYN_H_

#include <cstddef>

#include "best/base/access.h"
#include "best/func/fnref.h"

namespace best::dyn_internal {
using mark_as_used = best::fnref<void()>;

struct access {
  template <typename I>
  static constexpr bool can_wrap() {
    return requires(void* vp, const best::vtable<I>* vt) {
      { best::access::constructor<I>(vp, vt) };
    };
  }

  template <typename I>
  static constexpr I wrap(const void* vp, const best::vtable<I>* vt) {
    return best::access::constructor<I>(const_cast<void*>(vp), vt);
  }
};

template <typename F>
struct sig;
template <typename R, typename Arg>
struct sig<R(Arg)> {
  using type = Arg;
};

#define BEST_INTERFACE_(Interface_, ...)                                 \
 public:                                                                 \
  BEST_MAP(BEST_INTERFACE_DEF_MEMBER_, __VA_ARGS__)                      \
                                                                         \
  constexpr const auto& vtable() const { return *BEST_vt_; }             \
                                                                         \
  struct BestFuncs final {                                               \
    BEST_MAP(BEST_INTERFACE_DEF_FNPTR_, __VA_ARGS__)                     \
  };                                                                     \
                                                                         \
 private:                                                                \
  friend ::best::access;                                                 \
                                                                         \
  template <typename BEST_T_>                                            \
  static constexpr auto BEST_implements_ = ::best::vtable<Interface_>(   \
    ::best::types<BEST_T_>,                                              \
    typename ::best::vtable<Interface_>::funcs{                          \
      BEST_MAP(BEST_INTERFACE_GET_FNPTR_, __VA_ARGS__)});                \
                                                                         \
  template <typename BEST_T_>                                            \
  requires requires {                                                    \
    requires true; /* Empty requires{} is not allowed. */                \
    BEST_MAP(BEST_INTERFACE_REQUIREMENT_, __VA_ARGS__)                   \
  }                                                                      \
  constexpr friend const auto& BestImplements(BEST_T_*, Interface_*) {   \
    return BEST_implements_<BEST_T_>;                                    \
  }                                                                      \
                                                                         \
  constexpr Interface_(void* data, const ::best::vtable<Interface_>* vt) \
    : BEST_data_(data), BEST_vt_(vt) {}                                  \
                                                                         \
  void* BEST_data_;                                                      \
  const ::best::vtable<Interface_>* BEST_vt_  // Require a trailing comma.

#define BEST_INTERFACE_IF_CONST_(args, result) \
  BEST_INTERFACE_IF_CONST2_##args(result)
#define BEST_INTERFACE_IF_CONST2_(result)
#define BEST_INTERFACE_IF_CONST2_const(result) result

#define BEST_INTERFACE_DEF_MEMBER_(pack) BEST_INTERFACE_DEF_MEMBER2_ pack

#define BEST_INTERFACE_DEF_MEMBER2_(ret_, name_, args_, ...)          \
  constexpr typename ::best::id<ret_>::type name_(BEST_IMAP_JOIN_REC( \
    BEST_INTERFACE_DEF_PARAM2_, (, ), BEST_REMOVE_PARENS(args_)))     \
    __VA_ARGS__ {                                                     \
    return ::best::fnref<ret_ args_ BEST_INTERFACE_IF_CONST_(         \
      __VA_ARGS__, const)>(::best::unsafe(""), BEST_data_,            \
                           (*BEST_vt_)->name_)(BEST_IMAP_JOIN_REC(    \
      BEST_INTERFACE_REF_PARAM2_, (, ), BEST_REMOVE_PARENS(args_)));  \
  }

#define BEST_INTERFACE_DEF_FNPTR_(pack) BEST_INTERFACE_DEF_FNPTR2_ pack

#define BEST_INTERFACE_DEF_FNPTR2_(ret_, name_, args_, ...)   \
  typename ::best::fnref<ret_ args_ BEST_INTERFACE_IF_CONST_( \
    __VA_ARGS__, const)>::fnptr name_;

#define BEST_INTERFACE_GET_FNPTR_(pack) BEST_INTERFACE_GET_FNPTR2_ pack

#define BEST_INTERFACE_GET_FNPTR2_(ret_, name_, args_, ...)          \
  .name_ = [](BEST_INTERFACE_IF_CONST_(__VA_ARGS__, const)           \
                BEST_T_* BEST_this_ BEST_IMAP_REC(                   \
                  BEST_INTERFACE_DEF_PARAM_,                         \
                  BEST_REMOVE_PARENS(args_))) -> decltype(auto) {    \
    return (ret_)BEST_this_->name_(BEST_IMAP_JOIN_REC(               \
      BEST_INTERFACE_REF_PARAM2_, (, ), BEST_REMOVE_PARENS(args_))); \
  },

#define BEST_INTERFACE_REQUIREMENT_(pack) BEST_INTERFACE_REQUIREMENT2_ pack

#define BEST_INTERFACE_REQUIREMENT2_(ret_, name_, args_, ...)               \
  requires requires(                                                        \
    BEST_INTERFACE_IF_CONST_(__VA_ARGS__, const)                            \
      BEST_T_& BEST_this_ BEST_IMAP_REC(BEST_INTERFACE_DEF_PARAM_,          \
                                        BEST_REMOVE_PARENS(args_))) {       \
    {                                                                       \
      BEST_this_.name_(BEST_IMAP_JOIN_REC(BEST_INTERFACE_REF_PARAM2_, (, ), \
                                          BEST_REMOVE_PARENS(args_)))       \
    } -> ::best::converts_to<ret_>;                                         \
  };

#define BEST_INTERFACE_DEF_PARAM_(idx_, expr_) \
  , BEST_INTERFACE_DEF_PARAM2_(idx_, expr_)

#define BEST_INTERFACE_DEF_PARAM2_(idx_, expr_) \
  typename ::best::dyn_internal::sig<void(expr_)>::type _##idx_

#define BEST_INTERFACE_REF_PARAM_(idx_, expr_) \
  , BEST_INTERFACE_REF_PARAM2_(idx_, expr)
#define BEST_INTERFACE_REF_PARAM2_(idx_, expr_) BEST_FWD(_##idx_)

}  // namespace best::dyn_internal

#endif  // BEST_MEMORY_INTERNAL_DYN_H_
