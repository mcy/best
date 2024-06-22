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

#ifndef BEST_CONTAINER_INTERNAL_SIMPLE_OPTION_H_
#define BEST_CONTAINER_INTERNAL_SIMPLE_OPTION_H_

#include <cstddef>

#include "best/meta/taxonomy.h"

namespace best::container_internal {
/// A simple implementation of best::option that does not use the complex
/// best::choice machinery. Intended for use by things that cannot depend
/// on best::option for dependency-cycle reasons.
///
/// Interconverts with best::option in the obvious way.
template <typename T>
class option final {
 public:
  constexpr option() = default;
  constexpr option(T value) : value_(BEST_MOVE(value)), has_(true) {}

  constexpr option(const option&) = default;
  constexpr option& operator=(const option&) = default;
  constexpr option(option&&) = default;
  constexpr option& operator=(option&&) = default;

  constexpr bool has_value() const { return has_; }
  constexpr explicit operator bool() const { return has_; }

  constexpr const T& value() const { return value_; }
  constexpr T& value() { return value_; }
  constexpr const T& operator*() const { return value_; }
  constexpr T& operator*() { return value_; }

  constexpr const T* operator->() const { return best::addr(value_); }
  constexpr T* operator->() { return best::addr(value_); }

  constexpr bool operator==(const option& that) const {
    return has_ == that.has_ && value_ == that.value_;
  }
  constexpr bool operator==(const T& that) const {
    return has_ && value_ == that;
  }

  // TODO: BestFmt
  template <typename Os>
  friend Os& operator<<(Os& os, option opt) {
    if (!opt.has_value()) {
      return os << "none";
    } else {
      return os << "option(" << *opt << ")";
    }
  }

  T value_{};
  bool has_ = false;
};
}  // namespace best::container_internal

#endif  // BEST_CONTAINER_INTERNAL_SIMPLE_OPTION_H_
