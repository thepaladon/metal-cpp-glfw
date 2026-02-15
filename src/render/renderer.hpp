#pragma once

#include <memory>

struct GLFWwindow;

class Renderer
{
  public:
    virtual ~Renderer() = default;

    virtual bool initialize(GLFWwindow *window) = 0;
    virtual void renderFrame(float deltaTime) = 0;
    virtual void shutdown() = 0;
};

std::unique_ptr<Renderer> createRenderer();
