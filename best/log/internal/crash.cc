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

#include <pthread.h>
#include <stdarg.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "best/base/port.h"
#include "best/log/location.h"

namespace best::crash_internal {
namespace {
inline constexpr char Reset[] = "\N{ESCAPE}[0m";
inline constexpr char Red[] = "\N{ESCAPE}[31m";
}  // namespace

[[noreturn]] void die(
  best::location loc,
  best::fnref<void(char*, size_t, best::fnref<void(const char*, size_t)>)>
    write_message) {
  // TODO: Make this function non-reentrant.

  static char buf[512];
  ::pthread_t self = ::pthread_self();
  ::pthread_getname_np(self, buf, sizeof(buf));  // TODO: Handle this error.

  ::fprintf(stderr, "%slibbest: thread '%s' crashed at %s:%u\n%s", Red, buf,
            loc.impl().file_name(), unsigned(loc.line()), Reset);
  write_message(buf, sizeof(buf), [](const char* data, size_t len) {
    ::fprintf(stderr, "%slibbest: ", Red);
    ::fwrite(data, 1, len, stderr);
    ::fprintf(stderr, "\n%s", Reset);
  });
  ::fflush(stderr);
  ::exit(128);
}

[[noreturn]] BEST_WEAK void crash(best::track_location<const char*> fmt, ...) {
  va_list va;
  va_start(va, fmt);
  die(fmt.location(), [&](char* scratch, size_t scratch_len, auto write) {
    size_t len = ::vsnprintf(scratch, scratch_len, *fmt, va);
    write(scratch, len);
  });
  va_end(va);
}
}  // namespace best::crash_internal
