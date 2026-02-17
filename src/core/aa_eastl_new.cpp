#include "core/aa_build_config.hpp"
#include "core/aa_memory_tracker.hpp"

#include <stddef.h>
#include <stdlib.h>

namespace
{

void *AAEastlAllocAligned(size_t size, size_t alignment)
{
    if (alignment <= alignof(void *))
    {
        return malloc(size);
    }

#if defined(_WIN32)
    return _aligned_malloc(size, alignment);
#else
    void *mem = nullptr;
    if (posix_memalign(&mem, alignment, size) != 0)
    {
        return nullptr;
    }
    return mem;
#endif
}

void AAEastlFreeAligned(void *ptr, size_t alignment)
{
    if (ptr == nullptr)
    {
        return;
    }

#if defined(_WIN32)
    if (alignment <= alignof(void *))
    {
        free(ptr);
        return;
    }
    _aligned_free(ptr);
#else
    (void)alignment;
    free(ptr);
#endif
}

const void *AAReturnAddress()
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_return_address(0);
#else
    return nullptr;
#endif
}

} // namespace

void *operator new[](size_t size,
                     const char *name,
                     int,
                     unsigned,
                     const char *file,
                     int line)
{
    void *ptr = malloc(size);
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, name, file, line, AAReturnAddress());
#endif
    return ptr;
}

void *operator new[](size_t size,
                     size_t alignment,
                     size_t,
                     const char *name,
                     int,
                     unsigned,
                     const char *file,
                     int line)
{
    void *mem = AAEastlAllocAligned(size, alignment);
    if (mem == nullptr)
    {
        mem = malloc(size);
    }
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(mem, size, name, file, line, AAReturnAddress());
#endif
    return mem;
}

void operator delete[](void *p,
                       const char *,
                       int,
                       unsigned,
                       const char *,
                       int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(p);
#endif
    free(p);
}

void operator delete[](void *p,
                       size_t alignment,
                       size_t,
                       const char *,
                       int,
                       unsigned,
                       const char *,
                       int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(p);
#endif
    AAEastlFreeAligned(p, alignment);
}
