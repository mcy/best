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

template <typename T, bool trivial, auto args,
          typename _0 = decltype(args)::template type<0, void>,
          typename p0 = best::as_ptr<_0>>
concept ctor =
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

template <typename T, bool trivial, auto args,
          typename _0 = decltype(args)::template type<0, void>>
concept assign =
    ((best::object_type<T>
          ? args.size() == 1 &&
                (trivial ? std::is_trivially_assignable_v<best::as_ref<T>, _0>
                         : std::is_assignable_v<best::as_ref<T>, _0>)
          : ctor<T, trivial, args, _0>));

}  // namespace init_internal
}  // namespace best

#endif  // BEST_META_INTERNAL_INIT_H_