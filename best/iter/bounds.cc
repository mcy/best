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

#include "best/iter/bounds.h"

#include "best/log/internal/crash.h"

namespace best::bounds_internal {
[[noreturn]] void crash(bounds bounds, size_t len, best::location loc) {
  auto [start, end, including_end, count] = bounds;

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

  crash_internal::crash(
      "failed to print proper bounds check result? this is a bug. %zu %zu/%d "
      "%zu/%d %zu/%d %zu",
      start, *end, end.has_value(), *including_end, including_end.has_value(),
      *count, count.has_value(), len);
}
}  // namespace best::bounds_internal
