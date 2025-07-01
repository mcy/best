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

#ifndef BEST_CONTAINER_INTERNAL_TABLE_H_
#define BEST_CONTAINER_INTERNAL_TABLE_H_

#include <emmintrin.h>
#include <stdint.h>

#include <cstring>

#include "best/base/arch.h"
#include "best/base/hint.h"
#include "best/container/option.h"
#include "best/hash/fxhash.h"
#include "best/hash/impls.h"
#include "best/log/wtf.h"
#include "best/math/bit.h"
#include "best/math/int.h"
#include "best/memory/layout.h"
#include "best/meta/traits/empty.h"

namespace best::table_internal {
template <typename table>
class iter_impl;

struct ctrl final {
  static const ctrl Empty, Tombstone, Sentinel;

  constexpr ctrl() = default;
  constexpr ctrl(int8_t bits) : bits(bits) {}

  constexpr bool is_occupied() const { return bits >= 0; }
  constexpr bool is_vacant() const { return bits < -1; }
  constexpr bool is_empty() const { return bits == Empty; }
  constexpr bool is_tombstone() const { return bits == Tombstone; }

  constexpr bool operator==(const ctrl&) const = default;

  friend void BestFmt(auto& fmt, ctrl c) {
    if (c.is_empty()) {
      fmt.write("--");
    } else if (c.is_tombstone()) {
      fmt.write("🪦");
    } else {
      fmt.format("{:02x}", best::to_unsigned(c.bits));
    }
  }

  int8_t bits;
};

inline constexpr ctrl ctrl::Empty = 0b1000'0000;
inline constexpr ctrl ctrl::Tombstone = 0b1111'1110;
inline constexpr ctrl ctrl::Sentinel = 0b1111'1111;

inline uint64_t seed(const void* array) { return (uintptr_t)array >> 12; }

// A processed hash from a table.
struct hash final {
  template <typename K, typename H>
  static hash compute(const auto& query, const auto& id, const void* salt) {
    best::hasher<H> hasher;
    id.template hash<K>(hasher, query);
    uint64_t h = hasher.finish();

    return {
      .h1 = (h >> 7) ^ (reinterpret_cast<uintptr_t>(salt) >> 12),
      .h2 = h & 0x7f,
    };
  }

  uint64_t h1;
  ctrl h2;
};

template <typename I = best::default_identity, typename H = best::fxhash,
          typename A = best::malloc>
struct policy final {
  using identity = I;
  using hash_state = H;
  using allocator = A;
};

/// A general bitset, stored in an integer.
///
/// `width` is the number of bits in the integer dedicated to each. The nth bit
/// is set when the nth run of `width` bits is not all zeros. `len` is the total
/// length of the bitset.
///
/// For example, when `width` is 1, this is an actual bitset. When `width` is
/// 8, each byte corresponds to one bit.
template <typename Int, size_t len, size_t width>
struct bitset final {
  static_assert(width * len <= best::bits_of<Int>);
  Int bits;

  bool empty() const { return bits == 0; }

  /// Lowest and highest set bit index in the set.
  uint32_t lowest() const { return best::trailing_zeros(bits) / width; }
  uint32_t highest() const { return best::bits_for(bits) / width; }

  /// The number of leading and trailing zero bits.
  uint32_t trailing_zeros() const { return lowest(); }
  uint32_t leading_zeros() const {
    auto total_bits = width * len;
    auto extra_bits = best::bits_of<Int> - total_bits;
    return best::leading_zeros(bits) / width - extra_bits;
  }

  /// Returns the next bit index set.
  BEST_INLINE_ALWAYS
  best::option<uint32_t> next() {
    if (empty()) { return best::none; }

    uint32_t idx = lowest();
    bits &= bits - 1;
    return idx;
  }
};

#if BEST_SSE2
// An implementation of the group operations using an SSE2 (xmm) vector.
class group_sse2 final {
 public:
  using mask = bitset<uint64_t, 16, 1>;

