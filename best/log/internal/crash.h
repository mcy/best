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

#ifndef BEST_LOG_INTERNAL_CRASH_H_
#define BEST_LOG_INTERNAL_CRASH_H_

#include <stddef.h>

#include "best/log/location.h"

namespace best::crash_internal {
/// Internal shim for crashing without depending on the logging headers.
[[noreturn]] void crash(best::track_location<const char*> fmt, ...);
}  // namespace best::crash_internal

#endif  // BEST_LOG_INTERNAL_CRASH_H_
