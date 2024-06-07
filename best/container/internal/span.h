#ifndef BEST_CONTAINER_INTERNAL_SPAN_H_
#define BEST_CONTAINER_INTERNAL_SPAN_H_

#include <cstddef>

#include "best/container/object.h"
#include "best/container/option.h"

namespace best::span_internal {

template <typename T, best::option<size_t> n>
struct repr {
  best::object_ptr<T> data = nullptr;
  static constexpr size_t size = *n;
};

template <typename T>
struct repr<T, best::none> {
  best::object_ptr<T> data = nullptr;
  size_t size = 0;
};

}  // namespace best::span_internal

#endif  // BEST_CONTAINER_INTERNAL_SPAN_H_