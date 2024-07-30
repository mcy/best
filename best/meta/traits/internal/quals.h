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

#ifndef BEST_META_TRAITS_INTERNAL_QUALS_H_
#define BEST_META_TRAITS_INTERNAL_QUALS_H_

#include <type_traits>

namespace best::traits_internal {
template <typename T>
struct const_ {
  using add = const T;
  using remove = T;
};
template <typename T>
struct const_<const T> {
  using add = const T;
  using remove = T;
};
template <typename T, auto n>
struct const_<T[n]> {
  using add = const T[n];
  using remove = T[n];
};
template <typename T, auto n>
struct const_<const T[n]> {
  using add = const T[n];
  using remove = T[n];
};
template <typename T>
struct const_<T[]> {
  using add = const T[];
  using remove = T[];
};
template <typename T>
struct const_<const T[]> {
  using add = const T[];
  using remove = T[];
};

template <typename T>
struct volatile_ {
  using add = volatile T;
  using remove = T;
};
template <typename T>
struct volatile_<volatile T> {
  using add = volatile T;
  using remove = T;
};
template <typename T, auto n>
struct volatile_<T[n]> {
  using add = volatile T[n];
  using remove = T[n];
};
template <typename T, auto n>
struct volatile_<volatile T[n]> {
  using add = volatile T[n];
  using remove = T[n];
};
template <typename T>
struct volatile_<T[]> {
  using add = volatile T[];
  using remove = T[];
};
template <typename T>
struct volatile_<volatile T[]> {
  using add = volatile T[];
  using remove = T[];
};

template <typename Dst, typename Src>
struct quals {
  using copied = Dst;
};
template <typename Dst, typename Src>
requires std::is_reference_v<Src> || std::is_function_v<Src>
struct quals<Dst, Src> {
  using copied = const_<Dst>::add;
};

template <typename Dst, typename Src>
struct quals<Dst, const Src> {
  using copied = const_<Dst>::add;
};
template <typename Dst, typename Src>
struct quals<Dst, volatile Src> {
  using copied = volatile_<Dst>::add;
};
template <typename Dst, typename Src>
struct quals<Dst, const volatile Src> {
  using copied = const_<typename volatile_<Dst>::add>::add;
};

template <typename Dst, typename Src, auto n>
struct quals<Dst, const Src[n]> {
  using copied = const_<Dst>::add;
};
template <typename Dst, typename Src, auto n>
struct quals<Dst, volatile Src[n]> {
  using copied = volatile_<Dst>::add;
};
template <typename Dst, typename Src, auto n>
struct quals<Dst, const volatile Src[n]> {
  using copied = const_<typename volatile_<Dst>::add>::add;
};

template <typename Dst, typename Src>
struct quals<Dst, const Src[]> {
  using copied = const_<Dst>::add;
};
template <typename Dst, typename Src>
struct quals<Dst, volatile Src[]> {
  using copied = volatile_<Dst>::add;
};
template <typename Dst, typename Src>
struct quals<Dst, const volatile Src[]> {
  using copied = const_<typename volatile_<Dst>::add>::add;
};

template <typename Dst, typename Src>
struct refs {
  using copied = quals<Dst, Src>::copied;
};
template <typename Dst, typename Src>
struct refs<Dst, Src&> {
  using copied = std::add_lvalue_reference_t<typename quals<Dst, Src>::copied>;
};
template <typename Dst, typename Src>
struct quals<Dst, Src&&> {
  using copied = std::add_rvalue_reference_t<typename quals<Dst, Src>::copied>;
};
}  // namespace best::traits_internal

#endif  // BEST_META_TRAITS_INTERNAL_QUALS_H_
