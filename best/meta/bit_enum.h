#ifndef BEST_META_BIT_ENUM_H_
#define BEST_META_BIT_ENUM_H_

#include <type_traits>

//! Concepts and type traits.

namespace best {
/// Generates overloaded operators that make an enum into a bitset.
///
/// Given an enum type `Enum_`, this will automatically generate operators for
/// &, |, ^, ~, ==, and !=. Comparisons can be done between enum values and
/// its underlying type.
#define BEST_BIT_ENUM(Enum_)                                          \
  inline constexpr Enum_ operator|(Enum_ a, Enum_ b) {                \
    using U = std::underlying_type_t<Enum_>;                          \
    return static_cast<Enum_>(static_cast<U>(a) | static_cast<U>(b)); \
  }                                                                   \
  inline constexpr Enum_ operator&(Enum_ a, Enum_ b) {                \
    using U = std::underlying_type_t<Enum_>;                          \
    return static_cast<Enum_>(static_cast<U>(a) & static_cast<U>(b)); \
  }                                                                   \
  inline constexpr Enum_ operator^(Enum_ a, Enum_ b) {                \
    using U = std::underlying_type_t<Enum_>;                          \
    return static_cast<Enum_>(static_cast<U>(a) ^ static_cast<U>(b)); \
  }                                                                   \
  inline constexpr Enum_ operator~(Enum_ a) {                         \
    using U = std::underlying_type_t<Enum_>;                          \
    return static_cast<Enum_>(~static_cast<U>(a));                    \
  }

#define BEST_ENUM_CMP(Enum_)                                                   \
  inline constexpr bool operator==(Enum_ a, Enum_ b) {                         \
    using U = std::underlying_type_t<Enum_>;                                   \
    return static_cast<U>(a) == static_cast<U>(b);                             \
  }                                                                            \
  inline constexpr bool operator!=(Enum_ a, Enum_ b) {                         \
    using U = std::underlying_type_t<Enum_>;                                   \
    return static_cast<U>(a) != static_cast<U>(b);                             \
  }                                                                            \
  inline constexpr bool operator==(Enum_ a, std::underlying_type_t<Enum_> b) { \
    using U = std::underlying_type_t<Enum_>;                                   \
    return static_cast<U>(a) == b;                                             \
  }                                                                            \
  inline constexpr bool operator!=(Enum_ a, std::underlying_type_t<Enum_> b) { \
    using U = std::underlying_type_t<Enum_>;                                   \
    return static_cast<U>(a) != b;                                             \
  }                                                                            \
  inline constexpr bool operator==(std::underlying_type_t<Enum_> a, Enum_ b) { \
    using U = std::underlying_type_t<Enum_>;                                   \
    return a == static_cast<U>(b);                                             \
  }                                                                            \
  inline constexpr bool operator!=(std::underlying_type_t<Enum_> a, Enum_ b) { \
    using U = std::underlying_type_t<Enum_>;                                   \
    return a != static_cast<U>(b);                                             \
  }

namespace bit_enum_internal {
using mark_as_used = std::underlying_type<void>;
}
}  // namespace best

#endif  // BEST_META_BIT_ENUM_H_