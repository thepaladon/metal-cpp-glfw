#include "core/aa_build_config.hpp"
#include "core/aa_memory_tracker.hpp"

#include <new>
#include <stddef.h>
#include <stdlib.h>

namespace
{

void *AAAllocAligned(size_t size, size_t alignment)
{
    if (alignment <= alignof(max_align_t))
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

void AAFreeAligned(void *ptr, size_t alignment)
{
    if (ptr == nullptr)
    {
        return;
    }

#if defined(_WIN32)
    if (alignment <= alignof(max_align_t))
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

void *operator new(size_t size)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "global_new", nullptr, 0, AAReturnAddress());
#endif
    return ptr;
}

void operator delete(void *ptr) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new[](size_t size)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "global_new_array", nullptr, 0, AAReturnAddress());
#endif
    return ptr;
}

void operator delete[](void *ptr) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new(size_t size, const char *file, int line)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "AA_NEW", file, line, AAReturnAddress());
#endif
    return ptr;
}

void operator delete(void *ptr, const char *, int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new[](size_t size, const char *file, int line)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "AA_NEW_ARRAY", file, line, AAReturnAddress());
#endif
    return ptr;
}

void operator delete[](void *ptr, const char *, int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new(size_t size, const char *tag, const char *file, int line)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, tag, file, line, AAReturnAddress());
#endif
    return ptr;
}

void operator delete(void *ptr, const char *, const char *, int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new[](size_t size, const char *tag, const char *file, int line)
{
    if (size == 0)
    {
        size = 1;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, tag, file, line, AAReturnAddress());
#endif
    return ptr;
}

void operator delete[](void *ptr, const char *, const char *, int) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void *operator new(size_t size, std::align_val_t alignment)
{
    if (size == 0)
    {
        size = 1;
    }

    const size_t alignValue = static_cast<size_t>(alignment);
    void *ptr = AAAllocAligned(size, alignValue);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "global_new_aligned", nullptr, 0, AAReturnAddress());
#endif
    return ptr;
}

void operator delete(void *ptr, std::align_val_t alignment) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    AAFreeAligned(ptr, static_cast<size_t>(alignment));
}

void *operator new[](size_t size, std::align_val_t alignment)
{
    if (size == 0)
    {
        size = 1;
    }

    const size_t alignValue = static_cast<size_t>(alignment);
    void *ptr = AAAllocAligned(size, alignValue);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnAlloc(ptr, size, "global_new_array_aligned", nullptr, 0, AAReturnAddress());
#endif
    return ptr;
}

void operator delete[](void *ptr, std::align_val_t alignment) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    AAFreeAligned(ptr, static_cast<size_t>(alignment));
}

void operator delete(void *ptr, size_t) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void operator delete[](void *ptr, size_t) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    free(ptr);
}

void operator delete(void *ptr, size_t, std::align_val_t alignment) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    AAFreeAligned(ptr, static_cast<size_t>(alignment));
}

void operator delete[](void *ptr, size_t, std::align_val_t alignment) noexcept
{
#if AA_CFG_MEMORY_TRACKING
    aa::AAMemoryTrackerOnFree(ptr);
#endif
    AAFreeAligned(ptr, static_cast<size_t>(alignment));
}
