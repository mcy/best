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

#ifndef BEST_LOG_WTF_H_
#define BEST_LOG_WTF_H_

#include "best/text/format.h"
#include "best/text/strbuf.h"

//! Crashing and assertions.
//!
//! This header provides high-level functions for ending the current process
//! with a formatted message.

namespace best {
/// # `best::wtf()`
///
/// Quickly exits the program due to an unexpected, unrecoverable condition. It
/// prints a message using `best::format()` before exiting.
///
/// NOTE: This function is not currently async-signal-safe, because it
/// allocates. This limitation will eventually be lifted.
template <best::formattable... Args>
[[noreturn]] void wtf(best::format_template<Args...> templ = "",
                      const Args&... args) {
  best::strbuf message = best::format(templ, args...);
  auto write = [&](char* scratch, size_t scratch_len, auto write) {
    if (message.is_empty()) { message.push("explicit call to best::wtf()"); }
    write(message.data(), message.size());
  };

  crash_internal::die(templ.where(), write);
}
}  // namespace best

#endif  // BEST_LOG_WTF_H_
