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

#ifndef BEST_MEMORY_INTERNAL_BYTES_H_
#define BEST_MEMORY_INTERNAL_BYTES_H_

#include <cstddef>
#include <type_traits>

#include "best/base/hint.h"
#include "best/base/port.h"
#include "best/container/option.h"
#include "best/meta/taxonomy.h"
#include "best/meta/tlist.h"

#define BEST_CONSTEXPR_MEMCMP_ BEST_HAS_FEATURE(cxx_constexpr_string_builtins)

namespace best::bytes_internal {
extern "C" {
void* memcpy(void*, const void*, size_t) noexcept;
void* memmove(void*, const void*, size_t) noexcept;
void* memchr(const void*, int, size_t) noexcept;
void* memmem(const void*, size_t, const void*, size_t) noexcept;
void* memset(void*, int, size_t) noexcept;
int memcmp(const void*, const void*, size_t) noexcept;
size_t strlen(const char*) noexcept;
}  // extern "C"

#if BEST_CONSTEXPR_MEMCMP_
#define BEST_memcmp_ __builtin_memcmp
#define BEST_memchr_ __builtin_char_memchr
#else
#define BEST_memcmp_ best::bytes_internal::memcmp
#define BEST_memchr_ best::bytes_internal::memchr
#endif

template <typename>
struct tag {};

template <typename T, typename U>
void can_memcmp(tag<T>, tag<U>)
  requires requires {
    requires sizeof(T) == sizeof(U);
#if BEST_HAS_BUILTIN(__is_trivially_equality_comparable)
    requires __is_trivially_equality_comparable(T);
    requires __is_trivially_equality_comparable(U);
#else
    requires std::is_integral_v<T>;
    requires std::is_integral_v<U>;
#endif
  };

template <typename T, typename U>
void can_memcmp(tag<T*>, tag<U*>)
  requires requires {
    // Function pointer equality is all kinds of messed up.

    requires !best::is_func<T> && !best::is_func<U>;
    // libc++ asserts there's something weird about virtual bases.
    // This is not perfect detection for that case but allows a
    // lot more "reasonable" code than libc++'s constraint.
    //
    // See:
    // https://github.com/llvm/llvm-project/blob/ecf2a53407f517a261ee296e1f922c647a13a503/libcxx/include/__type_traits/is_equality_comparable.h#L41
    requires !std::is_polymorphic_v<T> && !std::is_polymorphic_v<U>;

    // Some platforms are just weird, man.
    requires sizeof(T*) == sizeof(U*);
  };

template <typename T>
concept is_char = best::same<best::as_auto<T>, char> ||
                  best::same<best::as_auto<T>, unsigned char> ||
                  best::same<best::as_auto<T>, signed char> ||
                  best::same<best::as_auto<T>, char8_t>;

template <typename T, typename U>
BEST_INLINE_ALWAYS constexpr bool equate(best::span<T> lhs, best::span<U> rhs) {
  if (lhs.size() != rhs.size()) return false;
  if (lhs.is_empty()) return true;

  if (!std::is_constant_evaluated()) {
    if (lhs.data() == rhs.data()) return true;
  }
  return 0 == BEST_memcmp_(lhs.data(), rhs.data(), lhs.size() * sizeof(T));
}

template <typename T, typename U = const T>
BEST_INLINE_ALWAYS constexpr best::ord compare(best::span<T> lhs,
                                               best::span<U> rhs) {
  if (lhs.is_empty() || rhs.is_empty()) return lhs.size() <=> rhs.size();
  if (!std::is_constant_evaluated()) {
    if (lhs.data() == rhs.data()) return lhs.size() <=> rhs.size();
  }

  auto to_compare = best::min(lhs.size(), rhs.size()) * sizeof(T);
  int result = BEST_memcmp_(lhs.data(), rhs.data(), to_compare);

  if (result == 0) {
    return lhs.size() <=> rhs.size();
  }
  return result <=> 0;
}

template <typename T, typename U>
best::option<size_t> constexpr search(best::span<T> haystack,
                                      best::span<U> needle) {
  auto* data =
      reinterpret_cast<const char*>(static_cast<const void*>(haystack.data()));
  size_t size = haystack.size() * sizeof(T);
  size_t travel = 0;

  do {
    void* found = bytes_internal::memmem(data, size, needle.data(),
                                         needle.size() * sizeof(T));
    if (!found) return best::none;

    size_t offset = static_cast<const char*>(found) - data;
    travel += offset;
    data += offset;
    size -= offset;
  } while (travel % alignof(T) != 0);

  return travel / sizeof(T);
}

template <typename T, typename U = const T>
BEST_INLINE_ALWAYS constexpr best::option<size_t> search_constexpr(
    best::span<T> haystack, best::span<U> needle) {
  size_t hz = haystack.size();
  size_t nz = needle.size();

  if (nz == 0) return 0;
  if (hz < nz) return best::none;

  T* hp = haystack.data().raw();
  U* np = needle.data().raw();

  U first = *np;
  const T* end = hp + hz;
  while (true) {
    size_t len = end - hp;
    if (len < nz) return best::none;

    // Skip to the next possible candidate.
    hp = (T*)BEST_memchr_(hp, first, len);
    if (hp == nullptr) return best::none;

    // Check if we actually found the string.
    if (BEST_memcmp_(hp, np, nz) == 0) {
      return hp - haystack.data().raw();
    }
    ++hp;
  }
}

#undef BEST_memcmp_
#undef BEST_memchr_
}  // namespace best::bytes_internal

#endif  // BEST_MEMORY_INTERNAL_BYTES_H_
