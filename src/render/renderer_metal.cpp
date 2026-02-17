#include "render/renderer_metal.hpp"

#include "core/aa_memory_tracker.hpp"
#include "gpu/gpu_api.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <Metal/MTLPixelFormat.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>
#include "imgui_impl_metal.h"

#include <chrono>
#include <cstdio>
#include <memory>

namespace
{
const char *kComputeMSL = R"METAL(
#include <metal_stdlib>
using namespace metal;

kernel void fill_texture(texture2d<float, access::write> outTex [[texture(0)]],
                         constant float& t [[buffer(0)]],
                         uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= outTex.get_width() || gid.y >= outTex.get_height()) return;

    float2 uv = float2(gid) / float2(outTex.get_width(), outTex.get_height());

    float r = fract(uv.x + t);
    float g = uv.y;
    float b = 0.25f + 0.25f * sin(t * 6.2831853f);

    outTex.write(float4(g, r, b, 1.0f), gid);
}

kernel void fill_texture2(texture2d<float, access::write> outTex [[texture(0)]],
                          constant float& t [[buffer(0)]],
                          uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= outTex.get_width() || gid.y >= outTex.get_height()) return;

    float2 uv = float2(gid) / float2(outTex.get_width(), outTex.get_height());

    float r = fract(uv.x + t);
    float g = uv.y;
    float b = 0.25f + 0.25f * sin(t * 6.2831853f);

    outTex.write(float4(r, g, b, 1.0f), gid);
}
)METAL";

class RendererMetal final : public Renderer
{
  public:
    bool initialize(GLFWwindow *window) override
    {
        window_ = window;
        if (!window_)
        {
            return false;
        }

        NS::Window *nswindow = reinterpret_cast<NS::Window *>(glfwGetCocoaWindow(window_));

        device_ = MTLCreateSystemDefaultDevice();
        if (!device_)
        {
            return false;
        }

        commandQueue_ = device_->newCommandQueue();
        if (!commandQueue_)
        {
            return false;
        }

        layer_ = CA::MetalLayer::layer();
        layer_->setDevice(device_);
        layer_->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
        layer_->setFramebufferOnly(false);

        NS::View *nsview = nswindow->contentView();
        nsview->setLayer(layer_);
        nsview->setWantsLayer(true);
        nsview->setOpaque(false);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiStyle &style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        baseStyle_ = ImGui::GetStyle();
        applyUiScale();

        if (!ImGui_ImplGlfw_InitForOther(window_, true))
        {
            std::printf("ImGui_ImplGlfw_InitForOther failed\n");
            ImGui::DestroyContext();
            return false;
        }
        if (!ImGui_ImplMetal_Init(device_))
        {
            std::printf("ImGui_ImplMetal_Init failed\n");
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        imguiRPD_ = MTL::RenderPassDescriptor::renderPassDescriptor();
        imguiRPD_->alloc();

        if (!buildComputePipeline())
        {
            return false;
        }

        gpuQueue_ = gpuCreateQueue();
        gpuPipeline_ = gpuCreateComputePipeline({});
        return true;
    }

    void renderFrame(float deltaTime) override
    {
        GpuCommandBuffer gpuCb = gpuStartCommandRecording(gpuQueue_);

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        layer_->setDrawableSize(CGSizeMake(width, height));

        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        CA::MetalDrawable *drawable = layer_->nextDrawable();
        if (!drawable)
        {
            pool->release();
            return;
        }

        MTL::Texture *outTex = drawable->texture();
        MTL::CommandBuffer *cb = commandQueue_->commandBuffer();

        imguiRPD_->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 1.0, 1.0));
        imguiRPD_->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
        imguiRPD_->colorAttachments()->object(0)->setTexture(outTex);
        imguiRPD_->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);

        ImGui_ImplMetal_NewFrame(imguiRPD_);
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        if (showDemoWindow_)
        {
            ImGui::ShowDemoWindow(&showDemoWindow_);
        }

        MTL::ComputeCommandEncoder *ce = cb->computeCommandEncoder();

        const uint32_t kernelIndex = frame_ % 2;
        frame_++;

        ce->setComputePipelineState(cps_[kernelIndex]);
        ce->setTexture(outTex, 0);

        ImGui::Begin("Control Panel");
        ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::SliderFloat("Time", &time_, 0.0f, 10.0f);
        ImGui::SliderFloat("Refresh Rate (FPS)", &refreshRate_, 1.0f / 240.0f, 1.0f);
        if (ImGui::SliderFloat("UI Scale", &uiScale_, 0.5f, 3.0f, "%.2fx"))
        {
            applyUiScale();
        }
        ImGui::Text("Using Kernel: %s", kernelIndex == 0 ? "fill_texture" : "fill_texture2");
        ImGui::InputText("Type here", inputBuf_, sizeof(inputBuf_));
        ImGui::End();

