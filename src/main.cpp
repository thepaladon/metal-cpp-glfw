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

// imgui crud:
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"

#include <chrono>
#include <cstdio>

static void on_window_focus(GLFWwindow *window, int focused);
static void on_cursor_enter(GLFWwindow *window, int entered);
static void on_cursor_pos(GLFWwindow *window, double x, double y);
static void on_mouse_button(GLFWwindow *window, int button, int action, int mods);
static void on_scroll(GLFWwindow *window, double xoffset, double yoffset);
static void on_key(GLFWwindow *window, int key, int scancode, int action, int mods);
static void on_char(GLFWwindow *window, unsigned int c);

static const char *kComputeMSL = R"METAL(
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

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Hello World", nullptr, nullptr);
    glfwFocusWindow(window);

    NS::Window *nswindow = reinterpret_cast<NS::Window *>(glfwGetCocoaWindow(window));
    MTL::Device *device = MTLCreateSystemDefaultDevice();
    MTL::CommandQueue *commandQueue = device->newCommandQueue();

    CA::MetalLayer *layer = CA::MetalLayer::layer();
    layer->setDevice(device);
    // ToDo: C/CPP extension errors here because it can't find the enum without the proper includes.
    layer->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);

    // Required for compute-write into the drawable texture.
    layer->setFramebufferOnly(false);

    NS::View *nsview = nswindow->contentView();
    nsview->setLayer(layer);
    nsview->setWantsLayer(true);
    nsview->setOpaque(false);

    // Imgui Init:
    IMGUI_CHECKVERSION();
    ImGuiContext *ctx = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup style
    ImGui::StyleColorsDark();

    assert(ImGui_ImplGlfw_InitForOther(window, false));
    assert(ImGui_ImplMetal_Init(device));
    glfwSetWindowFocusCallback(window, on_window_focus);
    glfwSetCursorEnterCallback(window, on_cursor_enter);
    glfwSetCursorPosCallback(window, on_cursor_pos);
    glfwSetMouseButtonCallback(window, on_mouse_button);
    glfwSetScrollCallback(window, on_scroll);
    glfwSetKeyCallback(window, on_key);
    glfwSetCharCallback(window, on_char);

    // Build compute pipeline once (expensive-ish, do it at init).
    MTL::Library *library = nullptr;
    MTL::Function *kernelFn[2] = {nullptr, nullptr};
    MTL::ComputePipelineState *cps[2] = {nullptr, nullptr};
    {
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        NS::Error *err = nullptr;
        NS::String *src = NS::String::string(kComputeMSL, NS::UTF8StringEncoding);
        library = device->newLibrary(src, nullptr, &err);
        if (!library)
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newLibrary failed: %s\n", msg);
            return 1;
        }

        NS::String *fnName = NS::String::string("fill_texture", NS::UTF8StringEncoding);
        kernelFn[0] = library->newFunction(fnName);
        if (!kernelFn[0])
        {
            std::printf("newFunction failed\n");
            return 1;
        }

        fnName = NS::String::string("fill_texture2", NS::UTF8StringEncoding);
        kernelFn[1] = library->newFunction(fnName);
        if (!kernelFn[1])
        {
            std::printf("newFunction2 failed\n");
            return 1;
        }

        cps[0] = device->newComputePipelineState(kernelFn[0], &err);
        if (!cps[0])
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newComputePipelineState failed: %s\n", msg);
            return 1;
        }

        cps[1] = device->newComputePipelineState(kernelFn[1], &err);
        if (!cps[1])
        {
            const char *msg = err ? err->localizedDescription()->utf8String() : "unknown";
            std::printf("newComputePipelineState2 failed: %s\n", msg);
            return 1;
        }

        pool->release();
    }

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastTime = Clock::now();
    float t = 0.0f;

    bool showDemoWindow = true;
    MTL::RenderPassDescriptor *imguiRPD = MTL::RenderPassDescriptor::renderPassDescriptor();
    imguiRPD->alloc();

    uint32_t frame = 0;
    while (!glfwWindowShouldClose(window))
    {
        Clock::time_point now = Clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        t += deltaTime;

        printf("Frame Time: %.3f ms (%.1f FPS)\n", deltaTime * 1000.0f, 1.0f / deltaTime);

        // test this wth printf
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
        {
            printf("Up Arrow is down\n");
        }

        glfwPollEvents();
        //
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
        //
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        layer->setDrawableSize(CGSizeMake(width, height));
        //
        CA::MetalDrawable *drawable = layer->nextDrawable();
        if (!drawable)
        {
            pool->release();
            continue;
        }
        MTL::Texture *outTex = drawable->texture();

        MTL::CommandBuffer *cb = commandQueue->commandBuffer();

        // Start the Dear ImGui frame
        imguiRPD->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 1.0, 1.0));
        imguiRPD->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
        imguiRPD->colorAttachments()->object(0)->setTexture(outTex);
        imguiRPD->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);

        ImGui_ImplMetal_NewFrame(imguiRPD);
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

