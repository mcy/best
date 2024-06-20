#include "best/meta/tlist.h"

#include <type_traits>

#include "best/meta/ops.h"

namespace best::tlist_test {

static_assert(std::is_trivial_v<best::tlist<>>);

constexpr auto empty = best::types<>;
static_assert(empty == empty);
static_assert(empty != best::types<int>);
static_assert(empty <= empty);
static_assert(empty >= empty);

static_assert(empty.is_empty());
static_assert(empty.map<[]<typename> {}>() == empty);
static_assert(empty.apply([]<typename... Ts> { return sizeof...(Ts); }) == 0);
static_assert(decltype(empty)::type<0, best::tlist<int>>{} == best::types<int>);
static_assert(best::types<> < best::types<int>);

constexpr auto two = best::types<int, long>;
static_assert(best::types<int> < two);
static_assert(!(two < best::types<int>));
static_assert(!(best::types<long> < two));

static_assert(!two.is_empty());
static_assert(two.map<[]<typename T>() -> T* { return nullptr; }>()
                  .map<extract_trait>() == best::types<int*, long*>);
static_assert(best::types<decltype(two)::type<0>> == best::types<int>);
static_assert(best::types<decltype(two)::type<1>> == best::types<long>);

static_assert(two.map<std::is_integral>().all());
static_assert(two.map<std::is_integral>().reduce<best::op::Add>() == 2);

static_assert(best::vals<1, 2, 3, 4>.at<bounds{.start = 1, .count = 2}>() ==
              best::vals<2, 3>);

static_assert(best::types<int*, int, void*>.find<std::is_integral>() == 1);
static_assert(
    !best::types<int*, int, void*>.find<std::is_floating_point>().has_value());
static_assert(best::types<int*, int, void*>.find<void*>() == 2);
static_assert(best::types<int*, int, int>.find<std::is_integral>() == 1);

static_assert(best::vals<1, 2, 3>.find([](auto x) { return x % 2 == 0; }) == 1);
static_assert(best::vals<1, 2, 3>.find(3) == 2);
static_assert(best::vals<1, 3, 3>.find(3) == 1);
static_assert(!best::vals<1, 2, 3>.find(4).has_value());

static_assert(best::vals<1, 3, 3>.find_unique(1) == 0);
static_assert(best::vals<1, 2, 3>.find_unique(3) == 2);
static_assert(!best::vals<1, 3, 3>.find_unique(3).has_value());
static_assert(!best::vals<1, 3, 3>.find_unique(4).has_value());

static_assert(best::vals<1, 2, 3>.concat(best::vals<4, 5, 6>) ==
              best::vals<1, 2, 3, 4, 5, 6>);

}  // namespace best::tlist_test

int main() {}