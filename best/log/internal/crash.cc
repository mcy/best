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

#include "best/log/internal/crash.h"

#include <stdarg.h>

#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "best/base/port.h"
#include "best/log/location.h"

namespace best::crash_internal {
[[noreturn]] BEST_WEAK void crash(best::track_location<const char*> fmt, ...) {
  va_list va;
  va_start(va, fmt);
  // clang-format off
  fprintf(stderr,
          "error: best::crash_internal::crash() called at %s:%" PRIu32 "\n"
          "       this typically means that <LOGGING LIBRARY TBD> was not linked in\n"
          "error: ",
          fmt.impl().file_name(), fmt.line());
  // clang-format on
  vfprintf(stderr, *fmt, va);
  fprintf(stderr, "\n");
  fflush(stderr);
  va_end(va);
  ::exit(128);
}
}  // namespace best::crash_internal