// Alternate between the two compute kernels.
#if 1
        MTL::ComputeCommandEncoder *ce = cb->computeCommandEncoder();

        uint32_t kernelIndex = frame % 2;
        frame++;
        ce->setComputePipelineState(cps[kernelIndex]);
        ce->setTexture(outTex, 0);

        static float t = 0.0f;
        static float refreshRate = 1.0f / 60.0f;

        ImGui::Begin("Control Panel");
        ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::SliderFloat("Time", &t, 0.0f, 10.0f);
        // refresh rate in fps editable
        ImGui::SliderFloat("Refresh Rate (FPS)", &refreshRate, 1.0f / 240.0f, 1.0f);

        ImGui::Text("Using Kernel: %s", kernelIndex == 0 ? "fill_texture" : "fill_texture2");
        ImGui::End();

        ce->setBytes(&t, sizeof(t), 0);

        // Grid = full texture
        MTL::Size grid = MTL::Size::Make(outTex->width(), outTex->height(), 1);

        // Threadgroup sizing guidance: align to threadExecutionWidth.
        NS::UInteger tgW = cps[kernelIndex]->threadExecutionWidth();
        NS::UInteger tgH = cps[kernelIndex]->maxTotalThreadsPerThreadgroup() / tgW;
        if (tgH == 0)
            tgH = 1;
        if (tgH > 16)
            tgH = 16;

        MTL::Size tgs = MTL::Size::Make(tgW, tgH, 1);
        ce->dispatchThreads(grid, tgs);

        
        ce->endEncoding();
#endif
        // End of compute

        // Start of imgui render
        MTL::RenderCommandEncoder *imguiEncoder = cb->renderCommandEncoder(imguiRPD);
        ImGui::Render();
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, imguiEncoder);
        imguiEncoder->endEncoding();

        cb->presentDrawableAfterMinimumDuration(drawable, refreshRate);
        cb->commit();

        pool->release();
    }

    for (int i = 0; i < 2; i++)
    {
        cps[i]->release();
        kernelFn[i]->release();
    }

    library->release();

    imguiRPD->release();
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    commandQueue->release();
    device->release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

static void on_window_focus(GLFWwindow *window, int focused)
{
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}

static void on_cursor_enter(GLFWwindow *window, int entered)
{
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}

static void on_cursor_pos(GLFWwindow *window, double x, double y)
{
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);
}

static void on_mouse_button(GLFWwindow *window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

static void on_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

static void on_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // Close on Cmd+W as well, like a normal macOS app.
    if (key == GLFW_KEY_W && action == GLFW_PRESS && (mods & GLFW_MOD_SUPER))
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void on_char(GLFWwindow *window, unsigned int c)
{
    ImGui_ImplGlfw_CharCallback(window, c);
}
