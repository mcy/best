#ifndef BEST_META_TRAITS_H_
#define BEST_META_TRAITS_H_

#include "best/meta/internal/traits.h"

//! Traits for performing miscellaneous metaprogramming tasks. Many of these
//! traits are implemented by `<type_traits>` but have horrendous names.

namespace best {
/// # `best::type_trait`
///
/// Whether `T` is a type trait, i.e., a type with an alias member named `type`.
template <typename T>
concept type_trait = requires { typename T::type; };

/// # `best::value_trait`
///
/// Whether `T` is a value trait, i.e., a type with a static data member named
/// `value`.
template <typename T>
concept value_trait = requires { T::value; };

/// # `best::dependent<...>
///
/// Makes a type dependent on template parameters.
///
/// There are sometimes cases where we want to force two-phase lookup[1] because
/// we are using e.g. an incomplete type in complete position in a template, and
/// the use is not dependent on the template parameters.
///
/// This always has the value of `T`, regardless of what `Deps` actually are.
///
/// [1]: https://en.cppreference.com/w/cpp/language/two-phase_lookup
template <typename T, typename... Deps>
using dependent = traits_internal::dependent<T, Deps...>::type;

/// # `best::select<...>`
///
/// Selects one of two types depending on a boolean condition.
template <bool cond, typename A, typename B>
using select = traits_internal::select<cond, A, B>::type;

/// # `best::select_trait<...>`
///
/// Selects one of two type traits depending on a boolean condition, and
/// extracts the result. This is useful for delaying materialization of an
/// illegal construction if it would not be selected. 
template <bool cond, best::type_trait A, type_trait B>
using select_trait = traits_internal::select<cond, A, B>::type::type;
}  // namespace best

#endif  // BEST_META_CONCEPTS_H_