#if AA_CFG_MEMORY_TRACKING && AA_CFG_MEMORY_TRACKING_UI
        ImGui::Begin("Memory Tracker");
        const aa::AAMemoryStats memStats = aa::AAMemoryTrackerGetStats();
        auto memoryUnitText = [](aa::u64 bytes, char *out, aa::usize outSize) {
            static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
            double value = static_cast<double>(bytes);
            aa::usize unitIndex = 0;
            while (value >= 1024.0 && unitIndex < 4)
            {
                value /= 1024.0;
                ++unitIndex;
            }
            snprintf(out, static_cast<size_t>(outSize), "%.2f %s", value, units[unitIndex]);
        };

        char liveBytesText[32] = {};
        char peakBytesText[32] = {};
        memoryUnitText(memStats.liveBytes, liveBytesText, sizeof(liveBytesText));
        memoryUnitText(memStats.peakBytes, peakBytesText, sizeof(peakBytesText));

        ImGui::Text("Live Bytes: %s", liveBytesText);
        ImGui::Text("Live Allocs: %llu", static_cast<unsigned long long>(memStats.liveCount));
        ImGui::Text("Peak Bytes: %s", peakBytesText);
        ImGui::Text("Total Allocs: %llu", static_cast<unsigned long long>(memStats.totalAllocs));
        ImGui::Text("Total Frees: %llu", static_cast<unsigned long long>(memStats.totalFrees));
        ImGui::Text("Dropped Records: %llu", static_cast<unsigned long long>(memStats.droppedRecords));

        static bool taggedOnly = false;
        ImGui::Checkbox("Tagged Only", &taggedOnly);

        aa::AAMemoryGroupInfo groups[64] = {};
        const aa::usize groupCount = aa::AAMemoryTrackerCollectGroups(groups, 64, taggedOnly);
        ImGui::Separator();
        ImGui::Text("Group Sample: %llu", static_cast<unsigned long long>(groupCount));
        for (aa::usize i = 0; i < groupCount; ++i)
        {
            const aa::AAMemoryGroupInfo &group = groups[i];
            char symbol[256] = {};
            char groupBytesText[32] = {};
            const char *symbolName = aa::AAMemoryTrackerSymbolize(group.frame0, symbol, sizeof(symbol));
            memoryUnitText(group.liveBytes, groupBytesText, sizeof(groupBytesText));
            ImGui::Text("bytes=%s count=%llu tag=%s",
                        groupBytesText,
                        static_cast<unsigned long long>(group.liveCount),
                        group.tag != nullptr ? group.tag : "n/a");
            if (group.file != nullptr)
            {
                ImGui::Text("  file %s:%d", group.file, group.line);
            }
            if (symbolName != nullptr)
            {
                ImGui::Text("  fn %s", symbolName);
            }
            else
            {
                ImGui::Text("  frame0 %p", group.frame0);
            }
        }
        ImGui::End();
