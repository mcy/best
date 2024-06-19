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
void ctor(tag<Args>...)
  requires std::is_constructible_v<T, Args...>;

template <best::is_object T, typename... Args>
void ctor(tag<trivially>, tag<Args>...)
  requires std::is_trivially_constructible_v<T, Args...>;

template <best::is_object T, best::is_void V>
void ctor(tag<V>)
  requires std::is_constructible_v<T>;

template <best::is_object T, best::is_void V>
void ctor(tag<trivially>, tag<V>)
  requires std::is_trivially_constructible_v<T>;

// -------------------------------------------------------------------------- //

template <best::is_lref T, best::is_ref R>
void ctor(tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_lref T, best::is_ref R>
void ctor(tag<trivially>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_rref T, best::is_rref R>
void ctor(tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

template <best::is_rref T, best::is_rref R>
void ctor(tag<trivially>, tag<R>)
  requires std::is_convertible_v<best::as_ptr<R>, best::as_ptr<T>>;

// -------------------------------------------------------------------------- //

template <best::is_func T, typename F>
void ctor(tag<F>)
  requires std::is_convertible_v<F, best::as_ref<T>>;

template <best::is_func T, typename F>
void ctor(tag<trivially>, tag<F>)
  requires std::is_convertible_v<F, best::as_ref<T>>;

// -------------------------------------------------------------------------- //

template <best::is_void T>
void ctor();

template <best::is_void T>
void ctor(tag<trivially>);

template <best::is_void T, typename U>
void ctor(tag<U>);

template <best::is_void T, typename U>
void ctor(tag<trivially>, tag<U>);

// -------------------------------------------------------------------------- //

template <typename T, typename... Args>
void ctor(tag<best::row_forward<Args...>>)
  requires requires { ctor<T>(tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<trivially>, tag<best::row_forward<Args...>>)
  requires requires { ctor<T>(tag<trivially>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<best::row_forward<Args...>&&>)
  requires requires { ctor<T>(tag<Args>{}...); };

template <typename T, typename... Args>
void ctor(tag<trivially>, tag<best::row_forward<Args...>&&>)
  requires requires { ctor<T>(tag<trivially>{}, tag<Args>{}...); };

// ========================================================================== //

template <best::is_object T, typename Arg>
void conv(tag<Arg>)
  requires std::is_convertible_v<Arg, T>;

template <best::is_object T, typename Arg>
void conv(tag<trivially>, tag<Arg>)
  requires std::is_convertible_v<Arg, T> &&
           requires { ctor<T>(tag<trivially>{}, tag<Arg>()); };

template <typename T, typename Arg>
void conv(tag<Arg>)
  requires(!best::is_object<T> && !best::is_rref<T>) &&
          requires { ctor<T>(tag<Arg>()); };

template <typename T, typename Arg>
void conv(tag<trivially>, tag<Arg>)
  requires(!best::is_object<T> && !best::is_rref<T>) &&
          requires { ctor<T>(tag<trivially>{}, tag<Arg>()); };

// ========================================================================== //

template <best::is_object T, typename Arg>
void assign(tag<Arg>)
  requires std::is_assignable_v<T&, Arg>;

template <best::is_object T, typename Arg>
void assign(tag<trivially>, tag<Arg>)
  requires std::is_trivially_assignable_v<T&, Arg>;

// -------------------------------------------------------------------------- //

template <typename T, typename Arg>
void assign(tag<Arg>)
  requires(!best::is_object<T>) && requires { ctor<T>(tag<Arg>{}); };

template <typename T, typename Arg>
void assign(tag<trivially>, tag<Arg>)
  requires(!best::is_object<T>) &&
          requires { ctor<T>(tag<trivially>{}, tag<Arg>{}); };

// -------------------------------------------------------------------------- //

template <best::is_void T>
void assign();

template <best::is_void T>
void assign(tag<trivially>);

// -------------------------------------------------------------------------- //

template <typename T, typename... Args>
void assign(tag<best::row_forward<Args...>>)
  requires requires { assign<T>(tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<trivially>, tag<best::row_forward<Args...>>)
  requires requires { assign<T>(tag<trivially>{}, tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<best::row_forward<Args...>&&>)
  requires requires { assign<T>(tag<Args>{}...); };

template <typename T, typename... Args>
void assign(tag<trivially>, tag<best::row_forward<Args...>&&>)
  requires requires { assign<T>(tag<trivially>{}, tag<Args>{}...); };

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