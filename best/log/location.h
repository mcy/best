#ifndef BEST_LOG_LOCATION_H_
#define BEST_LOG_LOCATION_H_

#include <source_location>
#include <string_view>

#include "best/meta/taxonomy.h"

namespace best {
inline constexpr struct here_t final {
} here;

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
  constexpr std::string_view file() const {
    if (impl_.file_name() == nullptr) {
      return "<unknown>";
    }
    return impl_.file_name();
  }

  /// Returns the function this location refers to, if known.
  constexpr std::string_view func() const {
    if (impl_.function_name() == nullptr) {
      return "<unknown>";
    }
    return impl_.function_name();
  }

  /// Returns the line this location refers to; 1-indexed.
  constexpr uint32_t line() const { return impl_.line(); }

  /// Returns the column this location refers to; 1-indexed.
  constexpr uint32_t col() const { return impl_.column(); }

  /// Converts this into a best::location.
  constexpr track_location<void> location() const { return *this; }

  // This makes best::track_location into a smart pointer.
  constexpr std::add_lvalue_reference_t<const T> operator*()

      const
    requires(!best::is_void<T>)
  {
    return value_;
  }
  constexpr std::add_lvalue_reference_t<T> operator*()
    requires(!best::is_void<T>)
  {
    return value_;
  }
  constexpr std::add_pointer_t<const T> operator->() const&
    requires(!best::is_void<T>)
  {
    return best::addr(value_);
  }
  constexpr std::add_pointer_t<const T> operator->() &
    requires(!best::is_void<T>)
  {
    return best::addr(value_);
  }
  constexpr operator std::add_lvalue_reference_t<const T>() const {
    return value_;
  }

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

 private:
  template <typename>
  friend class track_location;

  std::conditional_t<best::is_void<T>, char, T> value_;
  std::source_location impl_;
};

/// A location, not including a tracked argument.
using location = track_location<void>;
}  // namespace best

#endif  // BEST_LOG_LOCATION_H_