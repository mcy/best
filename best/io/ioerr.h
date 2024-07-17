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

#ifndef BEST_IO_IOERR_H_
#define BEST_IO_IOERR_H_

#include <cerrno>

#include "best/log/wtf.h"

namespace best {
/// # `best::ioresult`
///
/// A `best::result` with the very common error type of `best::ioerr`.
template <typename T = void>
using ioresult = best::result<T, best::ioerr>;

/// # `best::ioerr`
///
/// Wraps a non-zero `errno` value, likely resulting from some I/O operation.
class ioerr final {
 public:
  ioerr() = delete;
  constexpr ioerr(const ioerr&) = default;
  constexpr ioerr& operator=(const ioerr&) = default;
  constexpr ioerr(ioerr&&) = default;
  constexpr ioerr& operator=(ioerr&&) = default;

  /// # `ioerr::ioerr()`
  ///
  /// Constructs a new `best::ioerr` wrapping the given value, which must be
  /// positive. Passing a nonpositive value to this constructor will crash.
  constexpr ioerr(int value) : value_(value) {
    if (value > 0) { return; }
    best::wtf("best::ioerr must be positive: got {}", value);
  }

  /// # `ioerr::current()`
  ///
  /// Reads the `errno` value for this thread and wraps it in a best::ioresult.
  static best::ioresult<> current() {
    int value = errno;
    if (value == 0) {
      return best::ok();
    }
    return best::ioerr(value);
  }

  /// # `ioerr::raw()`
  ///
  /// Returns the underlying raw integer.
  constexpr int raw() const { return value_; }
  constexpr operator int() const { return value_; }

  /// # `ioerr::name()`, `ioerr::message()`
  ///
  /// Returns the name and message of this errno value. If this errno value is
  /// not known to `best`, returns `best::none`.
  best::option<best::str> name() const;
  best::option<best::str> message() const;

  friend void BestFmt(auto& fmt, const ioerr& e) {
    if (e.name().is_empty()) {
      fmt.format("Error {}: <unknown error>", e.raw());
      return;
    }
    fmt.format("Error {}: ({}), {}", e.raw(), *e.name(), *e.message());
  }
  friend void BestFmtQuery(auto& query, ioerr*) {
    query.requires_debug = false;
  }

  constexpr ioerr(niche) : value_(0) {}
  constexpr bool operator==(niche) { return value_ == 0; }

 private:
  int value_;
};

}  // namespace best

#endif  // BEST_IO_IOERR_H_
