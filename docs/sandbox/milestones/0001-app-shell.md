# Milestone 1 Record: Application Shell and Target Boundaries

- **Milestone:** 1
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-29
- **Development branch:** `feature/sfm-m1-app-shell`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Create a new runnable application target named `SfmSandbox`, introduce the first new-code target boundaries, and preserve the existing EDAF80/EDAN35 applications unchanged.

## Implemented source boundaries

```text
src/sandbox/
  CMakeLists.txt                 Target-local C++20 and warning policy
  core/
    CMakeLists.txt
    FrameClock.hpp/.cpp          CPU-side frame timing data
  gfx/
    CMakeLists.txt
    ClearPass.hpp/.cpp           Minimal viewport and frame-clear graphics service
src/apps/SfmSandbox/
  CMakeLists.txt
  main.cpp                       Application composition and ImGui status UI
```

`scene` and `viz` have deliberately not been added as empty targets: they will appear only when a compiled feature requires their responsibility boundary.

## Implemented behaviour

`SfmSandbox` currently:

- creates a resizable OpenGL/ImGui window using the retained Bonobo platform/window facility;
- renders a controlled dark clear-colour frame through `sfm::gfx::ClearPass`;
- keeps `glViewport` and frame-clearing OpenGL calls within `sandbox/gfx`;
- displays an ImGui status panel with frame index, CPU frame duration, FPS and framebuffer size;
- supports `F2` to hide/show the UI, `F3` to hide/show logs, `F11` to toggle fullscreen and `Esc` to close.

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| New product code remains outside legacy assignment files | Passed by source placement; no legacy assignment file changed |
| New sandbox code uses target-local C++20 | Implemented in `src/sandbox/CMakeLists.txt` |
| Initial renderer boundary is explicit | `ClearPass` contains viewport and clear rendering calls |
| App composes reusable services and UI | Implemented in `src/apps/SfmSandbox/main.cpp` |
| Do not add unused layers prematurely | `scene` and `viz` deferred |

## Files intentionally not modified

No source implementation changes are made under:

```text
src/EDAF80/
src/EDAN35/
src/core/
```

The only root-level change is adding the new CMake subdirectories for sandbox libraries and the new application.

## Validation still required on Windows / VS 2026 / Ninja

This record does **not** declare Milestone 1 complete until the intended local environment verifies build and launch behaviour.

From the Visual Studio 2026 x64 Developer Command Prompt, after switching to this branch:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m1-app-shell
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Locate binaries if necessary:

```bat
dir /s /b build\*.exe
```

Manual launch checklist:

| Executable | Required observation | Result |
|---|---|---|
| `EDAF80_Assignment1` | Existing scene launches and mouse/UI interaction still works | Pending |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Pending |
| `SfmSandbox` | Window opens with dark frame and `Sandbox status` panel | Pending |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc` behave correctly | Pending |
| `SfmSandbox` | Window resize updates framebuffer-size readout and clears correctly | Pending |

## Known limitations at this checkpoint

- `SfmSandbox` does not yet render scene geometry, reconstruction data, camera poses or point clouds.
- The window/platform bootstrap still reuses Bonobo intentionally; a new renderer/resource architecture begins beneath this boundary in later milestones.
- No automated build or application-level test is added in this milestone.
- No performance claim is made from the displayed CPU-frame value; it is an initial development statistic only.

## Completion gate

After local validation, update this document with observed results and merge `feature/sfm-m1-app-shell` into `feature/sfm-visualization-sandbox` only when all Milestone 1 acceptance criteria are satisfied.
