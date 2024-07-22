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

#ifndef BEST_LOG_LOCATION_H_
#define BEST_LOG_LOCATION_H_

#include <concepts>
#include <source_location>

#include "best/base/fwd.h"
#include "best/meta/taxonomy.h"
#include "best/meta/traits.h"

namespace best {
/// # `best::here`
///
/// The current `best::location`.
inline constexpr struct here_t final {
} here;

/// # `best::location`
///
/// A location, not including a tracked argument.
using location = best::track_location<void>;

/// # `best::track_location<T>`.
///
/// A tracked function argument.
template <typename T>
class track_location {
 public:
  /// Constructs a new location referring to the current context from
  /// best::here.
  constexpr track_location(
    here_t, std::source_location loc = std::source_location::current())
    requires best::is_void<T>
    : value_{}, impl_(loc) {}
  template <typename Arg = T>
  constexpr track_location(
    Arg&& arg, std::source_location loc = std::source_location::current())
    requires std::convertible_to<Arg, T>
    : value_(BEST_FWD(arg)), impl_(loc) {}

  /// Constructs a new location from the given location.
  template <typename U>
  constexpr track_location(best::track_location<U> loc)
    requires best::is_void<T>
    : value_{}, impl_(loc.impl_) {}
  template <typename Arg = T, typename U>
  constexpr track_location(Arg&& arg, best::track_location<U> loc)
    requires std::convertible_to<Arg, T>
    : value_(BEST_FWD(arg)), impl_(loc.impl_) {}

  /// Returns the file this location refers to.
  template <int&...,  // Delayed, since this header needs to
                      // appear extremely early.
            typename utf8 = best::utf8>
  constexpr best::text<utf8> file() const {
    if (impl_.file_name() == nullptr) { return "<unknown>"; }
    return *best::text<utf8>::from_nul(impl_.file_name());
  }

  /// Returns the function this location refers to, if known.
  template <int&...,  // Delayed, since this header needs to
                      // appear extremely early.
            typename utf8 = best::utf8>
  constexpr best::text<utf8> func() const {
    if (impl_.function_name() == nullptr) { return "<unknown>"; }
    return *best::text<utf8>::from_nul(impl_.function_name());
  }

  /// Returns the line this location refers to; 1-indexed.
  constexpr uint32_t line() const { return impl_.line(); }

  /// Returns the column this location refers to; 1-indexed.
  constexpr uint32_t col() const { return impl_.column(); }

  /// Converts this into a best::location.
  constexpr track_location<void> location() const { return *this; }

  // This makes best::track_location into a smart pointer.
  constexpr best::as_ref<const T> operator*() const requires (!best::is_void<T>)
  {
    return value_;
  }
  constexpr best::as_ref<T> operator*() requires (!best::is_void<T>)
  {
    return value_;
  }
  constexpr best::as_ptr<const T> operator->() const& requires (
    !best::is_void<T>)
  {
    return best::addr(value_);
  }
  constexpr best::as_ptr<const T> operator->() & requires (!best::is_void<T>)
  {
    return best::addr(value_);
  }
  constexpr operator best::as_ref<const T>() const { return value_; }

  constexpr track_location(const track_location&) = default;
  constexpr track_location& operator=(const track_location&) = default;
  constexpr track_location(track_location&&) = default;
  constexpr track_location& operator=(track_location&) = default;

  friend void BestFmt(auto& fmt, const track_location& loc)
    requires best::is_void<T> || requires { fmt.format(*loc); }
  {
    if constexpr (!best::is_void<T>) {
      fmt.format(*loc);
      fmt.write(" @ ");
    }
    fmt.format("{}:{}", loc.file(), loc.line());
  }

  constexpr auto impl() const { return impl_; }

 private:
  template <typename>
  friend class track_location;

  best::select<best::is_void<T>, char, T> value_;
  std::source_location impl_;
};

}  // namespace best

#endif  // BEST_LOG_LOCATION_H_