#endif

        ce->setBytes(&time_, sizeof(time_), 0);

        MTL::Size grid = MTL::Size::Make(outTex->width(), outTex->height(), 1);
        NS::UInteger tgW = cps_[kernelIndex]->threadExecutionWidth();
        NS::UInteger tgH = cps_[kernelIndex]->maxTotalThreadsPerThreadgroup() / tgW;
        if (tgH == 0)
        {
            tgH = 1;
        }
        if (tgH > 16)
        {
            tgH = 16;
        }

        MTL::Size tgs = MTL::Size::Make(tgW, tgH, 1);
        ce->dispatchThreads(grid, tgs);
        ce->endEncoding();

        MTL::RenderCommandEncoder *imguiEncoder = cb->renderCommandEncoder(imguiRPD_);
        ImGui::Render();
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, imguiEncoder);
        imguiEncoder->endEncoding();

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        cb->presentDrawableAfterMinimumDuration(drawable, refreshRate_);
        cb->commit();

        gpuSetPipeline(gpuCb, gpuPipeline_);
        Span<GpuCommandBuffer> submitList{&gpuCb, 1};
        gpuSubmit(gpuQueue_, submitList);

        pool->release();
    }

    void shutdown() override
    {
        gpuFreePipeline(gpuPipeline_);

        for (int i = 0; i < 2; i++)
        {
            if (cps_[i])
            {
                cps_[i]->release();
            }
            if (kernelFn_[i])
            {
                kernelFn_[i]->release();
            }
        }

        if (library_)
        {
            library_->release();
        }

        if (imguiRPD_)
        {
            imguiRPD_->release();
        }

        ImGui_ImplMetal_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (commandQueue_)
        {
            commandQueue_->release();
        }
        if (device_)
        {
            device_->release();
        }
    }

  private:
    void applyUiScale()
    {
        ImGuiStyle scaledStyle = baseStyle_;
        scaledStyle.ScaleAllSizes(uiScale_);
        ImGui::GetStyle() = scaledStyle;
        ImGui::GetIO().FontGlobalScale = uiScale_;
    }

    bool buildComputePipeline()
    {
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        NS::Error *err = nullptr;
        NS::String *src = NS::String::string(kComputeMSL, NS::UTF8StringEncoding);
        library_ = device_->newLibrary(src, nullptr, &err);
        if (!library_)
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newLibrary failed: %s\n", msg);
            pool->release();
            return false;
        }

        NS::String *fnName = NS::String::string("fill_texture", NS::UTF8StringEncoding);
        kernelFn_[0] = library_->newFunction(fnName);
        if (!kernelFn_[0])
        {
            std::printf("newFunction failed\n");
            pool->release();
            return false;
        }

        fnName = NS::String::string("fill_texture2", NS::UTF8StringEncoding);
        kernelFn_[1] = library_->newFunction(fnName);
        if (!kernelFn_[1])
        {
            std::printf("newFunction2 failed\n");
            pool->release();
            return false;
        }

        cps_[0] = device_->newComputePipelineState(kernelFn_[0], &err);
        if (!cps_[0])
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newComputePipelineState failed: %s\n", msg);
            pool->release();
            return false;
        }

        cps_[1] = device_->newComputePipelineState(kernelFn_[1], &err);
        if (!cps_[1])
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newComputePipelineState2 failed: %s\n", msg);
            pool->release();
            return false;
        }

        pool->release();
        return true;
    }

    GLFWwindow *window_ = nullptr;

    MTL::Device *device_ = nullptr;
    MTL::CommandQueue *commandQueue_ = nullptr;
    CA::MetalLayer *layer_ = nullptr;
    MTL::Library *library_ = nullptr;
    MTL::Function *kernelFn_[2] = {nullptr, nullptr};
    MTL::ComputePipelineState *cps_[2] = {nullptr, nullptr};
    MTL::RenderPassDescriptor *imguiRPD_ = nullptr;

    bool showDemoWindow_ = true;
    float time_ = 0.0f;
    float refreshRate_ = 1.0f / 144.0f;
    float uiScale_ = 1.0f;
    ImGuiStyle baseStyle_ = {};
    char inputBuf_[256] = "Type here for keyboard input test";
    uint32_t frame_ = 0;

    GpuQueue gpuQueue_ = {};
    GpuPipeline gpuPipeline_ = {};
};
} // namespace

std::unique_ptr<Renderer> createMetalRenderer()
{
    return std::make_unique<RendererMetal>();
}
