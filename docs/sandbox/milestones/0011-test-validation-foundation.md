# Milestone 11 Record: Test and Validation Foundation

- **Milestone:** 11
- **Status:** Complete — locally validated by the developer
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

## Local validation status

The developer reported successful completion of the clean build, CTest run and manual regression checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- `ctest --test-dir build --output-on-failure` passed;
- `sfm_sandbox_scene_tests` passed;
- screenshot of `SfmSandbox` running after M11;
- existing point-cloud loading/reload remains functional;
- existing camera-pose loading remains functional;
- renderer status is ready;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- developer confirmed `EDAF80_Assignment1` still launches and works;
- developer confirmed `EDAN35_Assignment2` still launches.

Reference build commands:

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
| Clean configure succeeds | Passed — developer confirmed |
| Clean build succeeds | Passed — developer confirmed |
| `ctest --test-dir build --output-on-failure` succeeds | Passed — developer confirmed |
| `sfm_sandbox_scene_tests` reports all tests passed | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches and interaction still works | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| Existing point-cloud loading/reload still works | Passed — screenshot confirmed |
| Existing camera-pose loading still works | Passed — screenshot confirmed |
| Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| Milestone 2 ownership probe still passes | Passed — screenshot confirmed |

## Known limitations

- M11 tests cover CPU-side parsers only; no OpenGL rendering regression tests are added yet.
- Tests use a small dependency-free custom assertion harness rather than a third-party test framework.
- Coordinate-convention math tests are scheduled for a later milestone.
- Parser tests do not yet cover PLY, COLMAP, Bundler, OpenMVG or full matrix/quaternion formats because those loaders do not exist yet.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Test target infrastructure exists | Passed — source implementation |
| Point-cloud parser tests exist | Passed — source implementation |
| Camera-pose parser tests exist | Passed — source implementation |
| Tests run locally through CTest | Passed — developer confirmed |
| CI workflows run CTest after build | Passed — workflow implementation; pending PR CI confirmation |
| Parser behavior is documented | Passed — `docs/sandbox/parser-validation.md` |
| Legacy launch checks pass | Passed — developer confirmed |
| Project goal is documented as a real extensible tool, not a throwaway prototype | Passed — README, roadmap and standards updated |

Milestone 11 is complete and may be merged into `feature/sfm-visualization-sandbox` after CI passes.
