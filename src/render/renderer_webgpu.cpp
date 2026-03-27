#include "render/renderer_webgpu.hpp"

#include "core/aa_types.hpp"
#include "imgui.h"
#include "imgui_impl_wgpu.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>

#include <cfloat>
#include <cmath>
#include <cstring>

namespace
{
const char *aaCanvasEventTarget = "#canvas";
const char *aaKeyboardEventTarget = EMSCRIPTEN_EVENT_TARGET_DOCUMENT;
constexpr bool aaUseCapture = true;

EM_JS(double, aaWebReadSettingValue, (const char *settingKey, double fallbackValue), {
    try {
        const keyText = UTF8ToString(settingKey);
        const rawValue = localStorage.getItem(keyText);
        if (rawValue === null) {
            return fallbackValue;
        }

        const parsedValue = Number(rawValue);
        if (Number.isFinite(parsedValue)) {
            return parsedValue;
        }
    } catch (error) {
    }

    return fallbackValue;
});

EM_JS(void, aaWebWriteSettingValue, (const char *settingKey, double value), {
    try {
        const keyText = UTF8ToString(settingKey);
        localStorage.setItem(keyText, String(value));
    } catch (error) {
    }
});

EM_JS(void, aaWebPrepareCanvasInput, (), {
    const canvasElement = document.getElementById("canvas");
    if (!canvasElement) {
        return;
    }

    canvasElement.setAttribute("tabindex", "0");
    canvasElement.style.outline = "none";

    if (!canvasElement.dataset.aaInputHooksInstalled) {
        canvasElement.addEventListener("pointerdown", () => {
            try {
                canvasElement.focus({ preventScroll: true });
            } catch (error) {
                canvasElement.focus();
            }
        });
        canvasElement.addEventListener("contextmenu", (event) => {
            event.preventDefault();
        });
        canvasElement.dataset.aaInputHooksInstalled = "1";
    }
});

EM_JS(void, aaWebFocusCanvas, (), {
    const canvasElement = document.getElementById("canvas");
    if (!canvasElement) {
        return;
    }

    try {
        canvasElement.focus({ preventScroll: true });
    } catch (error) {
        canvasElement.focus();
    }
});

inline int aaMapMouseButton(int buttonCode)
{
    switch (buttonCode)
    {
        case 0:
            return 0;
        case 1:
            return 2;
        case 2:
            return 1;
        default:
            return -1;
    }
}

ImGuiKey aaMapKeyCode(const char *code)
{
    if (code == nullptr)
    {
        return ImGuiKey_None;
    }

    if (strcmp(code, "Tab") == 0)
        return ImGuiKey_Tab;
    if (strcmp(code, "ArrowLeft") == 0)
        return ImGuiKey_LeftArrow;
    if (strcmp(code, "ArrowRight") == 0)
        return ImGuiKey_RightArrow;
    if (strcmp(code, "ArrowUp") == 0)
        return ImGuiKey_UpArrow;
    if (strcmp(code, "ArrowDown") == 0)
        return ImGuiKey_DownArrow;
    if (strcmp(code, "PageUp") == 0)
        return ImGuiKey_PageUp;
    if (strcmp(code, "PageDown") == 0)
        return ImGuiKey_PageDown;
    if (strcmp(code, "Home") == 0)
        return ImGuiKey_Home;
    if (strcmp(code, "End") == 0)
        return ImGuiKey_End;
    if (strcmp(code, "Insert") == 0)
        return ImGuiKey_Insert;
    if (strcmp(code, "Delete") == 0)
        return ImGuiKey_Delete;
    if (strcmp(code, "Backspace") == 0)
        return ImGuiKey_Backspace;
    if (strcmp(code, "Space") == 0)
        return ImGuiKey_Space;
    if (strcmp(code, "Enter") == 0)
        return ImGuiKey_Enter;
    if (strcmp(code, "NumpadEnter") == 0)
        return ImGuiKey_KeypadEnter;
    if (strcmp(code, "Escape") == 0)
        return ImGuiKey_Escape;
    if (strcmp(code, "ControlLeft") == 0)
        return ImGuiKey_LeftCtrl;
    if (strcmp(code, "ControlRight") == 0)
        return ImGuiKey_RightCtrl;
    if (strcmp(code, "ShiftLeft") == 0)
        return ImGuiKey_LeftShift;
    if (strcmp(code, "ShiftRight") == 0)
        return ImGuiKey_RightShift;
    if (strcmp(code, "AltLeft") == 0)
        return ImGuiKey_LeftAlt;
    if (strcmp(code, "AltRight") == 0)
        return ImGuiKey_RightAlt;
    if (strcmp(code, "MetaLeft") == 0)
        return ImGuiKey_LeftSuper;
    if (strcmp(code, "MetaRight") == 0)
        return ImGuiKey_RightSuper;

    if (code[0] == 'K' && code[1] == 'e' && code[2] == 'y' && code[3] >= 'A' && code[3] <= 'Z' && code[4] == '\0')
    {
        return static_cast<ImGuiKey>(ImGuiKey_A + (code[3] - 'A'));
    }
    if (code[0] == 'D' && code[1] == 'i' && code[2] == 'g' && code[3] == 'i' && code[4] == 't' && code[5] >= '0' &&
        code[5] <= '9' && code[6] == '\0')
    {
        return static_cast<ImGuiKey>(ImGuiKey_0 + (code[5] - '0'));
    }

    return ImGuiKey_None;
}

bool aaShouldInjectTextInput(const EmscriptenKeyboardEvent *eventData)
{
    if (eventData == nullptr)
    {
        return false;
    }
    if (eventData->ctrlKey || eventData->altKey || eventData->metaKey)
    {
        return false;
    }

    const char *keyText = eventData->key;
    if (keyText == nullptr || keyText[0] == '\0')
    {
        return false;
    }

    if (keyText[1] == '\0')
    {
        return true;
    }

    const unsigned char firstByte = static_cast<unsigned char>(keyText[0]);
    if ((firstByte & 0xE0) == 0xC0)
    {
        return keyText[1] != '\0' && keyText[2] == '\0';
    }
    if ((firstByte & 0xF0) == 0xE0)
    {
        return keyText[1] != '\0' && keyText[2] != '\0' && keyText[3] == '\0';
    }
    if ((firstByte & 0xF8) == 0xF0)
    {
        return keyText[1] != '\0' && keyText[2] != '\0' && keyText[3] != '\0' && keyText[4] == '\0';
    }

    return false;
}

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

