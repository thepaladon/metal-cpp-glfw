# Core Layer Plan

## Goal
Provide one reusable `src/core` module that isolates platform concerns and type/container policy from gameplay/rendering code.

## Current Building Blocks
- `aa_types.hpp`: fixed-size primitive aliases.
- `aa_containers.hpp`: EASTL-backed container wrappers (`AAVector`, `AAMap`, `AAList`, `AAString`).
- `aa_path.hpp/.cpp`: path composition utility (`AAPath`).
- `aa_platform.hpp/.cpp`: OS interaction for current working directory and recursive directory creation.
- `aa_file.hpp` + backend `.cpp`: cross-platform file abstraction (`AAFile`) with platform-specific implementations.

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
