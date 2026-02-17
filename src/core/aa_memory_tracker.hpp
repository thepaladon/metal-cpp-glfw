#pragma once

#include "core/aa_build_config.hpp"
#include "core/aa_types.hpp"

namespace aa
{

constexpr usize kAAMemoryMaxFrames = AA_CFG_MEMORY_TRACKING_MAX_FRAMES;

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
    usize frameCount = 0;
    const void *frames[kAAMemoryMaxFrames] = {};
    u64 sequence = 0;
};

struct AAMemoryGroupInfo
{
    u64 groupKey = 0;
    u64 liveBytes = 0;
    u64 liveCount = 0;
    const char *tag = nullptr;
    const char *file = nullptr;
    i32 line = 0;
    const void *frame0 = nullptr;
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
usize AAMemoryTrackerCollectGroups(AAMemoryGroupInfo *out, usize maxCount, b8 taggedOnly);
const char *AAMemoryTrackerSymbolize(const void *address, char *buffer, usize bufferSize);

} // namespace aa
