#pragma once

#include <cstddef>
#include <cstdint>

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

constexpr uint8 ALL_MIPS = 0xff;
constexpr uint16 ALL_LAYERS = 0xffff;

template <typename T>
struct Span
{
    const T *data = nullptr;
    std::size_t size = 0;
};

struct ByteSpan
{
    const uint8 *data = nullptr;
    std::size_t size = 0;
};

struct uvec3
{
    uint32 x = 1;
    uint32 y = 1;
    uint32 z = 1;
};

// Opaque handles
struct GpuPipeline
{
    uint64 value = 0;
};

struct GpuTexture
{
    uint64 value = 0;
};

struct GpuDepthStencilState
{
    uint64 value = 0;
};

struct GpuBlendState
{
    uint64 value = 0;
};

struct GpuQueue
{
    uint64 value = 0;
};

struct GpuCommandBuffer
{
    uint64 value = 0;
};

struct GpuSemaphore
{
    uint64 value = 0;
};

// Enums
enum MEMORY
{
    MEMORY_DEFAULT,
    MEMORY_GPU,
    MEMORY_READBACK,
};

enum CULL
{
    CULL_CCW,
    CULL_CW,
    CULL_ALL,
    CULL_NONE,
};

enum DEPTH_FLAGS : uint32
{
    DEPTH_NONE = 0,
    DEPTH_READ = 1u << 0,
    DEPTH_WRITE = 1u << 1,
};

enum OP
{
    OP_NEVER,
    OP_LESS,
    OP_EQUAL,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_NOT_EQUAL,
    OP_GREATER_EQUAL,
    OP_ALWAYS,
};

enum STENCIL_OP
{
    OP_KEEP,
    OP_ZERO,
    OP_REPLACE,
    OP_INCR_CLAMP,
    OP_DECR_CLAMP,
    OP_INVERT,
    OP_INCR_WRAP,
    OP_DECR_WRAP,
};

enum BLEND
{
    BLEND_ADD,
    BLEND_SUBTRACT,
    BLEND_REV_SUBTRACT,
    BLEND_MIN,
    BLEND_MAX,
};

enum FACTOR
{
    FACTOR_ZERO,
    FACTOR_ONE,
    FACTOR_SRC_COLOR,
    FACTOR_ONE_MINUS_SRC_COLOR,
    FACTOR_DST_COLOR,
    FACTOR_ONE_MINUS_DST_COLOR,
    FACTOR_SRC_ALPHA,
    FACTOR_ONE_MINUS_SRC_ALPHA,
    FACTOR_DST_ALPHA,
    FACTOR_ONE_MINUS_DST_ALPHA,
};

enum TOPOLOGY
{
    TOPOLOGY_TRIANGLE_LIST,
    TOPOLOGY_TRIANGLE_STRIP,
    TOPOLOGY_TRIANGLE_FAN,
};

enum TEXTURE
{
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_CUBE,
    TEXTURE_2D_ARRAY,
    TEXTURE_CUBE_ARRAY,
};

enum FORMAT
{
    FORMAT_NONE,
    FORMAT_RGBA8_UNORM,
    FORMAT_D32_FLOAT,
    FORMAT_RG11B10_FLOAT,
    FORMAT_RGB10_A2_UNORM,
};

enum USAGE_FLAGS : uint32
{
    USAGE_NONE = 0,
    USAGE_SAMPLED = 1u << 0,
    USAGE_STORAGE = 1u << 1,
    USAGE_COLOR_ATTACHMENT = 1u << 2,
    USAGE_DEPTH_STENCIL_ATTACHMENT = 1u << 3,
};

enum STAGE
{
    STAGE_TRANSFER,
    STAGE_COMPUTE,
    STAGE_RASTER_COLOR_OUT,
    STAGE_PIXEL_SHADER,
    STAGE_VERTEX_SHADER,
};

enum HAZARD_FLAGS : uint32
{
    HAZARD_NONE = 0,
    HAZARD_DRAW_ARGUMENTS = 1u << 0,
    HAZARD_DESCRIPTORS = 1u << 1,
    HAZARD_DEPTH_STENCIL = 1u << 2,
};

