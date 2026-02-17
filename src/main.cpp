#include "core/aa_file.hpp"
#include "core/aa_memory.hpp"
#include "core/aa_platform.hpp"
#include "core/aa_types.hpp"
#include "render/renderer.hpp"

#include <GLFW/glfw3.h>

#include <cstdio>

using namespace aa;

static void glfw_error_callback(i32 error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    const AAPath cwd = AAGetCurrentWorkingDirectory();
    const AAPath buildDir = cwd.EndsWith("build") ? cwd : cwd.Join("build");
    const AAPath appCacheDir = buildDir.Join(".appcache");
    AACreateDirectories(appCacheDir);
    AAWriteAllText(appCacheDir.Join("hello.txt"), "hello world");

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

    AAPtr<Renderer> renderer = createRenderer();
    if (!renderer || !renderer->initialize(window))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    f64 lastTime = glfwGetTime();
    b8 escWasDown = false;
#if defined(__APPLE__)
    b8 cmdWWasDown = false;
#endif

    while (!glfwWindowShouldClose(window))
    {
        const f64 now = glfwGetTime();
        const f32 deltaTime = static_cast<f32>(now - lastTime);
        lastTime = now;

        glfwPollEvents();

        const b8 escDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        if (escDown && !escWasDown)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        escWasDown = escDown;

#if defined(__APPLE__)
        const b8 wDown = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        const b8 cmdDown = glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
        const b8 cmdWDown = wDown && cmdDown;
        if (cmdWDown && !cmdWWasDown)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        cmdWWasDown = cmdWDown;

        //i32* test = AA_NEW_TAG("testing") i32;
#endif

        renderer->renderFrame(deltaTime);
    }

    renderer->shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
