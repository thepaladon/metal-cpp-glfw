# Cross-Platform Plan (macOS + Windows)

## Current setup
- macOS uses GLFW + Metal + ImGui (`imgui_impl_metal`).
- Windows/Linux use GLFW + OpenGL3 + ImGui (`imgui_impl_opengl3`) as a baseline path.

This keeps the project runnable on non-Apple platforms while preserving the existing Metal path.

## Recommended target architecture
- Keep one app loop + UI layer shared across platforms.
- Isolate platform/graphics-specific code behind a renderer interface.
- Build one renderer per backend:
  - `RendererMetal` for macOS.
  - `RendererD3D12` (preferred) or `RendererVulkan` for Windows parity with compute.
- Keep shader source and compute pipeline setup backend-specific but driven by the same high-level parameters.

## Suggested module boundaries
- `src/app/`:
  - frame state
  - UI logic
  - simulation/control variables (`time`, `refresh rate`, etc.)
- `src/render/`:
  - `IRenderer` interface (`initialize`, `resize`, `renderFrame`, `shutdown`)
  - backend implementations (`metal`, `d3d12` or `vulkan`, `opengl` fallback)
- `src/platform/`:
  - window creation/events if needed beyond GLFW defaults
- `src/gpu/`:
  - backend-agnostic API surface (`gpu_api.hpp`)
  - temporary stub backend (`gpu_api_stub.cpp`) to keep refactors unblocked

## Next implementation steps
1. Move current Metal render/compute code from `main.cpp` into `RendererMetal`.
2. Move OpenGL fallback path into `RendererOpenGL`.
3. Create shared frame/app state consumed by both renderers.
4. Add renderer selection in CMake (`-DRENDER_BACKEND=METAL|D3D12|OPENGL`).
5. Add CI matrix for macOS + Windows to ensure both build on every PR.

## Why this design
- Keeps platform-specific dependencies isolated.
- Prevents feature drift by sharing app state and UI logic.
- Lets Windows gain incremental parity without blocking macOS iteration.