  group_sse2() = default;
  group_sse2(__m128i xmm) : xmm_(xmm){};

  static group_sse2 load(const void* array, size_t offset) {
    auto* ptr = (const char*)array + offset;

    return _mm_loadu_si128((const __m128i*)ptr);
  }

  void store(void* array, size_t offset) {
    auto* ptr = (char*)array + offset;
    _mm_storeu_si128((__m128i*)ptr, xmm_);
  }

  mask match(ctrl h2) const {
    auto broadcast = _mm_set1_epi8(h2.bits);
    auto eq = _mm_cmpeq_epi8(broadcast, xmm_);
    return {.bits = uint64_t(_mm_movemask_epi8(eq))};
  }

  mask match_empty() const {
#if BEST_SSSE3
    // This only works because Empty is 0b1000'0000, the only value whose
    // two's complement is itself. All other values produce non-negative bytes
    // when pushed through `vpsignb`.
    auto sign_bits = _mm_sign_epi8(xmm_, xmm_);
    return {.bits = uint64_t(_mm_movemask_epi8(sign_bits))};
#else
    return match(ctrl::Empty);
#endif
  }

  mask match_vacant() const {
    auto broadcast = _mm_set1_epi8(ctrl::Sentinel.bits);
    auto gt = _mm_cmpgt_epi8(broadcast, xmm_);
    return {.bits = uint64_t(_mm_movemask_epi8(gt))};
  }

  uint32_t count_leading_vacant() const {
    auto broadcast = _mm_set1_epi8(ctrl::Sentinel.bits);
    auto gt = _mm_cmpgt_epi8(broadcast, xmm_);
    return best::trailing_zeros(_mm_movemask_epi8(gt) + 1);
  }

