#ifndef BEST_META_CONCEPTS_H_
#define BEST_META_CONCEPTS_H_

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#include "best/meta/internal/abominable.h"
#include "best/meta/internal/quals.h"
#include "best/meta/tlist.h"

//! Concepts and type traits.

namespace best {
/// Variadic version of std::is_same/std::same_as.
template <typename... Ts>
concept same =
    (std::same_as<Ts, typename best::tlist<Ts...>::template type<0, void>> &&
     ...);

/// "Tames" a function by removing cvref-qualifiers.
///
/// In other words, it converts an abominable function type into its equivalent
/// ordinary function type.
template <typename F>
using tame_func = best::abominable_internal::tame<F>::type;

/// A kind of reference, for use in type traits.
enum class ref_kind : uint8_t {
  Lvalue = 0,  // T&
  Rvalue = 1,  // T&&
};

/// Adds a reference to T, if it is a referenceable type.
template <typename T, ref_kind kind = ref_kind::Lvalue>
using as_ref =
    std::conditional_t<kind == ref_kind::Lvalue, std::add_lvalue_reference<T>,
                       std::add_rvalue_reference<T>>::type;
template <typename T>
using as_rref = as_ref<T, ref_kind::Rvalue>;

/// Removes a reference from T, if it is a reference type.
template <typename T>
using as_deref = std::remove_reference_t<T>;

/// Removes cv-qualifiers from T.
template <typename T>
using as_dequal = std::remove_cv_t<T>;

/// Adds a pointer to T, if it is a pointable type.
///
/// If T is a reference, the reference is replaced with a pointer, so
/// best::as_ptr<int&> is int*.
///
/// If T is a function type, it is first tamed.
template <typename T>
using as_ptr = std::add_pointer_t<T>;

/// Copies any top-level cv-qualification from Src to Dst.
template <typename Dst, typename Src>
using copy_quals = best::quals_internal::quals<Dst, Src>::copied;

/// Copies a reference (and cv-qualification for that reference) from Src to
/// Dst, as-if by best::as_ref.
template <typename Dst, typename Src>
using copy_ref = best::quals_internal::refs<Dst, Src>::copied;

/// An object type, i.e., anything that is not a reference, a function type,
/// or a void type.
template <typename T>
concept object_type = std::is_object_v<T>;

/// A reference type, referring to either an object or ordinary function type.
template <typename T, ref_kind kind = ref_kind{0xff}>
concept ref_type = (kind == ref_kind::Lvalue   ? std::is_lvalue_reference_v<T>
                    : kind == ref_kind::Rvalue ? std::is_rvalue_reference_v<T>
                                               : std::is_reference_v<T>);

/// A void type, i.e. cv-qualified void.
template <typename T>
concept void_type = std::is_void_v<T>;

/// An ordinary function type, i.e. one that can be made into a function
/// pointer.
template <typename T>
concept func_type = std::is_function_v<T> && std::is_same_v<T, as_ref<T>>;

/// An abominable function type, i.e. one that is e.g. cv-qualified.
template <typename T>
concept abominable_func_type = std::is_function_v<T>;

/// Whether U can be formed from T by adding cv-qualifiers.
template <typename T, typename U>
concept qualifies_to = same<T, U> || same<const T, U> || same<volatile T, U> ||
                       same<const volatile T, U>;

/// Whether a type T can be safely memcmp'd.
template <typename T>
concept can_memcmp =
    std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

/// Computes the smallest unsigned integer type that can represent `n`.
// TODO: move to the integers header.
template <uint64_t n>
struct smallest_uint
    : std::conditional<
          n <= UINT8_MAX, uint8_t,
          std::conditional_t<
              n <= UINT16_MAX, uint16_t,
              std::conditional_t<n <= UINT32_MAX, uint32_t, uint64_t>>> {};
template <uint64_t n>
using smallest_uint_t = smallest_uint<n>::type;

}  // namespace best

#endif  // BEST_META_CONCEPTS_H_