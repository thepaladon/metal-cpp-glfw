#include "render/renderer_webgpu.hpp"

#include "core/aa_types.hpp"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>

namespace
{
EM_JS(double, aaWebReadControlValue, (const char *elementId, const char *storageKey, double fallback), {
    try {
        const idText = UTF8ToString(elementId);
        const controlElement = document.getElementById(idText);
        if (controlElement && controlElement.value !== undefined) {
            const controlValue = Number(controlElement.value);
            if (Number.isFinite(controlValue)) {
                return controlValue;
            }
        }

        const keyText = UTF8ToString(storageKey);
        const storedValue = localStorage.getItem(keyText);
        if (storedValue === null) {
            return fallback;
        }

        const parsedValue = Number(storedValue);
        if (Number.isFinite(parsedValue)) {
            return parsedValue;
        }
    } catch (error) {
    }
    return fallback;
});

EM_JS(void, aaWebReportRenderState, (aa::u32 frameIndex, double clearR, double clearG, double clearB), {
    const stateElement = document.getElementById("renderState");
    if (stateElement) {
        stateElement.textContent =
            `Renderer frame ${frameIndex}: clear(${clearR.toFixed(2)}, ${clearG.toFixed(2)}, ${clearB.toFixed(2)})`;
    }
});

class RendererWebGpu final : public Renderer
{
  public:
    bool initialize() override
    {
        instance_ = wgpuCreateInstance(nullptr);
        if (instance_ == nullptr)
        {
            return false;
        }

        device_ = emscripten_webgpu_get_device();
        if (device_ == nullptr)
        {
            return false;
        }

        queue_ = wgpuDeviceGetQueue(device_);
        if (queue_ == nullptr)
        {
            return false;
        }

        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDescriptor =
            WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
        canvasDescriptor.selector.data = "#canvas";
        canvasDescriptor.selector.length = WGPU_STRLEN;

        WGPUSurfaceDescriptor surfaceDescriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
        surfaceDescriptor.nextInChain = &canvasDescriptor.chain;
        surface_ = wgpuInstanceCreateSurface(instance_, &surfaceDescriptor);
        if (surface_ == nullptr)
        {
            return false;
        }

        uiScale_ = static_cast<float>(aaWebReadControlValue("uiScale", "aaWeb.uiScale", 1.0));
        clearColorR_ = static_cast<float>(aaWebReadControlValue("clearR", "aaWeb.clearR", 0.2));
        clearColorG_ = static_cast<float>(aaWebReadControlValue("clearG", "aaWeb.clearG", 0.2));
        clearColorB_ = static_cast<float>(aaWebReadControlValue("clearB", "aaWeb.clearB", 0.2));
        aaWebReportRenderState(frameIndex_, clearColorR_, clearColorG_, clearColorB_);
        ++frameIndex_;

        refreshSurfaceConfig();
        return true;
    }

    void renderFrame(float) override
    {
        refreshSurfaceConfig();

        uiScale_ = static_cast<float>(aaWebReadControlValue("uiScale", "aaWeb.uiScale", 1.0));
        clearColorR_ = static_cast<float>(aaWebReadControlValue("clearR", "aaWeb.clearR", 0.2));
        clearColorG_ = static_cast<float>(aaWebReadControlValue("clearG", "aaWeb.clearG", 0.2));
        clearColorB_ = static_cast<float>(aaWebReadControlValue("clearB", "aaWeb.clearB", 0.2));

        WGPUSurfaceTexture surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
        wgpuSurfaceGetCurrentTexture(surface_, &surfaceTexture);
        const bool gotSurfaceTexture =
            surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal ||
            surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal;
        if (surfaceTexture.texture == nullptr || !gotSurfaceTexture)
        {
            return;
        }

        WGPUTextureView textureView = wgpuTextureCreateView(surfaceTexture.texture, nullptr);

        WGPUCommandEncoderDescriptor encoderDescriptor = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
        WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device_, &encoderDescriptor);

        WGPURenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        colorAttachment.view = textureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = WGPUColor{clearColorR_, clearColorG_, clearColorB_, 1.0f};

        WGPURenderPassDescriptor renderPassDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        renderPassDescriptor.colorAttachmentCount = 1;
        renderPassDescriptor.colorAttachments = &colorAttachment;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
        wgpuRenderPassEncoderEnd(renderPass);

        WGPUCommandBufferDescriptor commandBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, &commandBufferDescriptor);
        wgpuQueueSubmit(queue_, 1, &commandBuffer);

        wgpuCommandBufferRelease(commandBuffer);
        wgpuRenderPassEncoderRelease(renderPass);
        wgpuCommandEncoderRelease(commandEncoder);
        wgpuTextureViewRelease(textureView);
        wgpuTextureRelease(surfaceTexture.texture);
    }

    void shutdown() override
    {
        if (surface_ != nullptr)
        {
            wgpuSurfaceRelease(surface_);
            surface_ = nullptr;
        }

        if (queue_ != nullptr)
        {
            wgpuQueueRelease(queue_);
            queue_ = nullptr;
        }

        if (device_ != nullptr)
        {
            wgpuDeviceRelease(device_);
            device_ = nullptr;
        }

        if (instance_ != nullptr)
        {
            wgpuInstanceRelease(instance_);
            instance_ = nullptr;
        }
    }

  private:
    void refreshSurfaceConfig()
    {
        int width = 0;
        int height = 0;
        emscripten_get_canvas_element_size("#canvas", &width, &height);

        if (width <= 0 || height <= 0)
        {
            width = 1280;
            height = 720;
            emscripten_set_canvas_element_size("#canvas", width, height);
        }

        if (static_cast<aa::u32>(width) == canvasWidth_ && static_cast<aa::u32>(height) == canvasHeight_)
        {
            return;
        }

        canvasWidth_ = static_cast<aa::u32>(width);
        canvasHeight_ = static_cast<aa::u32>(height);

        WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
        config.device = device_;
        config.format = WGPUTextureFormat_BGRA8Unorm;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.width = canvasWidth_;
        config.height = canvasHeight_;
        config.presentMode = WGPUPresentMode_Fifo;
#if defined(WGPUCompositeAlphaMode_Auto)
        config.alphaMode = WGPUCompositeAlphaMode_Auto;
#endif
#if defined(WGPUTextureFormat_Undefined)
        config.viewFormatCount = 0;
        config.viewFormats = nullptr;
#endif

        wgpuSurfaceConfigure(surface_, &config);
    }

    WGPUInstance instance_ = nullptr;
    WGPUDevice device_ = nullptr;
    WGPUQueue queue_ = nullptr;
    WGPUSurface surface_ = nullptr;

    aa::u32 canvasWidth_ = 0;
    aa::u32 canvasHeight_ = 0;

    float uiScale_ = 1.0f;
    float clearColorR_ = 0.2f;
    float clearColorG_ = 0.2f;
    float clearColorB_ = 0.2f;
    aa::u32 frameIndex_ = 0;
};
} // namespace

std::unique_ptr<Renderer> createWebGpuRenderer()
{
    return std::make_unique<RendererWebGpu>();
}
#else
std::unique_ptr<Renderer> createWebGpuRenderer()
{
    return nullptr;
}
#endif