  group_sse2 prepare_for_rehash() const {
    auto msbs = _mm_set1_epi8(0b1000'0000);
    auto x126 = _mm_set1_epi8(0b0111'1110);
#if BEST_SSSE3
    return _mm_or_si128(_mm_shuffle_epi8(x126, xmm_), msbs);
#else
    auto zero = _mm_setzero_si128();
    auto special_mask = _mm_cmpgt_epi8(zero, xmm_);
    return _mm_or_si128(msbs, _mm_andnot_si128(special_mask, x126));
#endif
  }

  friend void BestFmt(auto& fmt, group_sse2 g) {
    ctrl bytes[sizeof(group_sse2)];
    best::span(bytes).copy_from(best::span(&g, 1).as_bytes());

    bool first = true;
    for (auto byte : bytes) {
      if (!std::exchange(first, false)) { fmt.write(" "); }
      fmt.format("{:?}", byte);
    }
  }

 private:
  __m128i xmm_;
};
#endif

// An implementation of the group operations using plain uint64 operations.
class group_scalar final {
 public:
  using mask = bitset<uint64_t, 8, 8>;

  group_scalar() = default;
  group_scalar(uint64_t reg) : reg_(reg){};

  static group_scalar load(const void* array, size_t offset) {
    auto* ptr = (const char*)array + offset;
    group_scalar g;
    std::memcpy(&g, ptr, sizeof(g));
    return g;
  }

  void store(void* array, size_t offset) {
    auto* ptr = (char*)array + offset;
    std::memcpy(ptr, this, sizeof(*this));
  }

  mask match(ctrl h2) const {
    // For the technique, see:
    // http://graphics.stanford.edu/~seander/bithacks.html##ValueInWord
    // (Determine if a word has a byte equal to n).
    //
    // Caveat: there are false positives but:
    // - they only occur if there is a real match
    // - they never occur on ctrl_t::kEmpty, ctrl_t::kDeleted, ctrl_t::kSentinel
    // - they will be handled gracefully by subsequent checks in code
    //
    // Example:
    //   v = 0x1716151413121110
    //   hash = 0x12
    //   retval = (v - lsbs) & ~v & msbs = 0x0000000080800000

    auto x = reg_ ^ (Lsbs * h2.bits);
    return {.bits = (x - Lsbs) & ~x & Msbs};
  }

  mask match_empty() const { return {.bits = (reg_ & (~reg_ << 6)) & Msbs}; }

  mask match_vacant() const { return {.bits = (reg_ & (~reg_ << 7)) & Msbs}; }

  uint32_t count_leading_vacant() const {
    return best::trailing_zeros((((~reg_ & (reg_ >> 7)) | Gaps) + 1) + 7) / 8;
  }

  group_scalar prepare_for_rehash() const {
    auto x = reg_ & Msbs;
    return (~x + (x >> 7)) & ~Lsbs;
  }

  friend void BestFmt(auto& fmt, group_scalar g) {
    ctrl bytes[sizeof(group_sse2)];
    best::span(bytes).copy_from(best::span(&g, 1).as_bytes());

    bool first = true;
    for (auto byte : bytes) {
      if (!std::exchange(first, false)) { fmt.write(" "); }
      fmt.format("{}", byte);
    }
  }

 private:
  static constexpr uint64_t Msbs = 0x8080808080808080;
  static constexpr uint64_t Lsbs = 0x0101010101010101;
  static constexpr uint64_t Gaps = ~Lsbs >> 8;

  uint64_t reg_;
};

#if BEST_SSE2
using group = group_sse2;
#else
using group = group_scalar;
#endif

constexpr size_t mirror(size_t idx, size_t cloned, size_t hard) {
  // This is intentionally branchless. If `i < kWidth`, it will write to the
  // cloned bytes as well as the "real" byte; otherwise, it will store `h`
  // twice.
  //
  // We want to map all values n < sizeof(group) to mask - n. Subtracting
  // off sizeof(group) produces negative values only for the values of interest;
  // for these values, `(idx - cloned) & mask` is `mask - (cloned - idx)`. We
  // can then add (cloned & mask) to shift everything up.
  size_t mask = hard - 1;
  return ((idx - cloned) & mask) + (cloned);
}

// A quadratic probe sequence.
//
// Currently, the sequence is a triangular progression of the form
// ```
// p(i) := kWidth/2 * (i^2 - i) + hash (mod mask + 1)
// ```
BEST_INLINE_ALWAYS auto probe(auto& table, const void* ctrl, hash h,
                              size_t hard, auto cb) {
  size_t mask = hard - 1;
  size_t offset = h.h1 & mask;
  size_t index = 0;

  best::prefetch_for_read<best::prefetch_locality::L3>(ctrl);
  while (true) {
    auto g = group::load(ctrl, offset);
    auto got = cb(offset, g);
    if (best::likely(got.has_value())) { return *got; }

    index += sizeof(group);
    offset += index;
    offset &= mask;

    best::debug_must(index < hard, "full table:\n{}", table.debug(false));
  }
}

// The capacity of a table.
class capacity final {
 public:
  capacity() = default;

  // Constructs a new layout for the given minimum number of elements.
  explicit capacity(size_t at_least) {
    hard = at_least + at_least / 7;
    hard =
      best::checked_next_pow2(hard).expect("map size too large: {}", at_least);
    hard = best::max(hard, sizeof(group));
    reset_soft();
  }

  // The number of groups this in this table's ctrl array.
  size_t group_count() const { return hard / size_of<group> + 1; }

  // Returns the layout for the backing array.
  template <typename K, typename V>
  best::layout layout() const {
    if constexpr (best::is_empty<V>) {
      return best::layout::of_struct({
        best::layout::of<group>().repeat(group_count()),
        best::layout::of<K>().repeat(hard),
        best::layout::of<char>(),
      });
    } else {
      return best::layout::of_struct({
        best::layout::of<group>().repeat(group_count()),
        best::layout::of<K>().repeat(hard),
        best::layout::of<V>().repeat(hard),
      });
    }
  }

  template <typename K>
  size_t keys_offset() const {
    auto ctrl = best::layout::of<group>().repeat(group_count());
    return best::round_up_to_pow2(ctrl, align_of<K>);
  }

  template <typename K, typename V>
  size_t values_offset() const {
    auto keys = best::layout::of_struct({
      best::layout::of<group>().repeat(group_count()),
      best::layout::of<K>().repeat(hard),
    });
    return best::round_up_to_pow2(keys, align_of<V>);
  }

  void reset_soft() {
    soft = hard - hard / 8;  // soft = hard * 7/8
  }

  // hard is always a power of 2; soft is the number of slots left before a
  // rehash needs to happen.
  size_t soft{}, hard{};
};

// The heap-backed array for a table. This is essentially the whole table
// except for the size and the policy.
template <typename K, typename V>
class array final {
 public:
  array() = default;
  explicit array(best::ptr<void> base, capacity cap) : cap_(cap) {
    size_t ctrl_count = cap.group_count() * sizeof(group);

    ctrl_ = base.template cast<table_internal::ctrl>().raw();
    keys_ = ctrl_.template skip<K>(ctrl_count);
    reset_ctrl();
  }

  table_internal::ctrl ctrl(size_t n) const { return *ctrl_.offset(n); }
  void set_ctrl(size_t n, table_internal::ctrl c) {
    if (ctrl(n) == table_internal::ctrl::Empty) { --cap_.soft; }

    *ctrl_.offset(n) = c;
    *ctrl_.offset(mirror(n, sizeof(group), cap_.hard)) = c;
  }

  best::ptr<K> key(size_t idx) const { return keys_ + idx; }
  best::ptr<V> value(size_t idx) const {
    auto ptr = keys_.template skip<V>(cap_.hard);
    if constexpr (!best::is_empty<V>) { ptr += idx; }
    return ptr;
  }

  const capacity& cap() const { return cap_; }
  capacity& cap() { return cap_; }

  auto* ptr() const { return ctrl_.raw(); }

  void reset_ctrl() {
    ctrl_.fill(table_internal::ctrl::Empty.bits,
               cap_.group_count() * sizeof(group));
  }

  void destroy() {
    if (ctrl_ == nullptr) { return; }

    auto keys = keys_;
    auto values = keys.template skip<V>(cap_.hard);

    auto cur = ctrl_, end = ctrl_ + cap_.hard;
    do {
      if (cur->is_occupied()) {
        keys.destroy();
        values.destroy();
      }

      ++cur;
      ++keys;
      if constexpr (!best::is_empty<V>) { ++values; }
    } while (cur != end);
  }

  BEST_INLINE_ALWAYS size_t vacant_unchecked(hash hash) const {
    return best::table_internal::probe(  //
      *this, ptr(), hash, cap().hard,
      [&](size_t base, group g) -> best::option<size_t> {
        return g.match_vacant().next().map(
          [&](auto i) { return (base + i) & (cap().hard - 1); });
      });
  }

  BEST_INLINE_ALWAYS best::option<size_t> vacant(hash hash) const {
    if (ptr() == nullptr) { return best::none; }

    size_t idx = vacant_unchecked(hash);
    if (cap().soft == 0 && !ctrl(idx).is_tombstone()) { return best::none; }

    return idx;
  }

  best::strbuf debug(bool) const {
    best::strbuf out =
      best::format("type: {}\narray: {:p}, {}/{}\n",
                   best::type_names::of<array>.path_with_params(), ptr(),
                   cap().soft, cap().hard);

    if (ptr() == nullptr) { return out; }

    out.push("ctrl:");
    for (auto i : best::bounds{.count = cap().group_count()}) {
      i *= sizeof(group);
      best::format(out, "\n  {:p}: {:?}", ptr() + i, group::load(ptr(), i));
    }
    out.push(" (mirrored)\n");
    return out;
  }

 private:
  best::ptr<table_internal::ctrl> ctrl_;
  best::ptr<K> keys_;
  capacity cap_;
};

}  // namespace best::table_internal

#endif  // BEST_CONTAINER_INTERNAL_TABLE_H_