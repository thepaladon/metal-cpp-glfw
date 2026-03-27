#include "render/renderer_opengl.hpp"

#include "core/aa_memory_tracker.hpp"
#include "gpu/gpu_api.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>
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

        if (!ImGui_ImplGlfw_InitForOpenGL(window_, true))
        {
            std::fprintf(stderr, "ImGui_ImplGlfw_InitForOpenGL failed\n");
            ImGui::DestroyContext();
            return false;
        }
        if (!ImGui_ImplOpenGL3_Init("#version 330"))
        {
            std::fprintf(stderr, "ImGui_ImplOpenGL3_Init failed\n");
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

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

        if (gameRenderCallback_)
        {
            gameRenderCallback_();
        }

#if AA_CFG_MEMORY_TRACKING && AA_CFG_MEMORY_TRACKING_UI
        ImGui::Begin("Memory Tracker");
        const aa::AAMemoryStats memStats = aa::AAMemoryTrackerGetStats();
        auto memoryUnitText = [](aa::u64 bytes, char *out, aa::usize outSize) {
            static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
            double value = static_cast<double>(bytes);
            aa::usize unitIndex = 0;
            while (value >= 1024.0 && unitIndex < 4)
            {
                value /= 1024.0;
                ++unitIndex;
            }
            snprintf(out, static_cast<size_t>(outSize), "%.2f %s", value, units[unitIndex]);
        };

        char liveBytesText[32] = {};
        char peakBytesText[32] = {};
        memoryUnitText(memStats.liveBytes, liveBytesText, sizeof(liveBytesText));
        memoryUnitText(memStats.peakBytes, peakBytesText, sizeof(peakBytesText));

        ImGui::Text("Live Bytes: %s", liveBytesText);
        ImGui::Text("Live Allocs: %llu", static_cast<unsigned long long>(memStats.liveCount));
        ImGui::Text("Peak Bytes: %s", peakBytesText);
        ImGui::Text("Total Allocs: %llu", static_cast<unsigned long long>(memStats.totalAllocs));
        ImGui::Text("Total Frees: %llu", static_cast<unsigned long long>(memStats.totalFrees));
        ImGui::Text("Dropped Records: %llu", static_cast<unsigned long long>(memStats.droppedRecords));

        static bool taggedOnly = false;
        ImGui::Checkbox("Tagged Only", &taggedOnly);

        aa::AAMemoryGroupInfo groups[64] = {};
        const aa::usize groupCount = aa::AAMemoryTrackerCollectGroups(groups, 64, taggedOnly);
        ImGui::Separator();
        ImGui::Text("Group Sample: %llu", static_cast<unsigned long long>(groupCount));
        for (aa::usize i = 0; i < groupCount; ++i)
        {
            const aa::AAMemoryGroupInfo &group = groups[i];
            char symbol[256] = {};
            char groupBytesText[32] = {};
            const char *symbolName = aa::AAMemoryTrackerSymbolize(group.frame0, symbol, sizeof(symbol));
            memoryUnitText(group.liveBytes, groupBytesText, sizeof(groupBytesText));
            ImGui::Text("bytes=%s count=%llu tag=%s",
                        groupBytesText,
                        static_cast<unsigned long long>(group.liveCount),
                        group.tag != nullptr ? group.tag : "n/a");
            if (group.file != nullptr)
            {
                ImGui::Text("  file %s:%d", group.file, group.line);
            }
            if (symbolName != nullptr)
            {
                ImGui::Text("  fn %s", symbolName);
            }
            else
            {
                ImGui::Text("  frame0 %p", group.frame0);
            }
        }
        ImGui::End();
#endif

        ImGui::Render();

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO &io = ImGui::GetIO();
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
