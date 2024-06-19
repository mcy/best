#ifndef BEST_META_INIT_H_
#define BEST_META_INIT_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/bit_enum.h"
#include "best/meta/internal/init.h"

//! Concepts for determining when a type can be initialized in a particular
//! way.

namespace best {
/// Pass this to the first argument of any of the initialization concepts to
/// require that the initialization be trivial.
class trivially;

/// Whether T can be initialized by direct construction from the given
/// arguments.
///
/// For object types, this is simply std::is_constructible and
/// std::is_trivially_constructible, respectively. Note that these imply
/// the respective versions of std::is_destructible. If the argument list is
/// a single void type, it is treated as-if it was empty.
///
/// For reference types, this is where Arg& can convert to T&, i.e., if
/// Arg* can convert to T*.
///
/// For void types, this is either no arguments, or a single argument of
/// any type, including void.
template <typename T, typename... Args>
concept constructible =
    requires { init_internal::ctor<T>(init_internal::tag<Args>{}...); };

/// Same as constructible, but moreover requires for object types that
/// initialization be possible by implicit conversion.
///
/// T&& cannot be converted to from any type, even T&& itself. This is to avoid
/// potential issues with binding to temporaries.
template <typename T, typename... Args>
concept convertible =
    requires { init_internal::conv<T>(init_internal::tag<Args>{}...); };

template <typename From, typename To>
concept converts_to = convertible<To, From>;

/// Whether T is copy-constructible.
template <typename T, typename... Args>
concept move_constructible =
    init_internal::only_trivial<Args...> &&  //
    best::convertible<T, Args..., T> &&
    best::convertible<T, Args..., std::add_rvalue_reference_t<T>>;

/// Whether T is move-constructible.
template <typename T, typename... Args>
concept copy_constructible =
    init_internal::only_trivial<Args...> &&  //
    best::move_constructible<T, Args...> &&
    best::convertible<T, Args..., std::add_const_t<T>> &&
    best::convertible<T, Args...,
                      std::add_lvalue_reference_t<std::add_const_t<T>>>;

/// Whether T can be initialized by direct assignment from the given
/// arguments.
///
/// For object types, this is simply std::is_assignable and
/// std::is_trivially_assignable, respectively.
///
/// For all other types, this is sizeof...(Args) <= 1 &&
/// base::constructible.
template <typename T, typename... Args>
concept assignable =
    requires { init_internal::assign<T>(init_internal::tag<Args>{}...); };

/// Whether T is copy-assignable.
template <typename T, typename... Args>
concept move_assignable =
    init_internal::only_trivial<Args...> &&  //
    best::assignable<T, Args..., T> &&
    best::assignable<T, Args..., std::add_rvalue_reference_t<T>>;

/// Whether T is move-assignable.
template <typename T, typename... Args>
concept copy_assignable =
    init_internal::only_trivial<Args...> &&  //
    best::move_assignable<T, Args...> &&
    best::assignable<T, Args..., std::add_const_t<T>> &&
    best::assignable<T, Args...,
                     std::add_lvalue_reference_t<std::add_const_t<T>>>;

/// Whether T is moveable.
template <typename T, typename... Args>
concept moveable =
    init_internal::only_trivial<Args...> &&  //
    best::move_constructible<T, Args...> && best::move_assignable<T, Args...>;

/// Whether T is copyable.
template <typename T, typename... Args>
concept copyable =
    init_internal::only_trivial<Args...> &&  //
    best::copy_constructible<T, Args...> && best::copy_assignable<T, Args...>;

/// Whether T can be relocated.
///
/// Equivalent to best::movable, unless trivially is true, in which case
/// this further requires that the type be trivial, but it may hold
/// for some other types, such as those annotated with BEST_RELOCATABLE.
template <typename T, typename... Args>
concept relocatable = init_internal::only_trivial<Args...> &&  //
                      (!std::is_object_v<T> ||                 //
                       (init_internal::is_trivial<Args...>
                            ? init_internal::trivially_relocatable<T>
                            : moveable<T>));

/// Whether T can be destroyed.
///
/// For object types, this is simply std::is_destructible and
/// std::is_trivially_destructible, respectively.
///
/// For all other types, this is true.
template <typename T, typename... Args>
concept destructible =
    init_internal::only_trivial<Args...> &&  //
    (!std::is_object_v<T> ||
     (init_internal::is_trivial<Args...> ? std::is_trivially_destructible_v<T>
                                         : std::is_destructible_v<T>));

/// Struct that groups most of the above information, along with some other
/// useful data, about a list of types.
struct init_info_t {
  bool can_default, trivial_default;
  bool can_copy, trivial_copy;
  bool can_move, trivial_move;
  bool can_dtor, trivial_dtor;
};

/// The init_info for a particular list of types.
template <typename... Ts>
inline constexpr init_info_t init_info = {
    .can_default = (best::constructible<Ts> && ...),
    .trivial_default = (best::constructible<Ts, trivially> && ...),

    .can_copy = (best::copyable<Ts> && ...),
    .trivial_copy = (best::copyable<Ts, trivially> && ...),

    .can_move = (best::moveable<Ts> && ...),
    .trivial_move = (best::moveable<Ts, trivially> && ...),

    .can_dtor = (best::destructible<Ts> && ...),
    .trivial_dtor = (best::destructible<Ts, trivially> && ...),
};

/// Options for `best::init_from`.
enum init_by {
  /// Initialize by explicit construction; this includes conversions.
  Construct = 1 << 0,
  /// Initialize by implicit conversion.
  Convert = 1 << 1,
  /// Initialize by direct assignment.
  Assign = 1 << 2,
};
BEST_BIT_ENUM(init_by);

/// Whether T can be initialized by `Args...` via the given initialization
/// method(s).
template <typename T, init_by opts, typename... Args>
concept init_from =
    (((opts & init_by::Construct) == 0) || best::constructible<T, Args...>)&&(
        ((opts & init_by::Convert) == 0) ||
        best::convertible<T, Args...>)&&(((opts & init_by::Assign) == 0) ||
                                         best::assignable<T, Args...>);

/// A callback that constructs a `T`.
///
/// Currently only available for object types.
template <best::is_object T>
inline constexpr auto ctor =
    [](auto&&... args) -> T { return T{BEST_FWD(args)...}; };

}  // namespace best

#endif  // BEST_META_INIT_H_