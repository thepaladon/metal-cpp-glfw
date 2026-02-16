#pragma once

#include "core/aa_types.hpp"

namespace aa
{

struct AAMemoryStats
{
    u64 liveBytes = 0;
    u64 liveCount = 0;
    u64 peakBytes = 0;
    u64 totalAllocs = 0;
    u64 totalFrees = 0;
    u64 droppedRecords = 0;
};

struct AAMemoryAllocInfo
{
    const void *ptr = nullptr;
    u64 size = 0;
    const char *tag = nullptr;
    const char *file = nullptr;
    i32 line = 0;
    const void *returnAddress = nullptr;
    u64 sequence = 0;
};

void AAMemoryTrackerOnAlloc(const void *ptr,
                            usize size,
                            const char *tag,
                            const char *file,
                            i32 line,
                            const void *returnAddress);
void AAMemoryTrackerOnFree(const void *ptr);

AAMemoryStats AAMemoryTrackerGetStats();
usize AAMemoryTrackerCollectLive(AAMemoryAllocInfo *out, usize maxCount);

} // namespace aa
