#ifndef BEST_CONTAINER_INTERNAL_BAG_H_
#define BEST_CONTAINER_INTERNAL_BAG_H_

#include "best/container/object.h"
#include "best/meta/ebo.h"

namespace best::bag_internal {
template <typename, typename...>
struct impl;

template <size_t... i, typename... Elems>
struct impl<const best::vlist<i...>, Elems...>
    : best::ebo<best::object<Elems>, Elems, i>... {};
}  // namespace best::bag_internal

#endif  // BEST_CONTAINER_INTERNAL_BAG_H_