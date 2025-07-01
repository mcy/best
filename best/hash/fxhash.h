
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

#ifndef BEST_HASH_FXHASH_H_
#define BEST_HASH_FXHASH_H_

#include <cstring>

#include "best/base/fwd.h"
#include "best/math/bit.h"
#include "best/math/int.h"
#include "best/math/overflow.h"
#include "best/memory/span.h"

namespace best {
/// # `best::fxhash`
///
/// An implementation of the "rustc hash", a variant of fxhash, a multiplicative
/// hash originally developed for FireFox.
///
/// See https://github.com/rust-lang/rustc-hash.
class fxhash final {
 public:
  using output = uint64_t;

  constexpr fxhash() : state_(Seed) {}
  explicit constexpr fxhash(uint64_t seed) : state_(seed) {}

  template <best::is_int Int>
  constexpr void write(Int value) {
    write(static_cast<uint64_t>(value));
  }

  constexpr void write(uint64_t n) { state_ = (state_ + n) * K; }

  constexpr void write(best::span<const char> bytes) {
    auto n = bytes.size();
    uint64_t x0 = Y0, x1 = Y1;  // Digits of pi.

    auto p = bytes.data().raw();
    if (n <= 16) {
      if (n >= 8) {
        x0 ^= load64(p);
        x1 ^= load64(p + (n - 8));
      } else if (n >= 4) {
        x0 ^= load32(p);
        x1 ^= load32(p + (n - 4));
      } else if (n > 0) {
        x0 ^= p[0];
        x1 ^= p[n / 2];
        x1 ^= uint64_t(p[n - 1]) << 8;
      }
    } else {
      auto end = p + (n - 16);
      while (p < end) {
        auto x = load64(p);
        auto y = load64(p + 8);
        auto t = best::mul(x0 ^ x, Y2 ^ y).mix();
        x0 = x1;
        x1 = t;
        p += 16;
      }

      x0 ^= load64(end);
      x1 ^= load64(end + 8);
    }

    write(best::mul(x0, x1).mix() ^ n);
  }

  constexpr output finish() const { return best::rotate_left(state_, 26); }

 private:
  // A non-deterministic seed, which will vary per-process due to ASLR, but will
  // at best vary per-build otherwise.
  static const uint64_t Seed;

  static constexpr uint64_t K = 0xf1357aea2e62a9c5;

  // Digits of pi.
  static constexpr uint64_t Y0 = 0x243f6a8885a308d3;
  static constexpr uint64_t Y1 = 0x13198a2e03707344;
  static constexpr uint64_t Y2 = 0xa4093822299f31d0;

  constexpr uint32_t load32(const char* p) {
    uint32_t value = 0;
    if (std::is_constant_evaluated()) {
      for (auto i = 0; i < 4; i++) { value |= uint32_t(p[i]) << (i * 8); }
    } else {
      memcpy(&value, p, 4);
    }
    return value;
  }

  constexpr uint64_t load64(const char* p) {
    uint64_t value = 0;
    if (std::is_constant_evaluated()) {
      for (auto i = 0; i < 8; i++) { value |= uint64_t(p[i]) << (i * 8); }
    } else {
      memcpy(&value, p, 8);
    }
    return value;
  }

  uint64_t state_ = 0;
};

inline const uint64_t fxhash::Seed = reinterpret_cast<uintptr_t>(&Seed);
}  // namespace best

#endif  // BEST_HASH_FXHASH_H_