#include "best/math/int.h"

#include "best/container/option.h"  // Needed for best::checked_cast.

namespace best::int_test {
static_assert(best::integer<int8_t>);
static_assert(best::integer<int16_t>);
static_assert(best::integer<int32_t>);
static_assert(best::integer<int64_t>);
static_assert(best::integer<uint8_t>);
static_assert(best::integer<uint16_t>);
static_assert(best::integer<uint32_t>);
static_assert(best::integer<uint64_t>);

static_assert(!best::integer<bool>);
static_assert(!best::integer<wchar_t>);
static_assert(!best::integer<char16_t>);
static_assert(!best::integer<char32_t>);

static_assert(best::to_unsigned(-1) == ~0);
static_assert(best::to_signed(~0u) == -1);

static_assert(best::min_of<int8_t> == -0x80);
static_assert(best::max_of<int8_t> == 0x7f);
static_assert(best::min_of<uint8_t> == 0x00);
static_assert(best::max_of<uint8_t> == 0xff);
static_assert(best::min_of<int32_t> == -0x8000'0000);
static_assert(best::max_of<int32_t> == 0x7fff'ffff);
static_assert(best::min_of<uint32_t> == 0x0000'0000);
static_assert(best::max_of<uint32_t> == 0xffff'ffff);

static_assert(best::bits_of<int8_t> == 8);
static_assert(best::bits_of<int16_t> == 16);
static_assert(best::bits_of<int32_t> == 32);
static_assert(best::bits_of<int64_t> == 64);
static_assert(best::bits_of<uint8_t> == 8);
static_assert(best::bits_of<uint16_t> == 16);
static_assert(best::bits_of<uint32_t> == 32);
static_assert(best::bits_of<uint64_t> == 64);

static_assert(best::same<best::common_int<short, signed, long>, long>);
static_assert(
    best::same<best::common_int<short, unsigned, long>, unsigned long>);

static_assert(best::uint_cmp(-1, 1) > 0);
static_assert(best::sint_cmp(~0u, 1) < 0);
static_assert(best::int_cmp(-1, 1u) < 0);
static_assert(best::int_cmp(-1u, 1) > 0);

static_assert(best::int_cmp(1, min_of<unsigned>) > 0);
static_assert(best::int_cmp(max_of<unsigned>, 1) > 0);

static_assert(best::checked_cast<unsigned>(-1).is_empty());
static_assert(best::checked_cast<unsigned>(1) == 1);
static_assert(best::checked_cast<int>(best::max_of<long>).is_empty());
static_assert(best::checked_cast<int>(200ll) == 200);
}  // namespace best::int_test