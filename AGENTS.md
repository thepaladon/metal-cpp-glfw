## Core Coding Rules
- Prefer the `src/core` abstraction layer for OS interaction (paths, files, directories, platform calls).
- Ban direct use of primitive unsized types in engine code: use sized aliases (`i8/i16/i32/i64`, `u8/u16/u32/u64`, `f32/f64`, `usize`) from `src/core/aa_types.hpp`.
- Ban direct STL containers in engine-facing code. Use core wrappers (`AAVector`, `AAMap`, `AAList`, `AAString`) from `src/core/aa_containers.hpp`.
- Avoid direct `std::` usage unless no core wrapper exists yet. If missing, add/extend a core wrapper first.