enum SIGNAL
{
    SIGNAL_ATOMIC_SET,
    SIGNAL_ATOMIC_MAX,
    SIGNAL_ATOMIC_OR,
};

inline DEPTH_FLAGS operator|(DEPTH_FLAGS a, DEPTH_FLAGS b)
{
    return static_cast<DEPTH_FLAGS>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline USAGE_FLAGS operator|(USAGE_FLAGS a, USAGE_FLAGS b)
{
    return static_cast<USAGE_FLAGS>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline HAZARD_FLAGS operator|(HAZARD_FLAGS a, HAZARD_FLAGS b)
{
    return static_cast<HAZARD_FLAGS>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

// Structs
struct Stencil
{
    OP test = OP_ALWAYS;
    STENCIL_OP failOp = OP_KEEP;
    STENCIL_OP passOp = OP_KEEP;
    STENCIL_OP depthFailOp = OP_KEEP;
    uint8 reference = 0;
};

struct GpuDepthStencilDesc
{
    DEPTH_FLAGS depthMode = DEPTH_NONE;
    OP depthTest = OP_ALWAYS;
    float depthBias = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    uint8 stencilReadMask = 0xff;
    uint8 stencilWriteMask = 0xff;
    Stencil stencilFront = {};
    Stencil stencilBack = {};
};

struct GpuBlendDesc
{
    BLEND colorOp = BLEND_ADD;
    FACTOR srcColorFactor = FACTOR_ONE;
    FACTOR dstColorFactor = FACTOR_ZERO;
    BLEND alphaOp = BLEND_ADD;
    FACTOR srcAlphaFactor = FACTOR_ONE;
    FACTOR dstAlphaFactor = FACTOR_ZERO;
    uint8 colorWriteMask = 0xf;
};

struct ColorTarget
{
    FORMAT format = FORMAT_NONE;
    uint8 writeMask = 0xf;
};

struct GpuRasterDesc
{
    TOPOLOGY topology = TOPOLOGY_TRIANGLE_LIST;
    CULL cull = CULL_NONE;
    bool alphaToCoverage = false;
    bool supportDualSourceBlending = false;
    uint8 sampleCount = 1;
    FORMAT depthFormat = FORMAT_NONE;
    FORMAT stencilFormat = FORMAT_NONE;
    Span<ColorTarget> colorTargets = {};
    GpuBlendDesc *blendstate = nullptr; // optional embedded blend state
};

struct uint32x3
{
    uint32 x = 1;
    uint32 y = 1;
    uint32 z = 1;
};

struct GpuTextureDesc
{
    TEXTURE type = TEXTURE_2D;
    uint32x3 dimensions = {};
    uint32 mipCount = 1;
    uint32 layerCount = 1;
    uint32 sampleCount = 1;
    FORMAT format = FORMAT_NONE;
    USAGE_FLAGS usage = USAGE_NONE;
};

struct GpuViewDesc
{
    FORMAT format = FORMAT_NONE;
    uint8 baseMip = 0;
    uint8 mipCount = ALL_MIPS;
    uint16 baseLayer = 0;
    uint16 layerCount = ALL_LAYERS;
};

struct GpuTextureSizeAlign
{
    std::size_t size = 0;
    std::size_t align = 1;
};

struct GpuTextureDescriptor
{
    uint64 data[4] = {};
};

struct GpuColorAttachmentDesc
{
    GpuTexture texture = {};
    FORMAT format = FORMAT_NONE;
    bool clear = false;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

struct GpuDepthAttachmentDesc
{
    GpuTexture texture = {};
    FORMAT format = FORMAT_NONE;
    bool clear = false;
    float clearDepth = 1.0f;
};

struct GpuRenderPassDesc
{
    Span<GpuColorAttachmentDesc> colorAttachments = {};
    GpuDepthAttachmentDesc depthAttachment = {};
};

// Memory
void *gpuMalloc(std::size_t bytes, MEMORY memory = MEMORY_DEFAULT);
void *gpuMalloc(std::size_t bytes, std::size_t align, MEMORY memory = MEMORY_DEFAULT);
void gpuFree(void *ptr);
void *gpuHostToDevicePointer(void *ptr);

// Textures
GpuTextureSizeAlign gpuTextureSizeAlign(GpuTextureDesc desc);
GpuTexture gpuCreateTexture(GpuTextureDesc desc, void *ptrGpu);
GpuTextureDescriptor gpuTextureViewDescriptor(GpuTexture texture, GpuViewDesc desc);
GpuTextureDescriptor gpuRWTextureViewDescriptor(GpuTexture texture, GpuViewDesc desc);

// Pipelines
GpuPipeline gpuCreateComputePipeline(ByteSpan computeIR);
GpuPipeline gpuCreateGraphicsPipeline(ByteSpan vertexIR, ByteSpan pixelIR, GpuRasterDesc desc);
GpuPipeline gpuCreateGraphicsMeshletPipeline(ByteSpan meshletIR, ByteSpan pixelIR, GpuRasterDesc desc);
void gpuFreePipeline(GpuPipeline pipeline);

// State objects
GpuDepthStencilState gpuCreateDepthStencilState(GpuDepthStencilDesc desc);
GpuBlendState gpuCreateBlendState(GpuBlendDesc desc);
void gpuFreeDepthStencilState(GpuDepthStencilState state);
void gpuFreeBlendState(GpuBlendState state);

// Queue
GpuQueue gpuCreateQueue();
GpuCommandBuffer gpuStartCommandRecording(GpuQueue queue);
void gpuSubmit(GpuQueue queue, Span<GpuCommandBuffer> commandBuffers);

// Semaphores
GpuSemaphore gpuCreateSemaphore(uint64 initValue);
void gpuWaitSemaphore(GpuSemaphore sema, uint64 value);
void gpuDestroySemaphore(GpuSemaphore sema);

// Commands
void gpuMemCpy(GpuCommandBuffer cb, void *destGpu, void *srcGpu);
void gpuCopyToTexture(GpuCommandBuffer cb, void *destGpu, void *srcGpu, GpuTexture texture);
void gpuCopyFromTexture(GpuCommandBuffer cb, void *destGpu, void *srcGpu, GpuTexture texture);

void gpuSetActiveTextureHeapPtr(GpuCommandBuffer cb, void *ptrGpu);

void gpuBarrier(GpuCommandBuffer cb, STAGE before, STAGE after, HAZARD_FLAGS hazards = HAZARD_NONE);
void gpuSignalAfter(GpuCommandBuffer cb, STAGE before, void *ptrGpu, uint64 value, SIGNAL signal);
void gpuWaitBefore(GpuCommandBuffer cb, STAGE after, void *ptrGpu, uint64 value, OP op, HAZARD_FLAGS hazards = HAZARD_NONE, uint64 mask = ~uint64(0));

void gpuSetPipeline(GpuCommandBuffer cb, GpuPipeline pipeline);
void gpuSetDepthStencilState(GpuCommandBuffer cb, GpuDepthStencilState state);
void gpuSetBlendState(GpuCommandBuffer cb, GpuBlendState state);

void gpuDispatch(GpuCommandBuffer cb, void *dataGpu, uvec3 gridDimensions);
void gpuDispatchIndirect(GpuCommandBuffer cb, void *dataGpu, void *gridDimensionsGpu);

void gpuBeginRenderPass(GpuCommandBuffer cb, GpuRenderPassDesc desc);
void gpuEndRenderPass(GpuCommandBuffer cb);

void gpuDrawIndexedInstanced(GpuCommandBuffer cb, void *vertexDataGpu, void *pixelDataGpu, void *indicesGpu, uint32 indexCount, uint32 instanceCount);
void gpuDrawIndexedInstancedIndirect(GpuCommandBuffer cb, void *vertexDataGpu, void *pixelDataGpu, void *indicesGpu, void *argsGpu);
void gpuDrawIndexedInstancedIndirectMulti(GpuCommandBuffer cb, void *dataVxGpu, uint32 vxStride, void *dataPxGpu, uint32 pxStride, void *argsGpu, void *drawCountGpu);

void gpuDrawMeshlets(GpuCommandBuffer cb, void *meshletDataGpu, void *pixelDataGpu, uvec3 dim);
void gpuDrawMeshletsIndirect(GpuCommandBuffer cb, void *meshletDataGpu, void *pixelDataGpu, void *dimGpu);
