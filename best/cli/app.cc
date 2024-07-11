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

#include "best/cli/app.h"

#include "best/log/wtf.h"

namespace best {
static std::atomic<app*> global_main = nullptr;
static best::vec<best::pretext<wtf8>> real_argv;

void app::install() {
  auto prev = global_main.exchange(this);
  if (prev != nullptr) {
    best::wtf(
        "detected two distinct `best::app`s in the same binary, at {:?} and "
        "{:?}",
        prev->loc_, loc_);
  }
}

best::pretext<wtf8> app::exe() {
  if (real_argv.is_empty()) return "";
  return real_argv[0];
}

best::span<const best::pretext<wtf8>> app::argv() {
  if (real_argv.is_empty()) return {};
  return real_argv[{.start = 1}];
}

[[noreturn]] void app::start(int argc, char** argv) {
  static std::atomic<bool> called = false;
  if (called.exchange(true)) {
    best::wtf("`best::app::start()` was called twice");
  }

  app* main = global_main.load();
  if (main == nullptr) {
    best::wtf("`best::app::start()` was called but no apps were declared");
  }

  for (char* arg : best::span(argv, argc)) {
    real_argv.push(best::pretext<wtf8>::from_nul(arg));
  }

  std::exit(main->shim_(main->main_));
}
}  // namespace best

BEST_WEAK int main(int argc, char** argv) { best::app::start(argc, argv); }
