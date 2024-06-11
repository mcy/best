#include "best/math/bit.h"

#include "best/math/int.h"

namespace best::bit_test {
static_assert(best::count_zeros(0) == best::bits_of<int>);
static_assert(best::count_ones(0) == 0);
static_assert(best::count_zeros(best::max_of<int>) == 1);
static_assert(best::count_ones(best::max_of<int>) == best::bits_of<int> - 1);

static_assert(best::leading_zeros(0) == best::bits_of<int>);
static_assert(best::leading_zeros(-1) == 0);
static_assert(best::leading_zeros(-1u << 1) == 0);
static_assert(best::leading_zeros(-1u >> 1) == 1);

static_assert(best::leading_ones(0) == 0);
static_assert(best::leading_ones(-1) == best::bits_of<int>);
static_assert(best::leading_ones(-1u << 1) == best::bits_of<int> - 1);
static_assert(best::leading_ones(-1u >> 1) == 0);

static_assert(best::trailing_zeros(0) == best::bits_of<int>);
static_assert(best::trailing_zeros(-1) == 0);
static_assert(best::trailing_zeros(-1u << 1) == 1);
static_assert(best::trailing_zeros(-1u >> 1) == 0);

static_assert(best::trailing_ones(0) == 0);
static_assert(best::trailing_ones(-1) == best::bits_of<int>);
static_assert(best::trailing_ones(-1u << 1) == 0);
static_assert(best::trailing_ones(-1u >> 1) == best::bits_of<int> - 1);

// Note: result is independent of the sign of the -1.
static_assert(best::shift_left(-1, 1) == -2);
static_assert(best::shift_right(-1, 1) == best::max_of<int>);
static_assert(best::shift_sign(-1, 1) == -1);
static_assert(best::shift_left(-1u, 1) == -2);
static_assert(best::shift_right(-1u, 1) == best::max_of<int>);
static_assert(best::shift_sign(-1u, 1) == -1);

static_assert(best::rotate_left(0xa0a0'a0a0, 3) == 0x0505'0505);
static_assert(best::rotate_right(0xa0a0'a0a0, 5) == 0x0505'0505);
static_assert(best::rotate_left(0xa0a0'a0a0, 3 + best::bits_of<int>) ==
              0x0505'0505);
static_assert(best::rotate_right(0xa0a0'a0a0, 5 + best::bits_of<int>) ==
              0x0505'0505);

static_assert(best::is_pow2(1u));
static_assert(best::is_pow2(2u));
static_assert(best::is_pow2(1024u));
static_assert(best::is_pow2(best::to_unsigned(best::min_of<int>)));

static_assert(!best::is_pow2(0u));
static_assert(!best::is_pow2(3u));
static_assert(!best::is_pow2(best::max_of<unsigned>));

static_assert(best::next_pow2(0u) == 1);
static_assert(best::next_pow2(1u) == 2);
static_assert(best::next_pow2(2u) == 4);
static_assert(best::next_pow2(3u) == 4);
static_assert(best::next_pow2(4u) == 8);
static_assert(best::wrapping_next_pow2(best::max_of<unsigned>) == 0);

static_assert(best::bits_for(0u) == 0);
static_assert(best::bits_for(1u) == 1);
static_assert(best::bits_for(2u) == 2);
static_assert(best::bits_for(3u) == 2);
static_assert(best::bits_for(23u) == 5);
static_assert(best::bits_for(127u) == 7);
static_assert(best::bits_for(128u) == 8);
}  // namespace best::bit_test