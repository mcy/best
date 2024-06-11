#ifndef BEST_MEMORY_LAYOUT_H_
#define BEST_MEMORY_LAYOUT_H_

#include <cstddef>
#include <cstdlib>

#include "best/math/int.h"
#include "best/math/overflow.h"
#include "best/memory/internal/layout.h"

namespace best {
/// The number of bytes required to store a T.
///
/// Unlike sizeof, this computes the size of a best::object<T>, so in particular
/// size_of<T&> is sizeof(void*), not sizeof(T).
template <typename T>
inline constexpr size_t size_of = best::layout_internal::size_of<T>;

/// The address alignment required to store a T.
///
/// Unlike alignof, this computes the size of a best::object<T>, so in
/// particular align_of<T&> is alignof(void*), not alignof(T).
template <typename T>
inline constexpr size_t align_of = best::layout_internal::align_of<T>;

/// The gross layout of some C++ type in memory.
///
/// This tracks both the size and alignment of some type.
struct layout final {
  /// Returns the layout for T.
  ///
  /// Technically, this is the layout for best::object<T>, which coincides
  /// with that of T if T is an object type.
  template <typename T>
  constexpr static layout of() {
    return layout(best::unsafe, size_of<T>, align_of<T>);
  }

  /// Returns the layout for an array of type T of the given length.
  ///
  /// As above, non-object Ts are replaced with best::object<T>.
  ///
  /// Crashes on overflow.
  template <typename T>
  constexpr static layout array(size_t n) {
    auto single = of<T>();

    auto [sz, of] = best::overflow(best::size_of<T>) * n;
    if (of || sz > best::max_of<size_t>) {
      best::crash_internal::crash(
          "attempted to allocate more than max_of<size_t>/2 bytes");
    }

    return layout(best::unsafe, sz, single.align());
  }

  /// Returns the layout of a struct with the given members.
  ///
  /// This implement the C/C++ standard layout class sizing algorithm. Zero
  /// types produces the layout of char.
  template <typename... Members>
  constexpr static layout of_struct() {
    return layout(best::unsafe, best::layout_internal::size_of<Members...>,
                  best::layout_internal::align_of<Members...>);
  }

  /// Returns the layout of a union with the given members.
  ///
  /// In other words, this computes the maximum size among the member types,
  /// rounded to the alignment of the most-aligned type. Zero types produces
  /// the layout of char.
  template <typename... Members>
  constexpr static layout of_union() {
    return layout(best::unsafe,
                  best::layout_internal::size_of_union<Members...>,
                  best::layout_internal::align_of<Members...>);
  }

  /// Returns the trivial layout (one byte, one align).
  constexpr layout() = default;

  /// Constructs a new layout from arbitrary size and alignment.
  ///
  /// This must observe two critical requirements: size % align == 0, and
  /// best::is_pow2(align).
  constexpr explicit layout(best::unsafe_t, size_t size, size_t align)
      : size_(size), align_(align) {}

  /// The size, in bytes.
  ///
  /// This is always divisible by `align`.
  constexpr size_t size() const { return size_; }

  /// The alignment, in bytes.
  ///
  /// This is always a power of 2.
  constexpr size_t align() const { return align_; };

 private:
  size_t size_ = 1;
  size_t align_ = 1;
};
}  // namespace best

#endif  // BEST_MEMORY_LAYOUT_H_