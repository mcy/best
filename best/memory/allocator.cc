#include "best/memory/allocator.h"

#include <cstddef>
#include <cstdlib>

#include "best/base/port.h"
#include "best/log/internal/crash.h"
#include "best/memory/layout.h"

namespace best {
inline constexpr size_t MaxAlign = alignof(::max_align_t);

void* malloc::alloc(best::layout layout) {
  void* p;
  if (layout.size() <= MaxAlign) {
    p = ::malloc(layout.size());
  } else {
    p = ::aligned_alloc(layout.align(), layout.size());
  }

  if (best::unlikely(p == nullptr)) {
    best::crash_internal::crash("malloc() returned a null pointer");
  }
  if (best::is_debug()) {
    std::memset(p, 0xcd, layout.size());
  }
  return p;
}

void* malloc::zalloc(best::layout layout) {
  if (layout.size() <= MaxAlign) {
    void* p = ::calloc(layout.size(), 1);
    if (best::unlikely(p == nullptr)) {
      best::crash_internal::crash("calloc() returned a null pointer");
    }
    return p;
  }

  void* p = alloc(layout);
  std::memset(p, 0, layout.size());
  return p;
}

void* malloc::realloc(void* ptr, best::layout old, best::layout layout) {
  if (layout.size() <= MaxAlign) {
    void* p = ::realloc(ptr, layout.size());
    if (best::unlikely(p == nullptr)) {
      best::crash_internal::crash("realloc() returned a null pointer");
    }
    return p;
  }

  void* p = alloc(layout);
  size_t common = best::min(old.size(), layout.size());
  std::memcpy(p, ptr, common);
  return p;
}

void malloc::dealloc(void* ptr, best::layout layout) { ::free(ptr); }
}  // namespace best