
#include "best/container/bounds.h"

#include "best/log/internal/crash.h"

namespace best {
[[noreturn]] void bounds::crash(size_t len, best::location loc) const {
  if (count != 1 && start > len) {
    crash_internal::crash({"bounds-check failed: %zu (start) > %zu (len)", loc},
                          start, len);
  }

  if (count == 1 && start >= len) {
    crash_internal::crash(
        {"bounds-check failed: %zu (start) >= %zu (len)", loc}, start, len);
  }

  if (end && *end < start) {
    crash_internal::crash({"bounds-check failed: %zu (start) > %zu (end)", loc},
                          start, *end);
  }

  if (including_end && *including_end < start) {
    crash_internal::crash({"bounds-check failed: %zu (start) > %zu (end)", loc},
                          start, *including_end);
  }

  if (end && *end > len) {
    crash_internal::crash({"bounds-check failed: %zu (end) > %zu (len)", loc},
                          end, len);
  }

  if (including_end && *including_end >= len) {
    crash_internal::crash({"bounds-check failed: %zu (end) >= %zu (len)", loc},
                          end, len);
  }

  if (count && start + *count > len) {
    crash_internal::crash(
        {"bounds-check failed: %zu + %zu (start + count) > %zu (len)", loc},
        start, *count, len);
  }

  std::terminate();
}
}  // namespace best