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

#ifndef BEST_MEMORY_LAYOUT_H_
#define BEST_MEMORY_LAYOUT_H_

#include <cstddef>
#include <cstdlib>

#include "best/base/unsafe.h"
#include "best/math/int.h"
#include "best/math/overflow.h"
#include "best/memory/internal/layout.h"

namespace best {
/// # `best::size_of<T>`
///
/// The number of bytes required to store a `T`.
///
/// Unlike the built-in `sizeof`, this computes the size of a `best::object<T>`,
/// so in particular `size_of<T&>` is `sizeof(void*)`, not `sizeof(T)`.
template <typename T>
inline constexpr size_t size_of = best::layout_internal::size_of<T>;

/// # `best::align_of<T>`
///
/// The address alignment required to store a `T`. This is always a power of 2.
///
/// Unlike the built-in `alignof`, this computes the size of a
/// `best::object<T>`, so in particular `align_of<T&>` is `alignof(void*)`, not
/// `alignof(T)`.
template <typename T>
inline constexpr size_t align_of = best::layout_internal::align_of<T>;

/// # `best::layout`
///
/// The gross layout of some C++ type in memory. This tracks both the size and
/// alignment of some type.
class layout final {
 public:
  /// # `layout::layout()`.
  ///
  /// The trivial layout (one byte, one align).
  constexpr layout() = default;

  /// # `layout::layout(unsafe, size, align)`.
  ///
  /// Constructs a new layout from arbitrary size and alignment.
  ///
  /// This must observe two critical requirements: `size % align == 0`, and
  /// `best::is_pow2(align)`.
  constexpr explicit layout(unsafe, size_t size, size_t align)
      : size_(size), align_(align) {}

  /// # `layout::of<T>()`
  ///
  /// Returns the layout for `T`.
  ///
  /// Technically, this is the layout for `best::object<T>`, which coincides
  /// with that of `T` if `T` is an object type.
  template <typename T>
  constexpr static layout of() {
    return layout(unsafe("manifest from calling size_of and align_of"),
                  size_of<T>, align_of<T>);
  }

  /// # `layout::array<T>()`
  ///
  /// Returns the layout for an array of type `T` of the given length. As above,
  /// non-object `T`s are replaced with `best::object<T>`.
  ///
  /// Crashes on overflow.
  template <typename T>
  constexpr static layout array(size_t n) {
    auto [sz, of] = best::overflow(size_of<T>) * n;
    if (of || sz > best::max_of<size_t>) {
      best::crash_internal::crash(
          "attempted to allocate more than max_of<size_t>/2 bytes");
    }

    return layout(unsafe("manifest from the bounds check above and align_of"),
                  sz, align_of<T>);
  }

  /// # `layout::of_struct<...>()`
  ///
  /// Returns the layout of a `struct` with the given members.
  ///
  /// This implement the C/C++ standard layout class sizing algorithm. Zero
  /// types produces the layout of `char`.
  template <typename... Members>
  constexpr static layout of_struct() {
    return layout(unsafe("manifest from calling size_of and align_of"),
                  layout_internal::size_of<Members...>,
                  layout_internal::align_of<Members...>);
  }

  /// # `layout::of_union<...>()`
  ///
  /// Returns the layout of a `union` with the given members.
  ///
  /// In other words, this computes the maximum size among the member types,
  /// rounded to the alignment of the most-aligned type. Zero types produces
  /// the layout of `char`.
  template <typename... Members>
  constexpr static layout of_union() {
    return layout(unsafe("manifest from calling size_of and align_of"),
                  layout_internal::size_of_union<Members...>,
                  layout_internal::align_of<Members...>);
  }

  /// # `layout::size()`.
  ///
  /// The size, in bytes. This is always divisible by `align`.
  constexpr size_t size() const { return size_; }

  /// # `layout::align()`.
  ///
  /// The alignment requirement, in bytes.  This is always a power of 2.
  constexpr size_t align() const { return align_; };

  friend void BestFmt(auto& fmt, layout ly) {
    auto rec = fmt.record();
    rec.field("size", ly.size());
    rec.field("align", ly.align());
  }

 private:
  size_t size_ = 1, align_ = 1;
};
}  // namespace best

#endif  // BEST_MEMORY_LAYOUT_H_
