#ifndef BEST_META_INTERNAL_INIT_H_
#define BEST_META_INTERNAL_INIT_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/concepts.h"

//! Concepts for determining when a type can be initialized in a particular
//! way.

namespace best {
class trivially;

namespace init_internal {
template <typename... Args>
concept only_trivial = types<Args...> <= types<trivially>;

template <auto args>
concept is_void = args == types<void>;

template <typename T, bool trivial, typename... Args>
constexpr bool ctor_by_row(best::tlist<best::row_forward<Args...>>);
template <typename T, bool trivial>
constexpr bool ctor_by_row(auto&&) {
  return false;
}

template <typename T, bool trivial, auto args,
          typename _0 = decltype(args)::template type<0, void>,
          typename p0 = best::as_ptr<_0>>
concept ctor =
    ctor_by_row<T, trivial>(
        args.template map<std::remove_cvref_t>()) ||  // Recurse if this is a
                                                      // row_forward.
    (best::object_type<T> &&
     (trivial
          ? (args == types<void>
                 ? std::is_trivially_default_constructible_v<T>
                 : args.apply([]<typename... Args> {
                     return std::is_trivially_constructible_v<T, Args...>;
                   }))

          : (args == types<void> ? std::is_default_constructible_v<T>
                                 : args.template apply([]<typename... Args> {
                                     return std::is_constructible_v<T, Args...>;
                                   })))) ||
    //
    (best::ref_type<T> && args.size() == 1 &&
     // References can only be constructed from references.
     best::ref_type<_0> &&
     // Rvalue references cannot be constructed from lvalue references.
     !(best::ref_type<T, ref_kind::Rvalue> &&
       best::ref_type<_0, ref_kind::Lvalue>)&&args.size() == 1 &&
     std::is_convertible_v<p0, best::as_ptr<T>>) ||
    //
    (best::abominable_func_type<T> && args.size() == 1 &&
     std::is_same_v<p0, std::add_pointer_t<T>>) ||
    //
    (best::void_type<T> && args.size() <= 1);

template <typename T, bool trivial, typename... Args>
constexpr bool ctor_by_row(best::tlist<best::row_forward<Args...>>) {
  return ctor<T, trivial, best::types<Args...>>;
}

template <typename T, bool trivial, typename... Args>
constexpr bool assign_by_row(best::tlist<best::row_forward<Args...>>);
template <typename T, bool trivial>
constexpr bool assign_by_row(auto) {
  return false;
}

template <typename T, bool trivial, auto args,
          typename _0 = decltype(args)::template type<0, void>>
concept assign =
    assign_by_row<T, trivial>(
        args.template map<std::remove_cvref_t>()) ||  // Recurse if this is a
                                                      // row_forward.
    ((best::object_type<T>
          ? args.size() == 1 &&
                (trivial ? std::is_trivially_assignable_v<best::as_ref<T>, _0>
                         : std::is_assignable_v<best::as_ref<T>, _0>)
          : ctor<T, trivial, args, _0>));

template <typename T, bool trivial, typename... Args>
constexpr bool assign_by_row(best::tlist<best::row_forward<Args...>>) {
  return assign<T, trivial, best::types<Args...>>;
}

template <typename T>
concept trivially_relocatable =
#if __has_builtin(__is_trivially_relocatable)
    __is_trivially_relocatable(T);
#else
    std::is_trivial_v<T>;
#endif

}  // namespace init_internal
}  // namespace best

#endif  // BEST_META_INTERNAL_INIT_H_