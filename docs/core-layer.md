# Core Layer Plan

## Goal
Provide one reusable `src/core` module that isolates platform concerns and type/container policy from gameplay/rendering code.

## Current Building Blocks
- `aa_types.hpp`: fixed-size primitive aliases.
- `aa_containers.hpp`: EASTL-backed container wrappers (`AAVector`, `AAMap`, `AAList`, `AAString`).
- `aa_path.hpp/.cpp`: path composition utility (`AAPath`).
- `aa_platform.hpp/.cpp`: OS interaction for current working directory and recursive directory creation.
- `aa_file.hpp` + backend `.cpp`: cross-platform file abstraction (`AAFile`) with platform-specific implementations.
- `aa_memory_tracker.hpp/.cpp`: fixed-capacity heap allocation tracker (live set + stats).
- `aa_global_new.cpp`: global `new/delete` instrumentation for non-EASTL heap allocations.
- `aa_eastl_new.cpp`: EASTL allocation hooks + tracker integration.

## Platform Split
- `aa_file_macos.cpp`: POSIX backend (`open`, `pread`, `pwrite`, `fstat`, `fsync`).
- `aa_file_windows.cpp`: Win32 CRT backend (`_sopen_s`, `_read`, `_write`, `_fstat64`, `_commit`).

## Recommended Next Expansions
1. Input layer wrappers (`AAKey`, `AAInputState`, key mapping).
2. Clock/timer wrappers to remove direct time APIs from game loop.
3. Threading/job wrappers.
4. Socket/process wrappers.
5. Math module (`AAVec2`, `AAVec3`, `AAMat4`, SIMD specializations).
6. Asset-level streaming API (async file reads, mapped file support).

## Memory Tracking Notes
- Use `AA_NEW` or `AA_NEW_TAG(\"Tag\")` from `aa_memory.hpp` to capture file/line metadata in allocations.
- ImGui memory panel is currently rendered in both renderer backends and shows:
  - live bytes/count
  - peak bytes
  - total alloc/free counters
  - sampled live allocation rows (ptr, size, tag, file:line)
- Current tracker is intentionally minimal and single-process static. It is not lock-protected yet.
