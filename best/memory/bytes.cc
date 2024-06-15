namespace best {
extern "C" {
// `best` assumes your libc is competent enough to have `memmem`. If not,
// here is a quadratic fallback you can enable if necessary.
#if 0
void* memmem(const void* a, size_t an, const void* b, size_t bn) noexcept {
  if (bn == 0) return const_cast<void*>(a);

  const char* ap = reinterpret_cast<const char*>(a);
  const char* bp = reinterpret_cast<const char*>(b);
  while (an >= bn && an > 0) {
    if (bytes_internal::memcmp(ap, bp, bn) == 0) {
      return const_cast<char*>(ap);
    }
    ++ap;
    --an;
  }
  return nullptr;
}
#endif
}
}  // namespace best