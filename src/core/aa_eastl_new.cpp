#include "core/aa_memory_tracker.hpp"

#include <stdlib.h>

namespace
{

const void *AAReturnAddress()
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_return_address(0);
#else
    return nullptr;
#endif
}

} // namespace

void *operator new[](unsigned long size,
                     const char *name,
                     int,
                     unsigned,
                     const char *file,
                     int line)
{
    void *ptr = malloc(size);
    aa::AAMemoryTrackerOnAlloc(ptr, size, name, file, line, AAReturnAddress());
    return ptr;
}

void *operator new[](unsigned long size,
                     unsigned long alignment,
                     unsigned long,
                     const char *name,
                     int,
                     unsigned,
                     const char *file,
                     int line)
{
    if (alignment <= alignof(void *))
    {
        void *ptr = malloc(size);
        aa::AAMemoryTrackerOnAlloc(ptr, size, name, file, line, AAReturnAddress());
        return ptr;
    }

    void *mem = nullptr;
    if (posix_memalign(&mem, alignment, size) != 0)
    {
        mem = malloc(size);
    }
    aa::AAMemoryTrackerOnAlloc(mem, size, name, file, line, AAReturnAddress());
    return mem;
}

void operator delete[](void *p,
                       const char *,
                       int,
                       unsigned,
                       const char *,
                       int) noexcept
{
    aa::AAMemoryTrackerOnFree(p);
    free(p);
}

void operator delete[](void *p,
                       unsigned long,
                       unsigned long,
                       const char *,
                       int,
                       unsigned,
                       const char *,
                       int) noexcept
{
    aa::AAMemoryTrackerOnFree(p);
    free(p);
}
