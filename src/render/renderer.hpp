#pragma once

#include <memory>

#if !defined(__EMSCRIPTEN__)
struct GLFWwindow;
#endif

class Renderer
{
  public:
    virtual ~Renderer() = default;

#if defined(__EMSCRIPTEN__)
    virtual bool initialize() = 0;
#else
    virtual bool initialize(GLFWwindow *window) = 0;
#endif
    virtual void renderFrame(float deltaTime) = 0;
    virtual void shutdown() = 0;
};

std::unique_ptr<Renderer> createRenderer();
