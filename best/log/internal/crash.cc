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
          fmt.file().data(), fmt.line());
  // clang-format on
  vfprintf(stderr, *fmt, va);
  fprintf(stderr, "\n");
  fflush(stderr);
  va_end(va);
  std::exit(128);
}
}  // namespace best::crash_internal