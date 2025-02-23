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

#ifndef BEST_FUNC_INTERNAL_DYN_H_
#define BEST_FUNC_INTERNAL_DYN_H_

#include <cstddef>

#include "best/base/access.h"
#include "best/func/fnref.h"

namespace best::dyn_internal {
using mark_as_used = best::fnref<void()>;

struct access {
  template <typename I>
  static constexpr bool can_wrap() {
    return requires(void* vp, const I::BestVtable* vt) {
      { best::access::constructor<I>(vp, vt) };
    };
  }

  template <typename I>
  static constexpr I wrap(const void* vp, const I::BestVtable* vt) {
    return best::access::constructor<I>(const_cast<void*>(vp), vt);
  }
};

template <typename I, typename T>
constexpr auto apply_vtable_defaults(auto vt) {
  if constexpr (requires {
                  { I::template BestFuncDefaults<T>(vt) };
                }) {
    I::template BestFuncDefaults<T>(vt);
  }
  return vt;
}

// Used to convert a declarator like `int x` into just the type, `int`.
// This takes advantage of the fact that you can write variable names in the
// arguments of a function type, which are discarded and not actually part of
// the type.
template <typename F>
struct sig;
template <typename R, typename Arg>
struct sig<R(Arg)> {
  using type = Arg;
};

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

template <typename Func, typename R, typename... Args>
class binder_impl {
 private:
  using ptr = best::select<best::is_const_func<Func>, const void*, void*>;

 public:
  using fnptr = R (*)(ptr, Args...);

  constexpr binder_impl() = default;
  constexpr binder_impl(std::nullptr_t){};

  constexpr binder_impl(no_captures auto closure)
    : fnptr_(+[](ptr self, Args... args) {
        decltype(closure) closure;
        return closure(ptr_cast{.ptr = self}, BEST_FWD(args)...);
      }) {}

  constexpr binder_impl(R (*fnptr)(ptr, Args...)) : fnptr_(fnptr) {}

  constexpr bool operator==(std::nullptr_t) const { return fnptr_ == nullptr; }
  constexpr explicit operator bool() const { return *this != nullptr; }

  constexpr R operator()(ptr self, Args... args) const {
    return fnptr_(self, BEST_FWD(args)...);
  }

  constexpr operator fnptr() const { return fnptr_; }

