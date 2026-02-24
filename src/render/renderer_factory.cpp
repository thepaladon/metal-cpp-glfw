#include "render/renderer.hpp"

#if defined(__EMSCRIPTEN__)
#include "render/renderer_webgpu.hpp"
#elif defined(__APPLE__)
#include "render/renderer_metal.hpp"
#else
#include "render/renderer_opengl.hpp"
#endif


std::unique_ptr<Renderer> createRenderer()
{
#if defined(__EMSCRIPTEN__)
    return createWebGpuRenderer();
#elif defined(__APPLE__)
    return createMetalRenderer();
#else
    return createOpenGLRenderer();
#endif
}
