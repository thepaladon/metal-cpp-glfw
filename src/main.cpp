#include "render/renderer.hpp"

#include "imgui_impl_glfw.h"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdio>
#include <memory>

static void on_window_focus(GLFWwindow *window, int focused);
static void on_cursor_enter(GLFWwindow *window, int entered);
static void on_cursor_pos(GLFWwindow *window, double x, double y);
static void on_mouse_button(GLFWwindow *window, int button, int action, int mods);
static void on_scroll(GLFWwindow *window, double xoffset, double yoffset);
static void on_key(GLFWwindow *window, int key, int scancode, int action, int mods);
static void on_char(GLFWwindow *window, unsigned int c);

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

    glfwSetWindowFocusCallback(window, on_window_focus);
    glfwSetCursorEnterCallback(window, on_cursor_enter);
    glfwSetCursorPosCallback(window, on_cursor_pos);
    glfwSetMouseButtonCallback(window, on_mouse_button);
    glfwSetScrollCallback(window, on_scroll);
    glfwSetKeyCallback(window, on_key);
    glfwSetCharCallback(window, on_char);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastTime = Clock::now();

    while (!glfwWindowShouldClose(window))
    {
        Clock::time_point now = Clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        glfwPollEvents();
        renderer->renderFrame(deltaTime);
    }

    renderer->shutdown();
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

#if defined(__APPLE__)
    if (key == GLFW_KEY_W && action == GLFW_PRESS && (mods & GLFW_MOD_SUPER))
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
#endif
}

static void on_char(GLFWwindow *window, unsigned int c)
{
    ImGui_ImplGlfw_CharCallback(window, c);
}
