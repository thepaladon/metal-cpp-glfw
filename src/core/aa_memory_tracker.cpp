#include "core/aa_memory_tracker.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <dlfcn.h>
#if AA_CFG_MEMORY_TRACKING_SYMBOLS
#include <cxxabi.h>
#endif
#if AA_CFG_MEMORY_TRACKING_CALLSTACK
#include <execinfo.h>
#endif
#endif

namespace aa
{

#if AA_CFG_MEMORY_TRACKING
namespace
{

constexpr usize kMaxTrackedAllocs = AA_CFG_MEMORY_TRACKING_MAX_ALLOCS;
#if AA_CFG_MEMORY_TRACKING_CALLSTACK
constexpr usize kMaxBacktraceFrames = kAAMemoryMaxFrames + 2;
#endif

struct AATrackerSlot
{
    const void *ptr = nullptr;
    u64 size = 0;
    const char *tag = nullptr;
    const char *file = nullptr;
    i32 line = 0;
    const void *returnAddress = nullptr;
    usize frameCount = 0;
    const void *frames[kAAMemoryMaxFrames] = {};
    u64 sequence = 0;
    b8 occupied = false;
    b8 tombstone = false;
};

AATrackerSlot g_slots[kMaxTrackedAllocs] = {};
AAMemoryStats g_stats = {};
u64 g_sequence = 0;
thread_local b8 g_inTracker = false;

usize AASlotIndex(const void *ptr)
{
    const uintptr_t value = reinterpret_cast<uintptr_t>(ptr);
    return static_cast<usize>((value >> 4u) % kMaxTrackedAllocs);
}

AATrackerSlot *AAFindSlot(const void *ptr)
{
    usize index = AASlotIndex(ptr);
    for (usize i = 0; i < kMaxTrackedAllocs; ++i)
    {
        AATrackerSlot &slot = g_slots[index];
        if (!slot.occupied && !slot.tombstone)
        {
            return nullptr;
        }

        if (slot.occupied && slot.ptr == ptr)
        {
            return &slot;
        }

        index = (index + 1) % kMaxTrackedAllocs;
    }

    return nullptr;
}

AATrackerSlot *AAFindInsertSlot(const void *ptr)
{
    usize index = AASlotIndex(ptr);
    AATrackerSlot *firstTombstone = nullptr;

    for (usize i = 0; i < kMaxTrackedAllocs; ++i)
    {
        AATrackerSlot &slot = g_slots[index];

        if (slot.occupied && slot.ptr == ptr)
        {
            return &slot;
        }

        if (!slot.occupied)
        {
            if (slot.tombstone)
            {
                if (firstTombstone == nullptr)
                {
                    firstTombstone = &slot;
                }
            }
            else
            {
                return firstTombstone != nullptr ? firstTombstone : &slot;
            }
        }

        index = (index + 1) % kMaxTrackedAllocs;
    }

    return firstTombstone;
}

b8 AATagged(const AATrackerSlot &slot)
{
    return slot.tag != nullptr && slot.tag[0] != '\0';
}

void AACaptureFrames(AATrackerSlot &slot, const void *returnAddress)
{
    slot.frameCount = 0;
    for (usize i = 0; i < kAAMemoryMaxFrames; ++i)
    {
        slot.frames[i] = nullptr;
    }

#if AA_CFG_MEMORY_TRACKING_CALLSTACK && !defined(_WIN32)
    void *trace[kMaxBacktraceFrames] = {};
    const int captured = backtrace(trace, static_cast<int>(kMaxBacktraceFrames));
    if (captured > 2)
    {
        const usize count = static_cast<usize>(captured - 2);
        slot.frameCount = count < kAAMemoryMaxFrames ? count : kAAMemoryMaxFrames;
        for (usize i = 0; i < slot.frameCount; ++i)
        {
            slot.frames[i] = trace[i + 2];
        }
        slot.returnAddress = slot.frames[0];
        return;
    }
#endif

    slot.returnAddress = returnAddress;
    if (returnAddress != nullptr)
    {
        slot.frames[0] = returnAddress;
        slot.frameCount = 1;
    }
}

u64 AAGroupHash(const AATrackerSlot &slot)
{
    u64 hash = 1469598103934665603ull;
    for (usize i = 0; i < slot.frameCount; ++i)
    {
        const u64 value = static_cast<u64>(reinterpret_cast<uintptr_t>(slot.frames[i]));
        hash ^= value;
        hash *= 1099511628211ull;
    }

    hash ^= static_cast<u64>(reinterpret_cast<uintptr_t>(slot.tag));
    hash *= 1099511628211ull;
    return hash;
}

void AASortGroups(AAMemoryGroupInfo *groups, usize count)
{
    for (usize i = 0; i < count; ++i)
    {
        usize maxIndex = i;
        for (usize j = i + 1; j < count; ++j)
        {
            if (groups[j].liveBytes > groups[maxIndex].liveBytes)
            {
                maxIndex = j;
            }
        }

        if (maxIndex != i)
        {
            const AAMemoryGroupInfo tmp = groups[i];
            groups[i] = groups[maxIndex];
            groups[maxIndex] = tmp;
        }
    }
}

const char *AAResolveSymbolRaw(const void *address)
{
#if defined(_WIN32)
    (void)address;
    return nullptr;
#else
    if (address == nullptr)
    {
        return nullptr;
    }

    Dl_info info = {};
    if (dladdr(address, &info) != 0)
    {
        return info.dli_sname;
    }
    return nullptr;
#endif
}

b8 AAShouldSkipFrame(const char *symbolName)
{
    if (symbolName == nullptr)
    {
        return false;
    }

    return strstr(symbolName, "AAMemoryTrackerOnAlloc") != nullptr ||
           strstr(symbolName, "AAAllocAligned") != nullptr ||
           strstr(symbolName, "_Znwm") != nullptr ||
           strstr(symbolName, "_Znam") != nullptr ||
           strstr(symbolName, "operator new") != nullptr ||
           strstr(symbolName, "eastl::allocator::allocate") != nullptr;
}

const void *AASelectUserFrame(const AATrackerSlot &slot)
{
    for (usize i = 0; i < slot.frameCount; ++i)
    {
        const void *frame = slot.frames[i];
        const char *symbolName = AAResolveSymbolRaw(frame);
        if (!AAShouldSkipFrame(symbolName))
        {
            return frame;
        }
    }

    if (slot.frameCount > 0)
    {
        return slot.frames[0];
    }
    return slot.returnAddress;
}

} // namespace

void AAMemoryTrackerOnAlloc(const void *ptr,
                            usize size,
                            const char *tag,
                            const char *file,
                            i32 line,
                            const void *returnAddress)
{
    if (ptr == nullptr || g_inTracker)
    {
        return;
    }

    g_inTracker = true;

    AATrackerSlot *slot = AAFindInsertSlot(ptr);
    if (slot == nullptr)
    {
        g_stats.droppedRecords += 1;
        g_inTracker = false;
        return;
    }

    if (slot->occupied)
    {
        if (g_stats.liveBytes >= slot->size)
        {
            g_stats.liveBytes -= slot->size;
        }
    }
    else
    {
        g_stats.liveCount += 1;
    }

    slot->occupied = true;
    slot->tombstone = false;
    slot->ptr = ptr;
    slot->size = static_cast<u64>(size);
    slot->tag = tag;
    slot->file = file;
    slot->line = line;
    slot->sequence = ++g_sequence;

    AACaptureFrames(*slot, returnAddress);

    g_stats.liveBytes += slot->size;
    g_stats.totalAllocs += 1;
    if (g_stats.liveBytes > g_stats.peakBytes)
    {
        g_stats.peakBytes = g_stats.liveBytes;
    }

    g_inTracker = false;
}

void AAMemoryTrackerOnFree(const void *ptr)
{
    if (ptr == nullptr || g_inTracker)
    {
        return;
    }

    g_inTracker = true;

    AATrackerSlot *slot = AAFindSlot(ptr);
    if (slot == nullptr)
    {
        g_inTracker = false;
        return;
    }

    if (g_stats.liveBytes >= slot->size)
    {
        g_stats.liveBytes -= slot->size;
    }
    if (g_stats.liveCount > 0)
    {
        g_stats.liveCount -= 1;
    }
    g_stats.totalFrees += 1;

    slot->occupied = false;
    slot->tombstone = true;
    slot->ptr = nullptr;
    slot->size = 0;
    slot->tag = nullptr;
    slot->file = nullptr;
    slot->line = 0;
    slot->returnAddress = nullptr;
    slot->frameCount = 0;
    for (usize i = 0; i < kAAMemoryMaxFrames; ++i)
    {
        slot->frames[i] = nullptr;
    }
    slot->sequence = 0;

    g_inTracker = false;
}

AAMemoryStats AAMemoryTrackerGetStats()
{
    return g_stats;
}

usize AAMemoryTrackerCollectLive(AAMemoryAllocInfo *out, usize maxCount)
{
    if (out == nullptr || maxCount == 0)
    {
        return 0;
    }

    usize count = 0;
    for (usize i = 0; i < kMaxTrackedAllocs; ++i)
    {
        const AATrackerSlot &slot = g_slots[i];
        if (!slot.occupied)
        {
            continue;
        }

        if (count >= maxCount)
        {
            break;
        }

        out[count].ptr = slot.ptr;
        out[count].size = slot.size;
        out[count].tag = slot.tag;
        out[count].file = slot.file;
        out[count].line = slot.line;
        out[count].returnAddress = slot.returnAddress;
        out[count].frameCount = slot.frameCount;
        for (usize f = 0; f < kAAMemoryMaxFrames; ++f)
        {
            out[count].frames[f] = slot.frames[f];
        }
        out[count].sequence = slot.sequence;
        count += 1;
    }

    return count;
}

usize AAMemoryTrackerCollectGroups(AAMemoryGroupInfo *out, usize maxCount, b8 taggedOnly)
{
    if (out == nullptr || maxCount == 0)
    {
        return 0;
    }

    for (usize i = 0; i < maxCount; ++i)
    {
        out[i] = {};
    }

    usize count = 0;
    for (usize i = 0; i < kMaxTrackedAllocs; ++i)
    {
        const AATrackerSlot &slot = g_slots[i];
        if (!slot.occupied)
        {
            continue;
        }
        if (taggedOnly && !AATagged(slot))
        {
            continue;
        }

        const u64 hash = AAGroupHash(slot);
        usize groupIndex = maxCount;
        for (usize g = 0; g < count; ++g)
        {
            if (out[g].groupKey == hash && out[g].tag == slot.tag && out[g].file == slot.file && out[g].line == slot.line)
            {
                groupIndex = g;
                break;
            }
        }

        if (groupIndex == maxCount)
        {
            if (count >= maxCount)
            {
                continue;
            }
            groupIndex = count;
            out[groupIndex].liveBytes = 0;
            out[groupIndex].liveCount = 0;
            out[groupIndex].groupKey = hash;
            out[groupIndex].tag = slot.tag;
            out[groupIndex].file = slot.file;
            out[groupIndex].line = slot.line;
            out[groupIndex].frame0 = nullptr;
            count += 1;
        }

        out[groupIndex].liveBytes += slot.size;
        out[groupIndex].liveCount += 1;
        out[groupIndex].frame0 = AASelectUserFrame(slot);
    }

    AASortGroups(out, count);
    return count;
}

const char *AAMemoryTrackerSymbolize(const void *address, char *buffer, usize bufferSize)
{
    if (buffer == nullptr || bufferSize == 0)
    {
        return nullptr;
    }

#if defined(_WIN32) || !AA_CFG_MEMORY_TRACKING_SYMBOLS
    (void)address;
    buffer[0] = '\0';
    return nullptr;
#else
    if (address == nullptr)
    {
        buffer[0] = '\0';
        return nullptr;
    }

    Dl_info info = {};
    if (dladdr(address, &info) != 0 && info.dli_sname != nullptr)
    {
        int demangleStatus = -1;
        char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &demangleStatus);
        const char *name = (demangleStatus == 0 && demangled != nullptr) ? demangled : info.dli_sname;

        usize i = 0;
        while (i + 1 < bufferSize && name[i] != '\0')
        {
            buffer[i] = name[i];
            ++i;
        }
        buffer[i] = '\0';

        if (demangled != nullptr)
        {
            free(demangled);
        }
        return buffer;
    }

    buffer[0] = '\0';
    return nullptr;
#endif
}

#else

void AAMemoryTrackerOnAlloc(const void *, usize, const char *, const char *, i32, const void *)
{
}

void AAMemoryTrackerOnFree(const void *)
{
}

AAMemoryStats AAMemoryTrackerGetStats()
{
    return {};
}

usize AAMemoryTrackerCollectLive(AAMemoryAllocInfo *, usize)
{
    return 0;
}

usize AAMemoryTrackerCollectGroups(AAMemoryGroupInfo *, usize, b8)
{
    return 0;
}

const char *AAMemoryTrackerSymbolize(const void *, char *buffer, usize bufferSize)
{
    if (buffer != nullptr && bufferSize > 0)
    {
        buffer[0] = '\0';
    }
    return nullptr;
}

#endif

} // namespace aa