        refreshSurfaceConfig();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::StyleColorsDark();
        baseStyle_ = ImGui::GetStyle();
        io.AddFocusEvent(true);

        uiScale_ = static_cast<float>(aaWebReadSettingValue("aaWeb.uiScale", 1.0));
        if (uiScale_ < 0.5f || uiScale_ > 3.0f)
        {
            uiScale_ = 1.0f;
        }
        clearColor_[0] = static_cast<float>(aaWebReadSettingValue("aaWeb.clearR", 0.2));
        clearColor_[1] = static_cast<float>(aaWebReadSettingValue("aaWeb.clearG", 0.2));
        clearColor_[2] = static_cast<float>(aaWebReadSettingValue("aaWeb.clearB", 0.2));
        for (int i = 0; i < 3; ++i)
        {
            if (clearColor_[i] < 0.0f)
            {
                clearColor_[i] = 0.0f;
            }
            if (clearColor_[i] > 1.0f)
            {
                clearColor_[i] = 1.0f;
            }
        }
        applyUiScale();

        ImGui_ImplWGPU_InitInfo initInfo = {};
        initInfo.Device = device_;
        initInfo.NumFramesInFlight = 2;
        initInfo.RenderTargetFormat = surfaceFormat_;
        initInfo.DepthStencilFormat = WGPUTextureFormat_Undefined;
        initInfo.PipelineMultisampleState.count = 1;
        if (!ImGui_ImplWGPU_Init(&initInfo))
        {
            ImGui::DestroyContext();
            return false;
        }

