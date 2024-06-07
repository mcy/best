#ifndef BEST_META_INIT_H_
#define BEST_META_INIT_H_

#include <stddef.h>

#include <type_traits>

#include "best/meta/bit_enum.h"
#include "best/meta/internal/init.h"
#include "best/meta/tags.h"

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
    init_internal::ctor<T, (types<trivially> <= types<Args...>),
                        types<Args...>.template trim_prefix(types<trivially>)>;

/// Same as constructible, but moreover requires for object types that
/// initialization be possible by implicit conversion.
///
/// T&& cannot be converted to from any type, even T&& itself. This is to avoid
/// potential issues with binding to temporaries.
template <typename T, typename... Args>
concept convertible =
    best::constructible<T, Args...> && !std::is_rvalue_reference_v<T> &&
    (!std::is_object_v<T> ||
     std::is_convertible_v<
         typename decltype(types<Args...>.template trim_prefix(
             types<trivially>))::template type<0, void>,
         T>);

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
    init_internal::assign<T, (types<trivially> <= types<Args...>),
                          types<Args...>.template trim_prefix(
                              types<trivially>)>;

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
concept relocatable =
    init_internal::only_trivial<Args...> &&  //
    (!std::is_object_v<T> ||                 //
     (types<trivially> <= types<Args...> ?
#if __has_builtin(__is_trivially_relocatable)
                                         __is_trivially_relocatable(T)
#else
                                         std::is_trivial_v<T>
#endif
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
     (types<trivially> <= types<Args...> ? std::is_trivially_destructible_v<T>
                                         : std::is_destructible_v<T>));

namespace init_internal {
template <typename T>
using filter_type = std::conditional_t<
    best::void_type<T>, best::empty,
    std::conditional_t<best::object_type<T>, T, best::as_ptr<T>>>;
}  // namespace init_internal

// Like alignof(), but works on multiple types; returns the maximum alignment.
//
// If the pack is empty, returns 1.
template <typename... Ts>
constexpr size_t align_of() {
  size_t align = 1;
  best::types<init_internal::filter_type<Ts>...>.each(
      [&]<typename T> { align = (align > alignof(T) ? align : alignof(T)); });
  return align;
}

// Like sizeof(), but works on multiple types; returns the size they would
// have if laid out in an aligned struct.
template <typename... Ts>
constexpr size_t size_of() {
  size_t size = 0, align = 1;

  auto align_to = [&size](size_t align) {
    auto remainder = size % align;
    if (remainder != 0) {
      size += align - remainder;
    }
  };

  best::types<init_internal::filter_type<Ts>...>.each([&]<typename T> {
    align_to(alignof(T));
    align = (align > alignof(T) ? align : alignof(T));
    size += sizeof(T);
  });

  align_to(align);
  return size;
}

// Like sizeof(), but works on multiple types; returns the alignment they would
// have if laid out in an aligned struct.
template <typename... Ts>
constexpr size_t size_of_union() {
  size_t size = 0, align = 1;
  auto align_to = [&](size_t align) {
    auto remainder = size % align;
    if (remainder != 0) {
      size += align - remainder;
    }
  };

  best::types<init_internal::filter_type<Ts>...>.each([&]<typename T> {
    align = (align > alignof(T) ? align : alignof(T));
    size = (size > sizeof(T) ? size : sizeof(T));
  });

  align_to(align);
  return size;
}

/// Struct that groups most of the above information, along with some other
/// useful data, about a list of types.
struct init_info_t {
  bool can_default, trivial_default;
  bool can_copy, trivial_copy;
  bool can_move, trivial_move;
  bool can_dtor, trivial_dtor;

  /// The size of a struct/union having the element types of members.
  /// void types are treated as empty structs, and references are treated
  /// as pointers.
  ///
  /// The alignment of both is always the same.
  size_t size, union_size;
  size_t align;
};

/// The inti_info for a particular list of types.
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

    .size = best::size_of<Ts...>(),
    .union_size = best::size_of_union<Ts...>(),
    .align = best::align_of<Ts...>(),
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
}  // namespace best

#endif  // BEST_META_INIT_H_