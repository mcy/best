#ifndef BEST_CONTAINER_INTERNAL_SIMPLE_OPTION_H_
#define BEST_CONTAINER_INTERNAL_SIMPLE_OPTION_H_

#include <cstddef>
#include <memory>

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
  constexpr option(T value) : value_(std::move(value)), has_(true) {}

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

  constexpr const T* operator->() const { return std::addressof(value_); }
  constexpr T* operator->() { return std::addressof(value_); }

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