 private:
  fnptr fnptr_ = nullptr;
};

// clang-format off
#define BEST_IFACE_(Interface_, ...)                                      \
 public:                                                                  \
  BEST_MAP(BEST_IFACE_MEM_, __VA_ARGS__)                                  \
                                                                          \
  struct BestVtable final {                                               \
    BEST_MAP(BEST_IFACE_FNPTR_, __VA_ARGS__)                              \
  };                                                                      \
                                                                          \
  template <typename BEST_T_, typename BEST_This_ = Interface_>           \
  static constexpr void BestFuncDefaults(BestVtable& BEST_vt_) {          \
    BEST_MAP(BEST_IFACE_DEFAULT_FNPTR_, __VA_ARGS__)                      \
  }                                                                       \
                                                                          \
 private:                                                                 \
  template <typename BEST_T_, typename BEST_This_>                        \
  static constexpr ::best::vtable<BEST_This_> BEST_implements_{           \
    ::best::types<BEST_T_>, [] {                                          \
    }(),                                                                  \
  };                                                                      \
                                                                          \
  template <typename BEST_T_, typename BEST_This_ = Interface_>           \
  requires requires {                                                     \
    requires true; /* Empty requires{} is not allowed. */                 \
    BEST_MAP(BEST_IFACE_REQ_, __VA_ARGS__)                                \
  }                                                                       \
  constexpr friend BestVtable BestImplements(BEST_T_*, Interface_*) {     \
    BestVtable BEST_vt_;                                                  \
    BEST_MAP(BEST_IFACE_GET_FNPTR_, __VA_ARGS__)                          \
    return BEST_vt_;                                                      \
  }                                                                       \
                                                                          \
  constexpr Interface_(void* data, const BestVtable* vt)                  \
    : BEST_data_(data), BEST_vt_(vt) {}                                   \
                                                                          \
  void* BEST_data_;                                                       \
  const BestVtable* BEST_vt_;                                             \
 public:                                                                  \
  friend ::best::access;                                                  \
  friend BestVtable                                                       \
    // Require a trailing semi.

#define BEST_IFACE_CONST_(args_) BEST_IFACE_CONST2_##args_
#define BEST_IFACE_CONST2_
#define BEST_IFACE_CONST2_const const

#define BEST_IFACE_MEM_(pack) BEST_IFACE_MEM_UNPACKED_ pack
#define BEST_IFACE_MEM_UNPACKED_(ret_, name_, args_, ...)                      \
  constexpr BEST_IFACE_RET_(ret_)                                              \
  name_(BEST_IFACE_PARAMS_(, args_))                                           \
  BEST_IFACE_CONST_(__VA_ARGS__)                                               \
  {                                                                            \
    return BEST_vt_->name_(BEST_IFACE_ARGS_(BEST_data_, args_));               \
  }

#define BEST_IFACE_RET_(ret_) \
  typename ::best::id<BEST_REMOVE_PARENS(ret_)>::type

#define BEST_IFACE_PARAMS_(prefix_, args_)                                     \
  prefix_ BEST_REMOVE_PARENS(                                                  \
    BEST_VA_OPT_REC(BEST_VA_OPT_REC((,), prefix_), BEST_REMOVE_PARENS(args_))) \
  BEST_IMAP_JOIN_REC(BEST_IFACE_PARAM_, (,), BEST_REMOVE_PARENS(args_))
#define BEST_IFACE_PARAM_(idx_, expr_) \
  typename ::best::dyn_internal::sig<void(expr_)>::type _##idx_

#define BEST_IFACE_ARGS_(prefix_, args_)                                       \
  prefix_ BEST_REMOVE_PARENS(                                                  \
    BEST_VA_OPT_REC(BEST_VA_OPT_REC((,), prefix_), BEST_REMOVE_PARENS(args_))) \
  BEST_IMAP_JOIN_REC(BEST_IFACE_ARG_, (,), BEST_REMOVE_PARENS(args_))
#define BEST_IFACE_ARG_(idx_, expr_)  BEST_FWD(_##idx_)

#define BEST_IFACE_FNPTR_(pack) BEST_IFACE_FNPTR_UNPACKED_ pack
#define BEST_IFACE_FNPTR_UNPACKED_(ret_, name_, args_, ...)                    \
  ::best::vtable_binder<BEST_IFACE_RET_(ret_) args_ BEST_IFACE_CONST_(__VA_ARGS__)> name_;
  
#define BEST_IFACE_DEFAULT_FNPTR_(pack) BEST_IFACE_DEFAULT_FNPTR_UNPACKED_ pack
#define BEST_IFACE_DEFAULT_FNPTR_UNPACKED_(ret_, name_, args_, ...)        \
  if constexpr(BEST_IFACE_DEFAULT_REQ_(ret_, name_, args_, __VA_ARGS__)) { \
    if (!BEST_vt_.name_) {                                              \
      BEST_vt_.name_ = [](BEST_IFACE_PARAMS_(                           \
                BEST_IFACE_CONST_(__VA_ARGS__) BEST_T_& BEST_this_,        \
                args_)) -> decltype(auto)                                  \
      {                                                                    \
        BEST_IFACE_CONST_(__VA_ARGS__) BEST_This_ BEST_dyn_(               \
          ::best::addr(BEST_this_),                                        \
          ::best::itable<BEST_This_>::of(::best::types<BEST_T_>).operator->()); \
        return BEST_dyn_.name_(                                            \
          BEST_IFACE_ARGS_(::best::defaulted{}, args_)                     \
        );                                                                 \
      };                                                                   \
    } \
  }

#define BEST_IFACE_GET_FNPTR_(pack) BEST_IFACE_GET_FNPTR_UNPACKED_ pack
#define BEST_IFACE_GET_FNPTR_UNPACKED_(ret_, name_, args_, ...)               \
  if constexpr (BEST_IFACE_PROVIDED_REQ_(ret_, name_, args_, __VA_ARGS__)) {  \
    BEST_vt_.name_ = [](BEST_IFACE_PARAMS_(                                \
                  BEST_IFACE_CONST_(__VA_ARGS__) BEST_T_& BEST_this_,         \
                  args_)) -> decltype(auto)                                   \
    {                                                                         \
      return (BEST_IFACE_RET_(ret_))                                          \
        BEST_this_.name_(BEST_IFACE_ARGS_(, args_));                          \
    };                                                                        \
  }


#define BEST_IFACE_REQ_(pack) BEST_IFACE_REQ_UNPACKED_ pack
#define BEST_IFACE_REQ_UNPACKED_(ret_, name_, args_, ...)           \
  requires BEST_IFACE_DEFAULT_REQ_(ret_, name_, args_, __VA_ARGS__) \
    || BEST_IFACE_PROVIDED_REQ_(ret_, name_, args_, __VA_ARGS__);

#define BEST_IFACE_PROVIDED_REQ_(ret_, name_, args_, ...)       \
  requires (BEST_IFACE_PARAMS_(                                 \
    BEST_IFACE_CONST_(__VA_ARGS__) BEST_T_& BEST_this_, args_)) \
  {                                                             \
    requires best::convertible<BEST_IFACE_RET_(ret_),           \
      decltype(BEST_this_.name_(BEST_IFACE_ARGS_(, args_)))>;   \
      /* Using { expr } -> concept syntax here crashes clang */ \
  }

#define BEST_IFACE_DEFAULT_REQ_(ret_, name_, args_, ...)                \
  requires (BEST_IFACE_PARAMS_(                                         \
    BEST_IFACE_CONST_(__VA_ARGS__) BEST_This_& BEST_this_, args_))      \
  {                                                                     \
    {                                                                   \
      BEST_this_.name_(BEST_IFACE_ARGS_(::best::defaulted{}, args_))    \
    };                                                                  \
  }
// clang-format on
}  // namespace best::dyn_internal

#endif  // BEST_FUNC_INTERNAL_DYN_H_
