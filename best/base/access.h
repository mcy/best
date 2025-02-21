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

#ifndef BEST_BASE_ACCESS_H_
#define BEST_BASE_ACCESS_H_

#include "best/base/fwd.h"

//! The access helper.
//!
//! `best::access` is an auxiliary type for allowing types to keep members that
//! implement non-FTADLE extension points private.

namespace best {
/// # `best::access`
///
/// Befriend this type to allow best easy access to key type members that you
/// may not want to make public API, such as `BestPtrMetadata`, `BestRowKey`,
/// and so on.
class access final {
 public:
  ~access() = delete;

  // Types and other members we want access to.
  template <typename T>
  static T::BestPtrMetadata _BestPtrMetadata(int);
  template <typename T>
  static void _BestPtrMetadata(...);
  template <typename T>
  using BestPtrMetadata = decltype(_BestPtrMetadata<T>(0));

  template <typename T>
  static constexpr T constructor(auto&&... args)
    requires requires { T(BEST_FWD(args)...); }
  {
    return T(BEST_FWD(args)...);
  }

  // Best types that can access them.
  friend ::best::ptr_internal::access;
  friend ::best::dyn_internal::access;
};
}  // namespace best

#endif  // BEST_BASE_ACCESS_H_
