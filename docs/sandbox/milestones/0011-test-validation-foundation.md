# Milestone 11 Record: Test and Validation Foundation

- **Milestone:** 11
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m11-test-validation`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Establish the first automated CPU-side validation layer for deterministic sandbox components, starting with the point-cloud and camera-pose parsers. This milestone supports the project goal of building a powerful, best-practice and extensible SfM visualization tool, not a throwaway prototype.

## Implemented source boundaries

```text
CMakeLists.txt                         Enables CTest and adds tests when BUILD_TESTING is on
.github/workflows/*.yml                Runs CTest after CI builds
tests/CMakeLists.txt                   Test tree entry point
tests/sandbox/CMakeLists.txt           Sandbox test target
tests/sandbox/SceneParserTests.cpp     Parser tests
docs/sandbox/parser-validation.md      Parser guarantees and exclusions
docs/sandbox/README.md                 Tool-goal clarification
docs/sandbox/ROADMAP.md                Tool-goal clarification
docs/sandbox/ENGINEERING_STANDARDS.md Tool-goal clarification
```

## Test coverage

The new `sfm_sandbox_scene_tests` executable covers:

- point-cloud parser support for comments, whitespace, commas, 3-field rows and 6-field rows;
- point-cloud parser RGB behavior for default colour, normalized RGB and byte-style RGB;
- point-cloud parser malformed-row counting;
- point-cloud parser failure for missing files and files with no valid points;
- camera-pose parser support for comments, whitespace, commas and valid `eye target colour` rows;
- camera-pose parser malformed-row counting;
- camera-pose parser rejection of degenerate view directions;
- camera-pose parser failure for files with no valid poses.

The tests create temporary files at runtime and do not depend on external datasets or absolute paths.

## CI behavior

The existing Windows, Ubuntu and macOS workflows now run:

```text
ctest --test-dir <build-dir> --build-config <config> --output-on-failure
```

after the build step.

## Local validation required

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m11-test-validation
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure succeeds | Pending |
| Clean build succeeds | Pending |
| `ctest --test-dir build --output-on-failure` succeeds | Pending |
| `sfm_sandbox_scene_tests` reports all tests passed | Pending |
| Existing `EDAF80_Assignment1` launches and interaction still works | Pending |
| Existing `EDAN35_Assignment2` launches | Pending |
| `SfmSandbox` launches | Pending |
| Existing point-cloud loading/reload still works | Pending |
| Existing camera-pose loading still works | Pending |
| Milestone 3 shader probe still passes | Pending |
| Milestone 2 ownership probe still passes | Pending |

## Known limitations

- M11 tests cover CPU-side parsers only; no OpenGL rendering regression tests are added yet.
- Tests use a small dependency-free custom assertion harness rather than a third-party test framework.
- Coordinate-convention math tests are scheduled for a later milestone.
- Parser tests do not yet cover PLY, COLMAP, Bundler, OpenMVG or full matrix/quaternion formats because those loaders do not exist yet.

## Completion gate

Milestone 11 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