        aaWebPrepareCanvasInput();
        installInputCallbacks();
        aaWebFocusCanvas();
        return true;
    }

    void renderFrame(float deltaTime) override
    {
        refreshSurfaceConfig();

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(displayWidth_, displayHeight_);
        io.DisplayFramebufferScale = ImVec2(framebufferScaleX_, framebufferScaleY_);
        io.DeltaTime = deltaTime > 0.0f ? deltaTime : (1.0f / 60.0f);

        ImGui_ImplWGPU_NewFrame();
        ImGui::NewFrame();

        ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockFlags);

        if (showDemoWindow_)
        {
            ImGui::ShowDemoWindow(&showDemoWindow_);
        }

        ImGui::Begin("Cross Platform Baseline");
        ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / io.DeltaTime);
        ImGui::Text("WebGPU renderer with Dear ImGui.");

        if (ImGui::ColorEdit3("Clear Color", clearColor_))
        {
            settingsDirty_ = true;
        }

        if (ImGui::SliderFloat("UI Scale", &uiScale_, 0.5f, 3.0f, "%.2fx"))
        {
            applyUiScale();
            settingsDirty_ = true;
        }

        ImGui::InputText("Type here", inputBuffer_, sizeof(inputBuffer_));
        ImGui::End();

        if (settingsDirty_)
        {
            aaWebWriteSettingValue("aaWeb.uiScale", uiScale_);
            aaWebWriteSettingValue("aaWeb.clearR", clearColor_[0]);
            aaWebWriteSettingValue("aaWeb.clearG", clearColor_[1]);
            aaWebWriteSettingValue("aaWeb.clearB", clearColor_[2]);
            settingsDirty_ = false;
        }

        if (gameRenderCallback_)
        {
            gameRenderCallback_();
        }

        ImGui::Render();

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
        colorAttachment.clearValue = WGPUColor{clearColor_[0], clearColor_[1], clearColor_[2], 1.0};

        WGPURenderPassDescriptor renderPassDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        renderPassDescriptor.colorAttachmentCount = 1;
        renderPassDescriptor.colorAttachments = &colorAttachment;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
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
        uninstallInputCallbacks();

        ImGui_ImplWGPU_Shutdown();
        ImGui::DestroyContext();

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
    void applyUiScale()
    {
        ImGuiStyle scaledStyle = baseStyle_;
        scaledStyle.ScaleAllSizes(uiScale_);
        ImGui::GetStyle() = scaledStyle;
        ImGui::GetIO().FontGlobalScale = uiScale_;
    }

    void refreshSurfaceConfig()
    {
        double cssWidth = 0.0;
        double cssHeight = 0.0;
        if (emscripten_get_element_css_size(aaCanvasEventTarget, &cssWidth, &cssHeight) != EMSCRIPTEN_RESULT_SUCCESS)
        {
            cssWidth = 1280.0;
            cssHeight = 720.0;
        }

        if (cssWidth <= 0.0 || cssHeight <= 0.0)
        {
            cssWidth = 1280.0;
            cssHeight = 720.0;
        }

        const double deviceScale = emscripten_get_device_pixel_ratio();
        const double safeScale = deviceScale > 0.0 ? deviceScale : 1.0;
        const int pixelWidth = static_cast<int>(std::round(cssWidth * safeScale));
        const int pixelHeight = static_cast<int>(std::round(cssHeight * safeScale));

        if (pixelWidth <= 0 || pixelHeight <= 0)
        {
            return;
        }

        const aa::u32 newCanvasWidth = static_cast<aa::u32>(pixelWidth);
        const aa::u32 newCanvasHeight = static_cast<aa::u32>(pixelHeight);

        displayWidth_ = static_cast<float>(cssWidth);
        displayHeight_ = static_cast<float>(cssHeight);
        framebufferScaleX_ = displayWidth_ > 0.0f ? static_cast<float>(newCanvasWidth) / displayWidth_ : 1.0f;
        framebufferScaleY_ = displayHeight_ > 0.0f ? static_cast<float>(newCanvasHeight) / displayHeight_ : 1.0f;

        if (newCanvasWidth == canvasWidth_ && newCanvasHeight == canvasHeight_)
        {
            return;
        }

        canvasWidth_ = newCanvasWidth;
        canvasHeight_ = newCanvasHeight;
        emscripten_set_canvas_element_size(aaCanvasEventTarget, pixelWidth, pixelHeight);

        WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
        config.device = device_;
        config.format = surfaceFormat_;
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

    void addModifierEvents(const EmscriptenKeyboardEvent *eventData)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiMod_Ctrl, eventData->ctrlKey != 0);
        io.AddKeyEvent(ImGuiMod_Shift, eventData->shiftKey != 0);
        io.AddKeyEvent(ImGuiMod_Alt, eventData->altKey != 0);
        io.AddKeyEvent(ImGuiMod_Super, eventData->metaKey != 0);
    }

    static EM_BOOL onMouseMove(int, const EmscriptenMouseEvent *eventData, void *userData)
    {
        RendererWebGpu *renderer = static_cast<RendererWebGpu *>(userData);
        ImGui::GetIO().AddMousePosEvent(static_cast<float>(eventData->targetX), static_cast<float>(eventData->targetY));
        renderer->hovered_ = true;
        return EM_TRUE;
    }

    static EM_BOOL onMouseButtonDown(int, const EmscriptenMouseEvent *eventData, void *)
    {
        aaWebFocusCanvas();
        ImGui::GetIO().AddFocusEvent(true);
        ImGui::GetIO().AddMousePosEvent(static_cast<float>(eventData->targetX), static_cast<float>(eventData->targetY));

        const int mappedButton = aaMapMouseButton(eventData->button);
        if (mappedButton >= 0)
        {
            ImGui::GetIO().AddMouseButtonEvent(mappedButton, true);
        }
        return EM_TRUE;
    }

    static EM_BOOL onMouseButtonUp(int, const EmscriptenMouseEvent *eventData, void *)
    {
        ImGui::GetIO().AddMousePosEvent(static_cast<float>(eventData->targetX), static_cast<float>(eventData->targetY));

        const int mappedButton = aaMapMouseButton(eventData->button);
        if (mappedButton >= 0)
        {
            ImGui::GetIO().AddMouseButtonEvent(mappedButton, false);
        }
        return EM_TRUE;
    }

    static EM_BOOL onMouseWheel(int, const EmscriptenWheelEvent *eventData, void *)
    {
        ImGui::GetIO().AddMouseWheelEvent(static_cast<float>(eventData->deltaX * 0.01),
                                          static_cast<float>(-eventData->deltaY * 0.01));
        return EM_TRUE;
    }

    static EM_BOOL onMouseLeave(int, const EmscriptenMouseEvent *, void *userData)
    {
        RendererWebGpu *renderer = static_cast<RendererWebGpu *>(userData);
        renderer->hovered_ = false;
        ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        return EM_TRUE;
    }

    static EM_BOOL onKeyDown(int, const EmscriptenKeyboardEvent *eventData, void *userData)
    {
        RendererWebGpu *renderer = static_cast<RendererWebGpu *>(userData);
        renderer->addModifierEvents(eventData);

        ImGuiIO &io = ImGui::GetIO();
        ImGuiKey key = aaMapKeyCode(eventData->code);
        if (key != ImGuiKey_None)
        {
            io.AddKeyEvent(key, true);
        }

        if (aaShouldInjectTextInput(eventData))
        {
            io.AddInputCharactersUTF8(eventData->key);
        }

        return EM_TRUE;
    }

    static EM_BOOL onKeyUp(int, const EmscriptenKeyboardEvent *eventData, void *userData)
    {
        RendererWebGpu *renderer = static_cast<RendererWebGpu *>(userData);
        renderer->addModifierEvents(eventData);

        ImGuiKey key = aaMapKeyCode(eventData->code);
        if (key != ImGuiKey_None)
        {
            ImGui::GetIO().AddKeyEvent(key, false);
        }

        return EM_TRUE;
    }

    static EM_BOOL onFocusOut(int, const EmscriptenFocusEvent *, void *)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddFocusEvent(false);
        io.AddMouseButtonEvent(0, false);
        io.AddMouseButtonEvent(1, false);
        io.AddMouseButtonEvent(2, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        io.AddKeyEvent(ImGuiMod_Shift, false);
        io.AddKeyEvent(ImGuiMod_Alt, false);
        io.AddKeyEvent(ImGuiMod_Super, false);
        return EM_TRUE;
    }

    static EM_BOOL onFocusIn(int, const EmscriptenFocusEvent *, void *)
    {
        ImGui::GetIO().AddFocusEvent(true);
        return EM_TRUE;
    }

    void installInputCallbacks()
    {
        emscripten_set_mousemove_callback(aaCanvasEventTarget, this, aaUseCapture, onMouseMove);
        emscripten_set_mousedown_callback(aaCanvasEventTarget, this, aaUseCapture, onMouseButtonDown);
        emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, aaUseCapture, onMouseButtonUp);
        emscripten_set_wheel_callback(aaCanvasEventTarget, this, aaUseCapture, onMouseWheel);
        emscripten_set_mouseleave_callback(aaCanvasEventTarget, this, aaUseCapture, onMouseLeave);

        emscripten_set_keydown_callback(aaKeyboardEventTarget, this, aaUseCapture, onKeyDown);
        emscripten_set_keyup_callback(aaKeyboardEventTarget, this, aaUseCapture, onKeyUp);
        emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, aaUseCapture, onFocusIn);
        emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, aaUseCapture, onFocusOut);
    }

    void uninstallInputCallbacks()
    {
        emscripten_set_mousemove_callback(aaCanvasEventTarget, nullptr, aaUseCapture, nullptr);
        emscripten_set_mousedown_callback(aaCanvasEventTarget, nullptr, aaUseCapture, nullptr);
        emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, aaUseCapture, nullptr);
        emscripten_set_wheel_callback(aaCanvasEventTarget, nullptr, aaUseCapture, nullptr);
        emscripten_set_mouseleave_callback(aaCanvasEventTarget, nullptr, aaUseCapture, nullptr);

        emscripten_set_keydown_callback(aaKeyboardEventTarget, nullptr, aaUseCapture, nullptr);
        emscripten_set_keyup_callback(aaKeyboardEventTarget, nullptr, aaUseCapture, nullptr);
        emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, aaUseCapture, nullptr);
        emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, aaUseCapture, nullptr);
    }

    WGPUInstance instance_ = nullptr;
    WGPUDevice device_ = nullptr;
    WGPUQueue queue_ = nullptr;
    WGPUSurface surface_ = nullptr;
    WGPUTextureFormat surfaceFormat_ = WGPUTextureFormat_BGRA8Unorm;

    aa::u32 canvasWidth_ = 0;
    aa::u32 canvasHeight_ = 0;
    float displayWidth_ = 1280.0f;
    float displayHeight_ = 720.0f;
    float framebufferScaleX_ = 1.0f;
    float framebufferScaleY_ = 1.0f;

    bool showDemoWindow_ = true;
    bool settingsDirty_ = false;
    bool hovered_ = false;

    float clearColor_[3] = {0.2f, 0.2f, 0.2f};
    float uiScale_ = 1.0f;
    ImGuiStyle baseStyle_ = {};
    char inputBuffer_[256] = "Type here";
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
