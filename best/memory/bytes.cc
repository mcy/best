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

#if 0

// `best` assumes your libc is competent enough to have `memmem`. If not,
// here is a quadratic fallback you can enable if necessary.

namespace best {
extern "C" {
void* memmem(const void* a, size_t an, const void* b, size_t bn) noexcept {
  if (bn == 0) return const_cast<void*>(a);

  const char* ap = reinterpret_cast<const char*>(a);
  const char* bp = reinterpret_cast<const char*>(b);
  while (an >= bn) {
    if (bytes_internal::memcmp(ap, bp, bn) == 0) {
      return const_cast<char*>(ap);
    }
    ++ap;
    --an;
  }
  return nullptr;
}
}
}  // namespace best
#endif
