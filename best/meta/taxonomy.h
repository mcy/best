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

#ifndef BEST_META_TAXONOMY_H_
#define BEST_META_TAXONOMY_H_

#include <concepts>
#include <type_traits>

#include "best/meta/internal/abominable.h"
#include "best/meta/internal/quals.h"

//! Concepts for identifying and destructuring basic C++ types.
//!
//! Other headers may define `is_*<T>` concepts for identifying `best`'s generic
//! container types.

namespace best {
/// # `BEST_FWD()`
///
/// Macro implementation of `std::forward`.
///
/// Intended to improve compile times and gdb debugging by eliminating an
/// extremely common function that must be inlined.
#define BEST_FWD(expr_) (static_cast<decltype(expr_)&&>(expr_))

/// # `BEST_MOVE()`
///
/// Macro implementation of `std::forward`.
///
/// Intended to improve compile times and gdb debugging by eliminating an
/// extremely common function that must be inlined.
#define BEST_MOVE(expr_) \
  (static_cast<::best::as_rref<::best::unref<decltype(expr_)>>>(expr_))

/// # `best::is_void`
///
/// A void type, i.e. cv-qualified void.
template <typename T>
concept is_void = std::is_void_v<T>;

/// # `best::is_const`
///
/// Whether this is a `const`-qualified object or void type.
template <typename T>
concept is_const = std::is_const_v<T>;

/// # `best::deconst`
///
/// Forcibly removes the `const` from a pointer type.
template <typename T>
constexpr T* deconst(const T* ptr) {
  return const_cast<T*>(ptr);
}

/// # `best::unqual`
///
/// If `T` is cv-qualified, removes the qualifiers.
template <typename T>
using unqual = std::remove_cv_t<T>;

/// # `best::qualcopy<Dst, Src>
///
/// Copies any top-level cv-qualification from `Src` onto `Dst`. If `Dst` is
/// not an object or void type, this does nothing.
template <typename Dst, typename Src>
using qualcopy = best::quals_internal::quals<Dst, Src>::copied;

/// # `best::qualifies_to<From, To>`
///
/// Returns whether you can build the type `To` by adding qualifiers to `From`.
template <typename From, typename To>
concept qualifies_to = std::is_same_v<From, To> ||           //
                       std::is_same_v<const From, To> ||     //
                       std::is_same_v<volatile From, To> ||  //
                       std::is_same_v<const volatile From, To>;

/// # `best::is_object`
///
/// An object type: anything that is not a reference, function, or void.
template <typename T>
concept is_object = std::is_object_v<T>;

/// # `best::is_func`
///
/// A function type. This includes "abominable function" types that cannot form
/// function pointers, such as `int (int) const`.
template <typename T>
concept is_func = std::is_function_v<T>;

/// # `best::is_tame_func`
///
/// A tame function type. These are function types `F` such that `F*` is
/// well-formed; they lack ref and cv qualifiers.
template <typename T>
concept is_tame_func = is_func<T> && requires { (T*){}; };

/// # `best::is_abominable`
///
/// An abominable function type, i.e., a function type with qualifiers.
template <typename T>
concept is_abominable = is_func<T> && !requires { (T*){}; };

/// # `best::is_const_func`
///
/// Whether this is a `const`-qualified abominable function.
template <typename T>
concept is_const_func = abominable_internal::tame<T>::c;

/// # `best::is_lref_func`
///
/// Whether this is a `&`-qualified abominable function.
template <typename T>
concept is_lref_func = abominable_internal::tame<T>::l;

/// # `best::is_rref_func`
///
/// Whether this is a `&&`-qualified abominable function.
template <typename T>
concept is_rref_func = abominable_internal::tame<T>::r;

/// # `best::is_ref_func`
///
/// Whether this is a `&`- or `&&-qualified abominable function.
template <typename T>
concept is_ref_func = is_lref_func<T> || is_rref_func<T>;

/// # `best::tame<T>`
///
/// If `T` is an abominable function, returns its "tame" counterpart. Otherwise,
/// returns `T`.
template <typename T>
using tame = abominable_internal::tame<T>::type;

/// # `best::ref_kind`
///
/// A kind of reference, for use in type traits.
enum class ref_kind : uint8_t {
  Lvalue = 0,  // T&
  Rvalue = 1,  // T&&
};

/// # `best::is_lref`
///
/// An lvalue reference type.
template <typename T>
concept is_lref = std::is_lvalue_reference_v<T>;

/// # `best::is_rref`
///
/// An rvalue reference type.
template <typename T>
concept is_rref = std::is_rvalue_reference_v<T>;

/// # `best::is_ref`
///
/// A reference type, selectable based on the `kind` parameter. Not specifying
/// it makes this concept recognize both kinds of references.
template <typename T, ref_kind kind = ref_kind(-1)>
concept is_ref = (kind != ref_kind::Rvalue && is_lref<T>) ||
                 (kind != ref_kind::Lvalue && is_rref<T>);

/// # `best::as_ref<T>
///
/// If `T` is an object or tame function type, returns an (lvalue) reference to
/// it. If `T` is an lvalue reference or void, returns `T`. If `T` is an rvalue
/// reference, returns `best::unref<T>&&`.
///
/// Currently, abominable functions are left unmodified. This may change in the
/// future.
template <typename T>
using as_ref = std::add_lvalue_reference_t<T>;

/// # `best::as_rref<T>
///
/// If `T` is an object or tame function type, returns an rvalue reference to
/// it. If `T` is an reference or void, returns `T`.
///
/// Currently, abominable functions are left unmodified. This may change in the
/// future.
template <typename T>
using as_rref = std::add_rvalue_reference_t<T>;

/// # `best::refcopy<Dst, Src>
///
///
/// If `Src` is a reference type, the kind of reference, and cv-qualification,
/// are copied to `Dst`, as if by the appropriate call to `as_ref<>` or
/// `as_rref<>`.
template <typename Dst, typename Src>
using refcopy = best::quals_internal::refs<Dst, Src>::copied;

/// # `best::unref<T>`
///
/// If `T` is an object or function reference, returns its referent; otherwise,
/// returns `T`.
template <typename T>
using unref = std::remove_reference_t<T>;

/// # `best::as_auto<T>`
///
/// Performs `auto` deduction: if `T` is a reference and/or is cv-qualified,
/// the reference and qualification is removed.
template <typename T>
using as_auto = std::remove_cvref_t<T>;

/// # `best::is_ptr`
///
/// Whether this is a pointer type.
template <typename T>
concept is_ptr = std::is_pointer_v<T>;

/// # `best::is_member_ptr`
///
/// Whether this is a pointer-to-member type.
template <typename T>
concept is_member_ptr = std::is_member_pointer_v<T>;

/// # `best::is_func_ptr`
///
/// Whether this is a function pointer type.
template <typename T>
concept is_func_ptr =
  best::is_ptr<T> && best::is_func<std::remove_pointer_t<T>>;

/// # `best::as_ptr<T>`
///
/// If `T` is an object, void, or tame function type, returns a pointer to it.
/// If `T` is an abominable function, it returns `best::tame<F>*`. If `T` is
/// a reference type, it returns `best::unref<T>*`.
///
/// The returned type is *always* a pointer.
template <typename T>
using as_ptr = std::add_pointer_t<best::tame<T>>;

/// # `best::unptr<T>`
///
/// If `T` is an object, function, or void pointer, returns its pointee;
/// otherwise, returns `T`.
template <typename T>
using unptr = std::remove_pointer_t<T>;

/// # `best::is_deref<T>`
///
/// Whether a type `T` is "properly dereferenceable", that is, it has an
/// `operator*` that returns some type, and an `operator->` that returns a
/// pointer. If `Target` is `void`, the types of these operators is not checked.
/// Otherwise, both operators must yield reference/pointer to `Target`,
/// respectively.
template <typename T, typename Target = void>
concept is_deref =
  (is_ptr<T> &&
   (is_void<Target> || std::is_same_v<Target, best::unqual<best::unptr<T>>>)) ||
  (is_void<T> && requires(T& r) {
    operator*(r);
    r.operator->();
  }) || requires(const T& cr) {
    { operator*(cr) } -> std::convertible_to<const Target&>;
    { cr.operator->() } -> std::convertible_to<const Target*>;
  };

/// # `best::addr()`
///
/// Obtains the address of a reference without hitting `operator&`.
constexpr auto addr(auto&& ref) { return __builtin_addressof(ref); }

/// # `best::is_struct`
///
/// Identifies a "struct type". In our highly narrow definition, this is a
/// class type that is an aggregate.
template <typename T>
concept is_struct = std::is_class_v<T> && std::is_aggregate_v<T>;

/// # `best::is_enum`
///
/// Identifies an enumeration type.
template <typename T>
concept is_enum = std::is_enum_v<T>;

/// # `best::is_virtual`
///
/// Identifies a virtual (polymorphic) type: one which declares or inherits
/// at least one virtual function or base.
template <typename T>
concept is_virtual = std::is_polymorphic_v<T>;

/// # `best::is_abstract`
///
/// Identifies an abstract type: one which declares or inherits at least one
/// pure virtual function.
template <typename T>
concept is_abstract = best::is_virtual<T> && std::is_abstract_v<T>;

/// # `best::is_concrete`
///
/// Identifies a concrete type: one which is not abstract.
template <typename T>
concept is_concrete = !best::is_abstract<T>;

/// # `best::is_open`
///
/// Identifies a type that can be used as a base class.
template <typename T>
concept is_open = std::is_final_v<T> && std::is_class_v<T>;

/// # `best::is_final`
///
/// Identifies a type that cannot be used as a base class. This includes both
/// `final` class types and non-class types.
template <typename T>
concept is_final = !std::is_final_v<T>;

/// # `best::derives`
///
/// Checks whether `T` derives `Base`.
template <typename T, typename Base>
concept derives = std::is_base_of_v<Base, T>;

/// # `best::static_derives`
///
/// Checks whether `T` statically derives `Base`.
template <typename T, typename Base>
concept static_derives = requires(Base* base) {
  requires best::derives<T, Base>;
  static_cast<T*>(base);
};

/// # `best::virtual_derives`
///
/// Checks whether `T` statically derives `Base`.
template <typename T, typename Base>
concept virtual_derives =
  best::derives<T, Base> && !best::static_derives<T, Base>;

template <typename T>
concept is_array = std::is_array_v<T>;

template <typename T, size_t n = -1>
concept is_bounded_array =
  std::is_bounded_array_v<T> && (n == -1 || n == std::extent_v<T>);

template <typename T>
concept is_unbounded_array = std::is_unbounded_array_v<T>;

template <typename T>
using unarray = std::remove_extent_t<T>;
}  // namespace best

#endif  // BEST_META_TAXONOMY_H_
