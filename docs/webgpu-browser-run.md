# WebGPU Browser Run Guide

## Build Target Separation
This repo has two different app targets:

1. `metalCppTest`:
- Native desktop target.
- Uses Metal on macOS, OpenGL3 on Windows/Linux.

2. `metalCppWeb`:
- Browser target.
- Uses WebGPU through Emscripten.

If you want browser output (`index.html` + `app.js` + `app.wasm`), always build `metalCppWeb`.

## Prerequisites
- Emscripten SDK installed and active in your shell.
- A browser with WebGPU enabled (latest Chrome, Edge, or Safari Technology Preview).

Activate Emscripten for the current shell:
```bash
source /path/to/emsdk/emsdk_env.sh
```

## Build
```bash
./scripts/generateProjectFiles.sh web
emmake make -C build/ProjectFiles/gmake config=debug metalCppWeb
```

Release build:
```bash
emmake make -C build/ProjectFiles/gmake config=release metalCppWeb
```

Generated files:
- `build/Web/Debug/index.html`
- `build/Web/Debug/app.js`
- `build/Web/Debug/app.wasm`

## Run Locally
Serve the build output over HTTP:
```bash
cd build/Web/Debug
python3 -m http.server 8080
```
Open:
- `http://localhost:8080`

## Why HTTP Is Required
- Browsers restrict WebAssembly fetch/instantiation from `file://` in many cases.
- WebGPU requires a secure context (`https://` or `http://localhost`).

## WASM MIME Type
Your host must serve `.wasm` as `application/wasm`.

## Per-User Settings Model
This build stores ImGui control settings per browser profile with `localStorage`:
- `aaWeb.uiScale`
- `aaWeb.clearR`
- `aaWeb.clearG`
- `aaWeb.clearB`

No account backend is required. Different browsers/devices naturally get separate settings.

## Troubleshooting
- If `app.wasm` fails to load, verify the server sets `application/wasm`.
- If WebGPU creation fails, update browser or enable WebGPU feature flags.
- If canvas is blank, check browser dev tools for `WebGPU` or `wgpu` errors.
- If mouse works but typing does not, click once inside the canvas to focus it, then try typing again.
- If UI controls are missing, open the Dear ImGui window (`Cross Platform Baseline` / Demo) rendered inside the canvas.
