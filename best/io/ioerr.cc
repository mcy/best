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

#include "best/io/errno_.h"

#include "best/base/guard.h"

namespace best {
namespace {
struct e {
  best::str name, message;
};

const std::array Errnos = {
// See `errnos.py`.
#include "best/io/errnos.inc"
};
}  // namespace

best::option<best::str> errnos::name() const {
  if (value_ >= Errnos.size() || Errnos[value_].name.is_empty()) {
    return best::none;
  }

  return Errnos[value_].name;
}
best::option<best::str> errnos::message() const {
  BEST_GUARD(name());
  return Errnos[value_].message;
}
}  // namespace best
