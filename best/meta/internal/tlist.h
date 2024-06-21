#ifndef BEST_META_INTERNAL_TLIST_H_
#define BEST_META_INTERNAL_TLIST_H_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/container/bounds.h"

#if !__has_builtin(__type_pack_element)
#include <tuple>
#endif

//! Type traits for working with parameter packs.

namespace best::tlist_internal {
struct secret;
struct strict {};

template <typename T>
struct curry_same {
  template <typename U>
  struct trait : std::is_same<T, U> {};
};

template <size_t n, typename Default, typename... Pack>
auto nth_impl()
  requires(!std::is_same_v<Default, strict> || n < sizeof...(Pack))
{
  if constexpr (n < sizeof...(Pack)) {
#if BEST_HAS_BUILTIN(__type_pack_element)
    return std::type_identity<__type_pack_element<n, Pack...>>{};
#else
    return std::tuple_type_t<i, std::tuple<std::type_identity<Ts>...>>{};
#endif
  } else {
    return std::type_identity<Default>{};
  }
}

template <size_t n, typename Default, typename... Pack>
using nth = decltype(nth_impl<n, Default, Pack...>())::type;

template <size_t start, size_t... count, typename... Pack>
tlist<
#if BEST_HAS_BUILTIN(__type_pack_element)
    __type_pack_element<start + count, Pack...>
#else
    std::tuple_type_t<start + count, std::tuple<Ts...>>
#endif
    ...>
    type_pack_slice(std::index_sequence<count...>, tlist<Pack...>);

template <best::bounds bounds, typename Default, typename Tag,
          auto count = bounds.try_compute_count(Tag::size())>
auto slice_impl()
  requires(!std::is_same_v<Default, strict> || count.has_value())
{
  if constexpr (!count) {
    return std::type_identity<Default>{};
  } else {
    return type_pack_slice<bounds.start>(std::make_index_sequence<*count>(),
                                         Tag{});
  }
}

template <best::bounds bounds, typename Default, typename Tag>
using slice = decltype(tlist_internal::slice_impl<bounds, Default, Tag>());

template <typename... Buffer>
constexpr tlist<Buffer...> concat() {
  return {};
}
template <typename... Buffer, int&..., typename... Those>
constexpr auto concat(tlist<Those...>, auto... rest) {
  return concat<Buffer..., Those...>(rest...);
}

template <typename T>
struct entry {};

struct fail : std::false_type {
  constexpr auto operator+(auto) { return fail{}; }
};

template <typename... Ts>
struct set final : std::true_type, entry<Ts>... {
  template <typename T>
  constexpr auto operator+(entry<T>) {
    if constexpr (std::is_base_of_v<entry<T>, set>) {
      return fail{};
    } else {
      return set<Ts..., T>{};
    }
  }
};

template <typename... Ts>
concept uniq = (set<>{} + ... + entry<Ts>{}).value;
}  // namespace best::tlist_internal

#endif  // BEST_META_INTERNAL_TLIST_H_