#include "render/renderer_opengl.hpp"

#include "gpu/gpu_api.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cassert>
#include <memory>

namespace
{
class RendererOpenGL final : public Renderer
{
  public:
    bool initialize(GLFWwindow *window) override
    {
        window_ = window;
        if (!window_)
        {
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        ImGui::StyleColorsDark();

        assert(ImGui_ImplGlfw_InitForOpenGL(window_, false));
        assert(ImGui_ImplOpenGL3_Init("#version 330"));

        queue_ = gpuCreateQueue();
        pipeline_ = gpuCreateGraphicsPipeline({}, {}, {});
        return true;
    }

    void renderFrame(float deltaTime) override
    {
        GpuCommandBuffer cb = gpuStartCommandRecording(queue_);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showDemoWindow_)
        {
            ImGui::ShowDemoWindow(&showDemoWindow_);
        }

        ImGui::Begin("Cross Platform Baseline");
        ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::Text("OpenGL renderer routed through gpu_api command submission.");
        ImGui::ColorEdit3("Clear Color", clearColor_);
        ImGui::End();

        ImGui::Render();

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);

        gpuSetPipeline(cb, pipeline_);
        Span<GpuCommandBuffer> list{&cb, 1};
        gpuSubmit(queue_, list);
    }

    void shutdown() override
    {
        gpuFreePipeline(pipeline_);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

  private:
    GLFWwindow *window_ = nullptr;
    bool showDemoWindow_ = true;
    float clearColor_[3] = {0.2f, 0.2f, 0.2f};

    GpuQueue queue_ = {};
    GpuPipeline pipeline_ = {};
};
} // namespace

std::unique_ptr<Renderer> createOpenGLRenderer()
{
    return std::make_unique<RendererOpenGL>();
}
