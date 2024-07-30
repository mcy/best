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

#include "best/base/hint.h"
#include "best/base/port.h"
#include "best/memory/layout.h"
#include "best/meta/init.h"
#include "best/meta/traits/empty.h"

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
class object_meta {
 public:
  using type = T;
  using pointee = T;
  using metadata = best::empty;
  using as_const = const T;

  constexpr object_meta() = default;
  constexpr object_meta(metadata) {}
  constexpr const metadata& to_metadata() const { return m_; }

  template <typename P>
  constexpr explicit(
    !best::same<best::un_qual<typename P::type>, best::un_qual<T>>)
    object_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<pointee>(); }
  static constexpr auto deref(pointee* ptr) { return ptr; }

  static constexpr object_meta meta_for(auto&&...) { return {}; }
  static constexpr void construct(pointee* dst, bool assign, auto&&... args)
    requires best::constructible<T, decltype(args)&&...>
  {
    if (assign) {
      if constexpr (best::assignable<T, decltype(args)&&...>) {
        *dst = (BEST_FWD(args), ...);
        return;
      }
      destroy(dst);
    }
    new (dst) T(BEST_FWD(args)...);
  }

  static constexpr bool is_statically_copyable() { return best::copyable<T>; }
  static constexpr bool is_dynamically_copyable() { return best::copyable<T>; }
  static constexpr void copy(pointee* dst, pointee* src, bool assign) {
    if constexpr (best::copyable<T>) {
      if (assign) {
        *dst = *src;
      } else {
        new (dst) T(*src);
      }
    }
  }

  static constexpr void destroy(pointee* ptr) { ptr->~T(); }

 private:
  [[no_unique_address]] best::empty m_;
};

// The pointer metadata type for a reference or function type.
template <typename T>
class ptr_like_meta {
 public:
  using type = T;
  using pointee = best::as_raw_ptr<T> const;
  using metadata = best::empty;
  using as_const = T;

  constexpr ptr_like_meta() = default;
  constexpr ptr_like_meta(metadata) {}
  constexpr const metadata& to_metadata() const { return m_; }

  template <typename P>
  constexpr explicit(false /* All ref and function conversions are lossless. */)
    ptr_like_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<pointee>(); }
  static constexpr auto deref(pointee* ptr) { return *ptr; }

  static constexpr ptr_like_meta meta_for(auto&&...) { return {}; }
  static constexpr void construct(pointee* dst, bool assign, auto&&... args)
    requires best::constructible<T, decltype(args)&&...>
  {
    if constexpr (best::is_ref<T>) {
      *const_cast<best::un_qual<pointee>*>(dst) = best::addr(args...);
    } else if constexpr (best::is_func<T>) {
      *const_cast<best::un_qual<pointee>*>(dst) = (args, ...);
    }
  }

  static constexpr bool is_statically_copyable() { return true; }
  static constexpr bool is_dynamically_copyable() { return true; }
  static constexpr void copy(pointee* dst, pointee* src, bool assign) {
    new (dst) pointee(*src);
  }

  static constexpr void destroy(pointee* ptr) {}

 private:
  [[no_unique_address]] best::empty m_;
};

// The pointer metadata type for a void type.
template <best::is_void T>
class void_meta {
 public:
  using type = T;
  using pointee = T;
  using metadata = best::empty;
  using as_const = const T;

  constexpr void_meta() = default;
  constexpr void_meta(metadata) {}
  constexpr const metadata& to_metadata() const { return m_; }

  template <typename P>
  constexpr explicit(!best::is_void<typename P::type>)
    void_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<void>(); }
  static constexpr auto deref(pointee* ptr) { return ptr; }

  static constexpr void_meta meta_for(auto&&...) { return {}; }
  static constexpr void construct(pointee*, bool, auto&&... args)
    requires best::constructible<T, decltype(args)&&...>
  {}

  static constexpr bool is_statically_copyable() { return true; }
  static constexpr bool is_dynamically_copyable() { return true; }
  static constexpr void copy(pointee* dst, pointee* src, bool assign) {}

  static constexpr void destroy(pointee* ptr) {}

 private:
  [[no_unique_address]] best::empty m_;
};

template <typename A>
class array_meta;

template <best::is_object T, size_t len>
class array_meta<T[len]> {
 public:
  using type = T[len];
  using pointee = T[len];
  using metadata = best::empty;
  using as_const = const T[len];

  constexpr array_meta() = default;
  constexpr array_meta(metadata) {}
  constexpr const metadata& to_metadata() const { return m_; }

  template <typename P>
  constexpr explicit(
    !best::same<best::un_qual<typename P::type>, best::un_qual<T>>)
    array_meta(best::tlist<P>, const typename P::metadata&)
      requires best::convertible<pointee*, typename P::pointee*>
  {}

  static constexpr best::layout layout() { return best::layout::of<T[len]>(); }
  // TODO: Switch view types. Doing this properly will unfortunately require
  // cooking up some way to do `best::span<T&&>` or such.
  //
  // constexpr best::span<T, len> deref(pointee* ptr) const {
  //   return best::span<T, len>{std::data(*ptr), len};
  // }
  static constexpr pointee* deref(pointee* ptr) { return ptr; }

