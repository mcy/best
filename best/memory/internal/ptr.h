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

#ifndef BEST_MEMORY_INTERNAL_PTR_H_
#define BEST_MEMORY_INTERNAL_PTR_H_

#include <cstddef>
#include <type_traits>

#include "best/base/hint.h"
#include "best/base/port.h"
#include "best/memory/layout.h"
#include "best/meta/empty.h"
#include "best/meta/init.h"
#include "best/meta/taxonomy.h"

#define BEST_CONSTEXPR_MEMCPY_ BEST_HAS_FEATURE(cxx_constexpr_string_builtins)

namespace best::ptr_internal {
#if BEST_CONSTEXPR_MEMCPY_
BEST_INLINE_SYNTHETIC constexpr void* memcpy(void* dst, const void* src,
                                             size_t len) {
  return __builtin_memcpy(dst, src, len);
}
BEST_INLINE_SYNTHETIC constexpr void* memmove(void* dst, const void* src,
                                              size_t len) {
  return __builtin_memmove(dst, src, len);
}
#else
extern "C" {
void* memcpy(void*, const void*, size_t) noexcept;
void* memmove(void*, const void*, size_t) noexcept;
}
#endif

extern "C" void* memset(void*, int, size_t) noexcept;

// The pointer metadata type for an ordinary object type `T`.
template <best::is_object T>
struct object_meta {
  using pointee = T;
  using metadata = best::empty;

  constexpr object_meta() = default;
  constexpr object_meta(metadata) {}
  static constexpr metadata to_metadata() { return {}; }

  template <typename P>
  constexpr explicit(
    !best::same<best::unqual<typename P::type>, best::unqual<T>>)
    object_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<pointee>(); }
  static constexpr best::layout stride() { return best::layout::of<pointee>(); }

  static constexpr object_meta offset(ptrdiff_t offset) { return {}; }

  static constexpr T* deref(pointee* ptr) { return ptr; }

  static constexpr bool copyable() { return best::copyable<T>; }
  static constexpr void copy(void* dst, pointee* src) {
    if constexpr (best::copyable<T>) { new (dst) T(*src); }
  }

  static constexpr void destroy(pointee* ptr) { ptr->~T(); }
};

// The pointer metadata type for a reference or function type.
template <typename T>
struct ptr_like_meta {
  using pointee = best::as_ptr<T> const;
  using metadata = best::empty;

  constexpr ptr_like_meta() = default;
  constexpr ptr_like_meta(metadata) {}
  static constexpr metadata to_metadata() { return {}; }

  template <typename P>
  constexpr explicit(false /* All ref and function conversions are lossless. */)
    ptr_like_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<pointee>(); }
  static constexpr best::layout stride() { return best::layout::of<pointee>(); }

  static constexpr ptr_like_meta offset(ptrdiff_t offset) { return {}; }

  static constexpr pointee deref(pointee* ptr) { return *ptr; }

  static constexpr bool copyable() { return true; }
  static constexpr void copy(void* dst, pointee* src) {
    new (dst) pointee(*src);
  }

  static constexpr void destroy(pointee* ptr) {}
};

// The pointer metadata type for a void type.
template <best::is_void T>
struct void_meta {
  using pointee = T;
  using metadata = best::empty;

  constexpr void_meta() = default;
  constexpr void_meta(metadata) {}
  static constexpr metadata to_metadata() { return {}; }

  template <typename P>
  constexpr explicit(!best::is_void<typename P::type>)
    void_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<void>(); }
  static constexpr best::layout stride() { return best::layout::of<void>(); }

  static constexpr void_meta offset(ptrdiff_t offset) { return {}; }

  static constexpr T* deref(pointee* ptr) { return ptr; }

  static constexpr bool copyable() { return true; }
  static constexpr void copy(void* dst, pointee* src) {}

  static constexpr void destroy(pointee* ptr) {}
};

/// The pointer metadata for an unbounded array type `T[]`.
template <best::is_object T>
struct unbounded_meta {
  using pointee = T;
  using metadata = size_t;

  constexpr unbounded_meta(size_t len) : len(len) {}
  constexpr size_t to_metadata() const { return len; }

  template <best::qualifies_to<T> U, size_t n>
  constexpr unbounded_meta(best::tlist<U>, const best::empty&)
    : len(size_of<U>) {}
  template <best::qualifies_to<T> U, size_t n>
  constexpr unbounded_meta(best::tlist<std::array<U, n>>, const best::empty&)
    : len(n * size_of<U>) {}
  template <best::qualifies_to<T> U, size_t n>
  constexpr unbounded_meta(best::tlist<U[n]>, const best::empty&)
    : len(n * size_of<U>) {}
  template <best::qualifies_to<T> U>
  constexpr unbounded_meta(best::tlist<U[]>, const size_t& n) : len(n) {}

  constexpr best::layout layout() const { return best::layout::array<T>(len); }
  static best::layout stride() { return best::layout::of<T>(); }

  constexpr unbounded_meta offset(ptrdiff_t offset) { return len - offset; }

  constexpr best::span<T> deref(pointee* ptr) const { return {ptr, len}; }

  static constexpr bool copyable() { return best::copyable<T>; }
  void copy(void* dst, pointee* src) const {
    // This is not constexpr... :/
    for (size_t i = 0; i < len; ++i) {
      new (dst) T(src[i]);
      dst = static_cast<const char*>(dst) + best::size_of<T>;
    }
  }

  constexpr void destroy(pointee* ptr) const {
    for (size_t i = 0; i < len; ++i) { ptr[i].~T(); }
  }

  size_t len = 0;
};

template <typename T>
using meta = decltype([] {
  if constexpr (requires { typename T::BestPtrMetadata; }) {
    return best::id<typename T::BestPtrMetadata>{};
  } else if constexpr (std::is_unbounded_array_v<T>) {
    return best::id<unbounded_meta<T>>{};
  } else if constexpr (best::is_object<T>) {
    return best::id<object_meta<T>>{};
  } else if constexpr (best::is_void<T>) {
    return best::id<void_meta<T>>{};
  } else {
    return best::id<ptr_like_meta<T>>{};
  }
}())::type;

}  // namespace best::ptr_internal

#endif  // BEST_MEMORY_INTERNAL_PTR_H_
