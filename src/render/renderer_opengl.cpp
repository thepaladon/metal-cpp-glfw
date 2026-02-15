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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiStyle &style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg].w = 0.0f;
        baseStyle_ = ImGui::GetStyle();
        applyUiScale();

        assert(ImGui_ImplGlfw_InitForOpenGL(window_, true));
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
        ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockFlags);

        if (showDemoWindow_)
        {
            ImGui::ShowDemoWindow(&showDemoWindow_);
        }

        ImGui::Begin("Cross Platform Baseline");
        ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::Text("OpenGL renderer routed through gpu_api command submission.");
        ImGui::ColorEdit3("Clear Color", clearColor_);
        if (ImGui::SliderFloat("UI Scale", &uiScale_, 0.5f, 3.0f, "%.2fx"))
        {
            applyUiScale();
        }
        ImGui::End();

        ImGui::Render();
        ImGuiIO &io = ImGui::GetIO();

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backupCurrentContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentContext);
        }

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
    void applyUiScale()
    {
        ImGuiStyle scaledStyle = baseStyle_;
        scaledStyle.ScaleAllSizes(uiScale_);
        ImGui::GetStyle() = scaledStyle;
        ImGui::GetIO().FontGlobalScale = uiScale_;
    }

    GLFWwindow *window_ = nullptr;
    bool showDemoWindow_ = true;
    float clearColor_[3] = {0.2f, 0.2f, 0.2f};
    float uiScale_ = 1.0f;
    ImGuiStyle baseStyle_ = {};

    GpuQueue queue_ = {};
    GpuPipeline pipeline_ = {};
};
} // namespace

std::unique_ptr<Renderer> createOpenGLRenderer()
{
    return std::make_unique<RendererOpenGL>();
}