  static constexpr array_meta meta_for(auto&&...) { return {}; }
  template <typename U>
  constexpr void construct(pointee* dst, bool assign,
                           best::span<U, len> s) const
    requires best::constructible<T, U&>
  {
    for (size_t i = 0; i < len; ++i) {
      best::ptr next = best::addr((*dst)[i]);
      if (assign) {
        next.assign(s.data()[i]);
      } else {
        next.construct(s.data()[i]);
      }
    }
  }
  template <typename U>
  constexpr void construct(pointee* dst, bool assign, U (&s)[len]) const
    requires best::constructible<T, U&>
  {
    for (size_t i = 0; i < len; ++i) {
      best::ptr next = best::addr((*dst)[i]);
      if (assign) {
        next.assign(s[i]);
      } else {
        next.construct(s[i]);
      }
    }
  }
  template <typename U>
  constexpr void construct(pointee* dst, bool assign, U (&&s)[len]) const
    requires best::constructible<T, U&>
  {
    for (size_t i = 0; i < len; ++i) {
      best::ptr next = best::addr((*dst)[i]);
      if (assign) {
        next.assign(BEST_MOVE(s[i]));
      } else {
        next.construct(BEST_MOVE(s[i]));
      }
    }
  }

  static constexpr bool is_statically_copyable() { return best::copyable<T>; }
  static constexpr bool is_dynamically_copyable() { return best::copyable<T>; }
  constexpr void copy(pointee* dst, pointee* src, bool assign) const {
    if constexpr (best::copyable<T>) { construct(dst, assign, *src); }
  }

  constexpr void destroy(pointee* ptr) const {
    for (size_t i = 0; i < len; ++i) { (*ptr)[i].~T(); }
  }

 private:
  [[no_unique_address]] best::empty m_;
};

/// The pointer metadata for an unbounded array type `T[]`.
template <best::is_object T>
class array_meta<T[]> {
 public:
  using type = T[];
  using pointee = T;
  using metadata = size_t;
  using as_const = const T[];

  constexpr array_meta() = default;
  constexpr array_meta(metadata len) : len_(len) {}
  constexpr const metadata& to_metadata() const { return len_; }

  template <best::qualifies_to<T> U>
  constexpr array_meta(best::tlist<best::ptr<U>>, const auto&)
    requires (best::ptr<U>::is_thin())
    : len_(1) {}
  template <best::qualifies_to<T> U, size_t n>
  constexpr array_meta(best::tlist<best::ptr<std::array<U, n>>>,
                       const best::empty&)
    : len_(n) {}
  template <best::qualifies_to<T> U, size_t n>
  constexpr array_meta(best::tlist<best::ptr<U[n]>>, const best::empty&)
    : len_(n) {}
  template <best::qualifies_to<T> U>
  constexpr array_meta(best::tlist<best::ptr<U[]>>, const size_t& n)
    : len_(n) {}

  constexpr best::layout layout() const { return best::layout::array<T>(len_); }
  constexpr best::span<T> deref(pointee* ptr) const { return {ptr, len_}; }

  static constexpr array_meta meta_for() { return 0; }
  constexpr void construct(pointee* dst, bool assign) const {}

  static constexpr array_meta meta_for(auto&& s) { return s.size(); }
  template <typename U>
  constexpr void construct(pointee* dst, bool assign, best::span<U> s) const
    requires best::constructible<T, U&>
  {
    if (assign) {
      deref(dst).copy_from(s);
    } else {
      deref(dst).emplace_from(s);
    }
  }
  template <typename U>
  constexpr void construct(pointee* dst, bool assign,
                           std::initializer_list<U> s) const
    requires best::constructible<T, U&>
  {
    construct(dst, assign, best::span<const U>(s));
  }

  static constexpr bool is_statically_copyable() { return best::copyable<T>; }
  static constexpr bool is_dynamically_copyable() { return best::copyable<T>; }
  constexpr void copy(pointee* dst, pointee* src, bool assign) const {
    if constexpr (best::copyable<T>) {
      construct(dst, assign, best::span{src, len_});
    }
  }

  constexpr void destroy(pointee* ptr) const {
    for (size_t i = 0; i < len_; ++i) { ptr[i].~T(); }
  }

 private:
  size_t len_ = 0;
};

struct access {
  template <typename T>
  static auto get_meta() {
    if constexpr (requires { typename T::BestPtrMetadata; }) {
      return best::id<typename T::BestPtrMetadata>{};
    } else if constexpr (best::is_array<T>) {
      return best::id<array_meta<T>>{};
    } else if constexpr (best::is_object<T>) {
      return best::id<object_meta<T>>{};
    } else if constexpr (best::is_void<T>) {
      return best::id<void_meta<T>>{};
    } else {
      return best::id<ptr_like_meta<T>>{};
    }
  }
};

template <typename T>
using meta = decltype(access::get_meta<T>())::type;

}  // namespace best::ptr_internal

#endif  // BEST_MEMORY_INTERNAL_PTR_H_
