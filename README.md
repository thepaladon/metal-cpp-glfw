# Minimal GLFW metal-cpp Premake Example

This example uses `metal-cpp` and a couple of additions to `metal-cpp-extensions` in order to show a pure C++ GLFW / Metal implementation. It's a port of [this](https://gist.github.com/gcatlin/987be74e2d58da96093a7598f3fbfb27) Objective-C based example.

The `metal-cpp-extensions` code was taken from this [Apple sample](https://developer.apple.com/metal/LearnMetalCPP.zip) and a couple of methods were added to `NS::Window` and `NS::View` to allow Metal to be set-up, as this code does not use `MetalKit`.

Note that Apple do not support `metal-cpp-extensions` and there are a couple of extensions that have been made to it, however it's pretty simple to extend yourself.

## Build system

This project now uses `Premake5` instead of CMake.

### Double-click quick start

Windows:
- Double-click `scripts/01-Setup-And-Open-Windows.bat` to sync deps, generate Visual Studio files, and open the solution.
- Double-click `scripts/02-Build-All-Windows.bat` to sync deps, generate files, and build Debug + Release.

macOS:
- Double-click `scripts/01-Setup-And-Open-Mac.command` to sync deps, generate Xcode files, and open the workspace.
- Double-click `scripts/02-Build-All-Mac.command` to sync deps, generate Makefiles, and build Debug + Release.
- If Finder will not run the `.command` files, run this once in Terminal:
```bash
chmod +x scripts/01-Setup-And-Open-Mac.command scripts/02-Build-All-Mac.command
```

### First-time setup

1. Install `premake5` and ensure it is in `PATH`.
2. Fetch pinned third-party dependencies into `/thirdParty`.

Windows:
```powershell
./scripts/bootstrapDeps.ps1
```

macOS/Linux:
```bash
./scripts/bootstrapDeps.sh
```

### Generate project files

Windows (Visual Studio 2022):
```powershell
./scripts/generateProjectFiles.ps1 -Action vs2022
```

macOS (Xcode):
```bash
./scripts/generateProjectFiles.sh xcode4
```

Linux/macOS (GNU Make):
```bash
./scripts/generateProjectFiles.sh gmake2
```

Generated project files go to `build/ProjectFiles/<action>`.
Build outputs are kept in:
- `build/Build/<Config>` for runnable binaries
- `build/Intermediate/<Config>` for object files and static libs

### Build

Windows:
- Open `build/ProjectFiles/vs2022/metalCppGlfw.sln` in Visual Studio and build `metalCppTest`.

macOS/Linux with GNU Make:
```bash
make -C build/ProjectFiles/gmake2 config=debug
make -C build/ProjectFiles/gmake2 config=release
```

## Cross-platform status

- macOS path: GLFW + Metal + ImGui.
- Windows/Linux path: GLFW + OpenGL3 + ImGui baseline.

See `/docs/cross-platform-plan.md` for the architecture and migration plan toward full Windows parity.

## GPU API scaffold

Initial cross-platform graphics API scaffolding lives in:
- `/src/gpu/gpu_api.hpp`
- `/src/gpu/gpu_api_stub.cpp`

The stub backend currently returns placeholder handles/no-op behavior so higher-level refactors can proceed before backend-specific implementations are complete.
