#include "core/aa_memory_tracker.hpp"

#include <stdint.h>

namespace aa
{
namespace
{

constexpr usize kMaxTrackedAllocs = 16384;

struct AATrackerSlot
{
    const void *ptr = nullptr;
    u64 size = 0;
    const char *tag = nullptr;
    const char *file = nullptr;
    i32 line = 0;
    const void *returnAddress = nullptr;
    u64 sequence = 0;
    b8 occupied = false;
    b8 tombstone = false;
};

AATrackerSlot g_slots[kMaxTrackedAllocs] = {};
AAMemoryStats g_stats = {};
u64 g_sequence = 0;

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

} // namespace

void AAMemoryTrackerOnAlloc(const void *ptr,
                            usize size,
                            const char *tag,
                            const char *file,
                            i32 line,
                            const void *returnAddress)
{
    if (ptr == nullptr)
    {
        return;
    }

    AATrackerSlot *slot = AAFindInsertSlot(ptr);
    if (slot == nullptr)
    {
        g_stats.droppedRecords += 1;
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
    slot->returnAddress = returnAddress;
    slot->sequence = ++g_sequence;

    g_stats.liveBytes += slot->size;
    g_stats.totalAllocs += 1;
    if (g_stats.liveBytes > g_stats.peakBytes)
    {
        g_stats.peakBytes = g_stats.liveBytes;
    }
}

void AAMemoryTrackerOnFree(const void *ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    AATrackerSlot *slot = AAFindSlot(ptr);
    if (slot == nullptr)
    {
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
    slot->sequence = 0;
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
        out[count].sequence = slot.sequence;
        count += 1;
    }

    return count;
}

} // namespace aa
