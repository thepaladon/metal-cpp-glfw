#pragma once

#include <functional>
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

    void setGameRenderCallback(std::function<void()> callback)
    {
        gameRenderCallback_ = std::move(callback);
    }

  protected:
    std::function<void()> gameRenderCallback_;
};

std::unique_ptr<Renderer> createRenderer();
