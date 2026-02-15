#include "render/renderer.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdio>
#include <memory>

static void glfw_error_callback(int error, const char *description)
{
    std::fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return 1;
    }

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow *window = glfwCreateWindow(1280, 720, "metal-cpp-glfw", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    std::unique_ptr<Renderer> renderer = createRenderer();
    if (!renderer || !renderer->initialize(window))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastTime = Clock::now();
    bool escWasDown = false;
#if defined(__APPLE__)
    bool cmdWWasDown = false;
#endif

    while (!glfwWindowShouldClose(window))
    {
        Clock::time_point now = Clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        glfwPollEvents();

        const bool escDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        if (escDown && !escWasDown)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        escWasDown = escDown;

#if defined(__APPLE__)
        const bool wDown = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        const bool cmdDown = glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                             glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
        const bool cmdWDown = wDown && cmdDown;
        if (cmdWDown && !cmdWWasDown)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        cmdWWasDown = cmdWDown;
#endif

        renderer->renderFrame(deltaTime);
    }

    renderer->shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
