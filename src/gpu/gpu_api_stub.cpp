#include "gpu/gpu_api.hpp"

#include <cstdlib>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace
{
uint64 nextHandle()
{
    static uint64 next = 1;
    return next++;
}

uint32 formatBytesPerPixel(FORMAT format)
{
    switch (format)
    {
        case FORMAT_RGBA8_UNORM:
            return 4;
        case FORMAT_D32_FLOAT:
            return 4;
        case FORMAT_RG11B10_FLOAT:
            return 4;
        case FORMAT_RGB10_A2_UNORM:
            return 4;
        case FORMAT_NONE:
        default:
            return 1;
    }
}
} // namespace

void *gpuMalloc(std::size_t bytes, MEMORY)
{
    return std::malloc(bytes);
}

void *gpuMalloc(std::size_t bytes, std::size_t align, MEMORY)
{
#if defined(_MSC_VER)
    return _aligned_malloc(bytes, align);
#else
    void *ptr = nullptr;
    if (posix_memalign(&ptr, align, bytes) != 0)
    {
        return nullptr;
    }
    return ptr;
#endif
}

void gpuFree(void *ptr)
{
#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

void *gpuHostToDevicePointer(void *ptr)
{
    return ptr;
}

GpuTextureSizeAlign gpuTextureSizeAlign(GpuTextureDesc desc)
{
    const std::size_t pixelBytes = static_cast<std::size_t>(formatBytesPerPixel(desc.format));
    const std::size_t width = static_cast<std::size_t>(desc.dimensions.x == 0 ? 1 : desc.dimensions.x);
    const std::size_t height = static_cast<std::size_t>(desc.dimensions.y == 0 ? 1 : desc.dimensions.y);
    const std::size_t depth = static_cast<std::size_t>(desc.dimensions.z == 0 ? 1 : desc.dimensions.z);
    const std::size_t layers = static_cast<std::size_t>(desc.layerCount == 0 ? 1 : desc.layerCount);
    const std::size_t size = width * height * depth * layers * pixelBytes;
    return GpuTextureSizeAlign{size, 256};
}

GpuTexture gpuCreateTexture(GpuTextureDesc, void *)
{
    return GpuTexture{nextHandle()};
}

GpuTextureDescriptor gpuTextureViewDescriptor(GpuTexture texture, GpuViewDesc desc)
{
    GpuTextureDescriptor out{};
    out.data[0] = texture.value;
    out.data[1] = static_cast<uint64>(desc.format);
    out.data[2] = static_cast<uint64>(desc.baseMip) | (static_cast<uint64>(desc.mipCount) << 32);
    out.data[3] = static_cast<uint64>(desc.baseLayer) | (static_cast<uint64>(desc.layerCount) << 32);
    return out;
}

GpuTextureDescriptor gpuRWTextureViewDescriptor(GpuTexture texture, GpuViewDesc desc)
{
    return gpuTextureViewDescriptor(texture, desc);
}

GpuPipeline gpuCreateComputePipeline(ByteSpan)
{
    return GpuPipeline{nextHandle()};
}

GpuPipeline gpuCreateGraphicsPipeline(ByteSpan, ByteSpan, GpuRasterDesc)
{
    return GpuPipeline{nextHandle()};
}

GpuPipeline gpuCreateGraphicsMeshletPipeline(ByteSpan, ByteSpan, GpuRasterDesc)
{
    return GpuPipeline{nextHandle()};
}

void gpuFreePipeline(GpuPipeline)
{
}

GpuDepthStencilState gpuCreateDepthStencilState(GpuDepthStencilDesc)
{
    return GpuDepthStencilState{nextHandle()};
}

GpuBlendState gpuCreateBlendState(GpuBlendDesc)
{
    return GpuBlendState{nextHandle()};
}

void gpuFreeDepthStencilState(GpuDepthStencilState)
{
}

void gpuFreeBlendState(GpuBlendState)
{
}

GpuQueue gpuCreateQueue()
{
    return GpuQueue{nextHandle()};
}

GpuCommandBuffer gpuStartCommandRecording(GpuQueue)
{
    return GpuCommandBuffer{nextHandle()};
}

void gpuSubmit(GpuQueue, Span<GpuCommandBuffer>)
{
}

GpuSemaphore gpuCreateSemaphore(uint64)
{
    return GpuSemaphore{nextHandle()};
}

void gpuWaitSemaphore(GpuSemaphore, uint64)
{
}

void gpuDestroySemaphore(GpuSemaphore)
{
}

void gpuMemCpy(GpuCommandBuffer, void *, void *)
{
}

void gpuCopyToTexture(GpuCommandBuffer, void *, void *, GpuTexture)
{
}

void gpuCopyFromTexture(GpuCommandBuffer, void *, void *, GpuTexture)
{
}

void gpuSetActiveTextureHeapPtr(GpuCommandBuffer, void *)
{
}

void gpuBarrier(GpuCommandBuffer, STAGE, STAGE, HAZARD_FLAGS)
{
}

void gpuSignalAfter(GpuCommandBuffer, STAGE, void *, uint64, SIGNAL)
{
}

void gpuWaitBefore(GpuCommandBuffer, STAGE, void *, uint64, OP, HAZARD_FLAGS, uint64)
{
}

void gpuSetPipeline(GpuCommandBuffer, GpuPipeline)
{
}

void gpuSetDepthStencilState(GpuCommandBuffer, GpuDepthStencilState)
{
}

void gpuSetBlendState(GpuCommandBuffer, GpuBlendState)
{
}

void gpuDispatch(GpuCommandBuffer, void *, uvec3)
{
}

void gpuDispatchIndirect(GpuCommandBuffer, void *, void *)
{
}

void gpuBeginRenderPass(GpuCommandBuffer, GpuRenderPassDesc)
{
}

void gpuEndRenderPass(GpuCommandBuffer)
{
}

void gpuDrawIndexedInstanced(GpuCommandBuffer, void *, void *, void *, uint32, uint32)
{
}

void gpuDrawIndexedInstancedIndirect(GpuCommandBuffer, void *, void *, void *, void *)
{
}

void gpuDrawIndexedInstancedIndirectMulti(GpuCommandBuffer, void *, uint32, void *, uint32, void *, void *)
{
}

void gpuDrawMeshlets(GpuCommandBuffer, void *, void *, uvec3)
{
}

void gpuDrawMeshletsIndirect(GpuCommandBuffer, void *, void *, void *)
{
}
