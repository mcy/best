#ifndef BEST_CONTAINER_INTERNAL_ROW_H_
#define BEST_CONTAINER_INTERNAL_ROW_H_

#include "best/container/object.h"
#include "best/meta/empty.h"
#include "best/meta/tlist.h"

namespace best::row_internal {
template <typename, typename...>
struct impl;

template <size_t... i, typename... Elems>
struct impl<const best::vlist<i...>, Elems...>
    : best::ebo<best::object<Elems>, Elems, i>... {};
}  // namespace best::row_internal

#endif  // BEST_CONTAINER_INTERNAL_ROW_H_