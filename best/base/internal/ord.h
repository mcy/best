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

#ifndef BEST_BASE_INTERNAL_ORD_H_
#define BEST_BASE_INTERNAL_ORD_H_

#include "best/func/call.h"
#include "best/meta/taxonomy.h"

namespace best::ord_internal {
struct le {
  operator std::strong_ordering() const { return std::strong_ordering::less; }
  operator std::partial_ordering() const { return std::partial_ordering::less; }
};
struct eq {
  operator std::strong_ordering() const {
    return std::strong_ordering::equivalent;
  }
  operator std::partial_ordering() const {
    return std::partial_ordering::equivalent;
  }
};
struct gt {
  operator std::strong_ordering() const {
    return std::strong_ordering::greater;
  }
  operator std::partial_ordering() const {
    return std::partial_ordering::greater;
  }
};
struct uo {
  operator std::partial_ordering() const {
    return std::partial_ordering::unordered;
  }
};

using sord = std::strong_ordering;
using pord = std::partial_ordering;
using word = std::weak_ordering;

sord common_ord();

template <typename T>
T common_ord(T);
template <typename T>
auto common_ord(T, T, auto... rest)  //
  -> decltype(common_ord(best::lie<T>, rest...));

sord common_ord(eq);
sord common_ord(le);
sord common_ord(gt);
pord common_ord(uo);

sord common_ord(eq, eq);
sord common_ord(eq, le);
sord common_ord(eq, gt);
pord common_ord(eq, uo);
sord common_ord(le, eq);
sord common_ord(le, le);
sord common_ord(le, gt);
pord common_ord(le, uo);
sord common_ord(gt, eq);
sord common_ord(gt, le);
sord common_ord(gt, gt);
pord common_ord(gt, uo);

pord common_ord(uo, eq);
pord common_ord(uo, le);
pord common_ord(uo, gt);
pord common_ord(uo, uo);

auto common_ord(sord, eq, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(sord, le, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(sord, gt, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(eq, sord, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(le, sord, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(gt, sord, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));

auto common_ord(word, eq, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(word, le, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(word, gt, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(eq, word, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(le, word, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(gt, word, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));

auto common_ord(pord, eq, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(pord, le, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(pord, gt, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(eq, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(le, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(gt, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));

auto common_ord(uo, sord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(uo, word, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(uo, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(sord, uo, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(word, uo, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(pord, uo, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));

auto common_ord(sord, sord, auto... rest)  //
  -> decltype(common_ord(sord::equivalent, rest...));
auto common_ord(sord, word, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(sord, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));

auto common_ord(word, sord, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(word, word, auto... rest)  //
  -> decltype(common_ord(word::equivalent, rest...));
auto common_ord(word, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));

auto common_ord(pord, sord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(pord, word, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));
auto common_ord(pord, pord, auto... rest)  //
  -> decltype(common_ord(pord::equivalent, rest...));

auto common_impl(best::rank<1>, auto... args) -> decltype(common_ord(args...));
void common_impl(best::rank<0>, auto...);

template <typename... Ts>
using common = decltype(common_impl(best::rank<1>{}, best::lie<Ts>...));

template <typename Cb>
class chain final {
 public:
  constexpr explicit chain(Cb cb) : cb_(BEST_FWD(cb)) {}

  constexpr friend auto operator->*(auto lhs, chain&& rhs) {
    using output =
      common<decltype(lhs), decltype(best::call(BEST_FWD(rhs).cb_))>;

    if (!(lhs == 0)) { return output(lhs); }
    return output(best::call(BEST_FWD(rhs).cb_));
  }

 private:
  Cb cb_;
};
}  // namespace best::ord_internal

#endif  // BEST_BASE_INTERNAL_ORD_H_
