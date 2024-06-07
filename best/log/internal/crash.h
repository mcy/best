#ifndef BEST_LOG_INTERNAL_CRASH_H_
#define BEST_LOG_INTERNAL_CRASH_H_

#include <stddef.h>

#include "best/log/location.h"

namespace best::crash_internal {
/// Internal shim for crashing without depending on the logging headers.
[[noreturn]] void crash(best::track_location<const char*> fmt, ...);
}  // namespace best::crash_internal

#endif  // BEST_LOG_INTERNAL_CRASH_H_