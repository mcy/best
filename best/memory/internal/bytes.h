#ifndef BEST_MEMORY_INTERNAL_BYTES_H_
#define BEST_MEMORY_INTERNAL_BYTES_H_

#include <cstddef>

namespace best::bytes_internal {
extern "C" {
void* memcpy(void*, const void*, size_t) noexcept;
void* memmove(void*, const void*, size_t) noexcept;
void* memmem(const void*, size_t, const void*, size_t) noexcept;
void* memset(void*, int, size_t) noexcept;
int memcmp(const void*, const void*, size_t) noexcept;
}  // extern "C"
}  // namespace best::bytes_internal

#endif  // BEST_MEMORY_INTERNAL_BYTES_H_