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

#include "best/memory/allocator.h"

#include <cstddef>
#include <cstdlib>

#include "best/base/port.h"
#include "best/log/internal/crash.h"
#include "best/memory/layout.h"

namespace best {
namespace {
constexpr size_t MaxAlign = alignof(::max_align_t);

// In debug mode, `best::malloc` implements a "cookie" system, where each
// allocation allocates an extra 16 bytes to hold a layout, which it verifies on
// free, to ensure that sized delete is actually used correctly.
constexpr bool UseCookies = best::is_debug();

// Updates a layout to include a cookie for verifying the layout on free/resize.
best::layout embiggen(best::layout layout) {
  size_t new_align = best::max(layout.align(), best::align_of<best::layout>);
  size_t grow_by = best::size_of<best::layout>;
  if (grow_by < new_align) { grow_by = new_align; }

  return best::layout(
    unsafe("the alignment is a power of two, because it is either `layout`'s "
           "alignment or that of `best::layout`"),
    layout.size() + grow_by, new_align);
}

// Given a pointer to free and its supposed layout, verifies the layout, and
// returns the *actual* pointer to pass to free().
void check_layout(void*& p, best::layout& layout) {
  if (!UseCookies) { return; }

  auto actual = embiggen(layout);
  p = static_cast<char*>(p) - (actual.size() - layout.size());

  best::layout cookie;
  std::memcpy(&cookie, p, sizeof(cookie));
  if (cookie.size() != layout.size() || cookie.align() != layout.align()) {
    best::crash_internal::crash(
      "attempted to free allocation with layout %zu:%zu, but it was actually "
      "allocated with %zu:%zu",
      layout.size(), layout.align(), cookie.size(), cookie.align());
  }

  layout = actual;
}

// Checks for illicit pointer values that should not be passed to the
// allocator's functions.
void check_addr(void* p) {
  if (p == nullptr) {
    best::crash_internal::crash("attempted to de/reallocate a null pointer");
  }
  if (reinterpret_cast<uintptr_t>(p) < 0x1000) {
    best::crash_internal::crash(
      "attempted to de/reallocate a dangling pointer");
  }
}
}  // namespace

void* malloc::alloc(best::layout layout) {
  auto actual = UseCookies ? embiggen(layout) : layout;

  void* p;
  if (actual.align() <= MaxAlign) {
    p = ::malloc(actual.size());
  } else {
    p = ::aligned_alloc(actual.align(), actual.size());
  }

  if (best::unlikely(p == nullptr)) {
    best::crash_internal::crash(
      "malloc() returned a null pointer on layout %zu:%zu", layout.size(),
      layout.align());
  }

  if (UseCookies) {
    std::memcpy(p, &layout, sizeof(layout));
    p = static_cast<char*>(p) + (actual.size() - layout.size());
  }

  if (best::is_debug()) { std::memset(p, 0xcd, layout.size()); }
  return p;
}

void* malloc::zalloc(best::layout layout) {
  auto actual = UseCookies ? embiggen(layout) : layout;

  if (actual.align() <= MaxAlign) {
    void* p = ::calloc(actual.size(), 1);
    if (best::unlikely(p == nullptr)) {
      best::crash_internal::crash(
        "calloc() returned a null pointer on layout %zu:%zu", layout.size(),
        layout.align());
    }
    if (UseCookies) {
      std::memcpy(p, &layout, sizeof(layout));
      p = static_cast<char*>(p) + (actual.size() - layout.size());
    }
    return p;
  }

  void* p = alloc(layout);
  std::memset(p, 0, layout.size());
  return p;
}

void* malloc::realloc(void* ptr, best::layout old, best::layout layout) {
  check_addr(ptr);
  check_layout(ptr, old);
  auto actual = UseCookies ? embiggen(layout) : layout;

  if (actual.align() <= MaxAlign) {
    void* p = ::realloc(ptr, actual.size());
    if (best::unlikely(p == nullptr)) {
      best::crash_internal::crash(
        "realloc() returned a null pointer on layout %zu:%zu -> %zu:%zu",
        old.size(), old.align(), layout.size(), layout.align());
    }
    if (UseCookies) {
      std::memcpy(p, &layout, sizeof(layout));
      p = static_cast<char*>(p) + (actual.size() - layout.size());
    }
    return p;
  }

  void* p = alloc(layout);
  size_t common = best::min(old.size(), layout.size());
  std::memcpy(p, ptr, common);
  return p;
}

void malloc::dealloc(void* ptr, best::layout layout) {
  check_addr(ptr);
  check_layout(ptr, layout);
  ::free(ptr);
}
}  // namespace best
