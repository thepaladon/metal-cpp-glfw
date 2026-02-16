# Codex Worktree Policy

## Goal
Every new task must run in its own git worktree and branch so changes stay isolated.

## Required Workflow For Codex
1. Derive a short task slug from the user request (for example, `fix-crash-on-resize`).
2. Run:
   - `./scripts/worktree-task.sh create <task-slug>`
   - `./scripts/worktree-task.sh path <task-slug>`
3. Do all task edits, builds, and tests inside that worktree path, not in the main checkout.
4. Keep branch name `codex/<task-slug>` (handled by the script).
5. At task completion, report:
   - worktree path
   - branch name
   - push command
6. Only run merge/cleanup when the user explicitly asks to finish:
   - `./scripts/worktree-task.sh finish <task-slug>`

## Safety Rules
- Never mix unrelated tasks in one worktree or branch.
- Never delete a worktree branch unless finishing that exact task.
- If a worktree for the task already exists, reuse it.
- Do not use `main` for direct feature work.

## Core Coding Rules
- Prefer the `src/core` abstraction layer for OS interaction (paths, files, directories, platform calls).
- Ban direct use of primitive unsized types in engine code: use sized aliases (`i8/i16/i32/i64`, `u8/u16/u32/u64`, `f32/f64`, `usize`) from `src/core/aa_types.hpp`.
- Ban direct STL containers in engine-facing code. Use core wrappers (`AAVector`, `AAMap`, `AAList`, `AAString`) from `src/core/aa_containers.hpp`.
- Avoid direct `std::` usage unless no core wrapper exists yet. If missing, add/extend a core wrapper first.
