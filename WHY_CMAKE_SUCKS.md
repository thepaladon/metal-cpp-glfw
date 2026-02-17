# Why CMake Sucks (Reminder)

1. Generator noise: VS mode floods output with `ALL_BUILD`, `ZERO_CHECK`, `.dir`, `x64`, and many helper files.
2. IDE coupling: behavior/layout changes by generator (`Visual Studio` vs `Ninja` vs `Xcode`), so "one build setup" is not truly one.
3. Cache fragility: stale cache causes confusing behavior until build dirs are deleted or reconfigured carefully.
4. Tooling assumptions: compilers/toolchains/env setup often fail silently or indirectly (especially on Windows without dev env initialized).
5. Dependency complexity: package-first vs fetch-at-configure adds branching logic and edge cases.
6. Cross-platform inconsistency: same CMake code still needs platform-specific conditionals and dependency quirks.
7. Poor first-run UX: onboarding usually needs extra tools (`cmake`, generator, compiler, package manager), plus PATH issues.
8. Error quality: many failures surface late and with indirect messages, slowing diagnosis.
9. Build-dir hygiene is hard: clean artifact layout conflicts with generator internals unless you enforce strict presets/scripts.
10. Maintenance overhead: you end up maintaining build logic, bootstrap scripts, presets, and docs just to keep usage simple.
