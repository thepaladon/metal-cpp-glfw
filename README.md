# Minimal GLFW metal-cpp cmake example

This example uses `metal-cpp` and a couple of additions to `metal-cpp-extensions` in order to show a pure C++ GLFW / Metal implementation.  It's really a port of [this](https://gist.github.com/gcatlin/987be74e2d58da96093a7598f3fbfb27) Objective-C based example.

The `metal-cpp-extensions` code was taken from this [Apple sample](https://developer.apple.com/metal/LearnMetalCPP.zip) and a couple of methods were added to `NS::Window` and `NS::View` to allow Metal to be set-up, as this code does not use `MetalKit`.

Note that Apple do not support `metal-cpp-extensions` and there are a couple of extensions that have been made to it, however it's pretty simple to extend yourself.

## Bootstrap

- Windows: `./setupWindows.cmd`
- macOS: `./setupMacOs.sh`

Both scripts install/check required tools and run configure + Debug build.

## Cross-platform status

- macOS path: GLFW + Metal + ImGui.
- Windows/Linux path: GLFW + OpenGL3 + ImGui baseline.

See `/docs/cross-platform-plan.md` for the architecture and migration plan toward full Windows parity.

## GPU API scaffold

Initial cross-platform graphics API scaffolding lives in:
- `/src/gpu/gpu_api.hpp`
- `/src/gpu/gpu_api_stub.cpp`

The stub backend currently returns placeholder handles/no-op behavior so higher-level refactors can proceed before backend-specific implementations are complete.
