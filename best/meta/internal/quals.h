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

#ifndef BEST_META_INTERNAL_QUALS_H_
#define BEST_META_INTERNAL_QUALS_H_

#include <type_traits>

namespace best::quals_internal {
template <typename Dst, typename Src>
struct quals {
  using copied = Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, const Src> {
  using copied = const Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, volatile Src> {
  using copied = volatile Dst;
};
template <typename Dst, typename Src>
struct quals<Dst, const volatile Src> {
  using copied = const volatile Dst;
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
}  // namespace best::quals_internal

#endif  // BEST_META_INTERNAL_QUALS_H_
