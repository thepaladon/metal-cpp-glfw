#include <stdlib.h>

void *operator new[](unsigned long size,
                     const char *,
                     int,
                     unsigned,
                     const char *,
                     int)
{
    return malloc(size);
}

void *operator new[](unsigned long size,
                     unsigned long alignment,
                     unsigned long,
                     const char *,
                     int,
                     unsigned,
                     const char *,
                     int)
{
    if (alignment <= alignof(void *))
    {
        return malloc(size);
    }

    void *mem = nullptr;
    if (posix_memalign(&mem, alignment, size) != 0)
    {
        return malloc(size);
    }
    return mem;
}

void operator delete[](void *p,
                       const char *,
                       int,
                       unsigned,
                       const char *,
                       int) noexcept
{
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
    free(p);
}
