#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

// metal-cpp-extensions (modified)
#include <AppKit/AppKit.hpp>

static void quit(GLFWwindow *window, int key, int scancode, int action, int mods);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(1280, 720, "Hello World", nullptr, nullptr);

    auto nswindow = reinterpret_cast<NS::Window*>(glfwGetCocoaWindow(window));
    auto device = MTLCreateSystemDefaultDevice();
    auto commandQueue = device->newCommandQueue();
    auto layer = CA::MetalLayer::layer();
    layer->setDevice(device);
    auto nsview = nswindow->contentView();
    nsview->setLayer(layer);
    nsview->setWantsLayer(true);
    nsview->setOpaque(true);

    glfwSetKeyCallback(window, quit);
    MTL::ClearColor color = MTL::ClearColor::Make(1, 1, 1, 1);

    std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();

    
    while (!glfwWindowShouldClose(window)) {

        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        float FPS = 1 / deltaTime;

        glfwPollEvents();

        auto autoReleasePool = NS::AutoreleasePool::alloc()->init();
        
        color.red = color.red > 1.0 ? 0.0 : color.red + 0.01;
        printf("DeltaTime: %f, FPS: %f\n", deltaTime, FPS);

        auto surface = layer->nextDrawable();
        auto pass = MTL::RenderPassDescriptor::renderPassDescriptor();
        auto passColorAttachment0 = pass->colorAttachments()->object(0);
        passColorAttachment0->setClearColor(color);
        passColorAttachment0->setLoadAction(MTL::LoadActionClear);
        passColorAttachment0->setStoreAction(MTL::StoreActionStore);
        passColorAttachment0->setTexture(surface->texture());

        auto commandBuffer = commandQueue->commandBuffer();
        auto encoder = commandBuffer->renderCommandEncoder(pass);
        encoder->endEncoding();
        commandBuffer->presentDrawable(surface);
        commandBuffer->commit();

        autoReleasePool->release();
    }

    commandQueue->release();
    device->release();
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

static void quit(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
