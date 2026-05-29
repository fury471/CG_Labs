# Milestone 2 Record: Move-only RAII GPU Resource Layer

- **Milestone:** 2
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m2-gpu-raii`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Introduce safe ownership for new GPU resources in the SfM Visualization Sandbox. The resource layer must be move-only, deterministic in cleanup, safe when empty, and visible in development tooling through OpenGL object labels.

## Implemented source boundaries

```text
src/sandbox/gfx/
  Buffer.hpp/.cpp             Owns one OpenGL buffer object
  VertexArray.hpp/.cpp        Owns one OpenGL vertex array object
  Texture2D.hpp/.cpp          Owns one OpenGL 2D texture object
  Sampler.hpp/.cpp            Owns one OpenGL sampler object
  ShaderProgram.hpp/.cpp      Owns one OpenGL program object
  Framebuffer.hpp/.cpp        Owns one OpenGL framebuffer object
  DebugLabel.hpp/.cpp         Internal helper for OpenGL object labels
  OwnershipProbe.hpp/.cpp     Runtime validation probe for ownership invariants
```

Milestone 1 files also received explanatory comments where they clarify architectural responsibility and learning value:

```text
src/apps/SfmSandbox/main.cpp
src/sandbox/core/FrameClock.hpp/.cpp
src/sandbox/gfx/ClearPass.hpp/.cpp
```

## Ownership rules implemented

Each GPU owner follows the same contract:

- default construction creates a safe empty owner with `id() == 0`;
- labelled construction creates one OpenGL object;
- copy construction and copy assignment are deleted;
- move construction and move assignment are `noexcept`;
- moving transfers the OpenGL name and leaves the source empty;
- `reset()` releases the object if present and is safe to call repeatedly;
- the destructor calls `reset()`;
- `operator bool()` reports whether the owner currently contains an OpenGL name.

## API scope deliberately deferred

The resource classes currently own lifetime only. They do **not** yet define:

- buffer storage/upload policy;
- vertex attribute layout policy;
- texture storage format or upload policy;
- sampler filtering/wrapping policy;
- shader compilation/linkage diagnostics;
- framebuffer attachments or completeness checks.

Those decisions are deferred to later milestones so their APIs are driven by real renderer and visualization requirements.

## Runtime validation probe

`SfmSandbox` now calls `sfm::gfx::run_ownership_probe()` after the OpenGL context is created and before the window/context is destroyed.

The probe checks:

- compile-time non-copyable / move-enabled contracts;
- successful non-empty creation of each resource type;
- move-construction ownership transfer;
- move-assignment ownership transfer;
- safe empty state after `reset()`.

The result appears in the `Sandbox status` panel under:

```text
Milestone 2: GPU RAII ownership
Ownership probe: passed/failed
```

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| GPU ownership is represented by RAII owner types | Implemented for six initial OpenGL resource classes |
| Owners are non-copyable | Enforced by deleted copy constructor and copy assignment |
| Owners are move-enabled | Implemented with `noexcept` move constructor and move assignment |
| Moved-from objects remain valid and empty | Implemented with `std::exchange(..., 0u)` and checked by the probe |
| Application code does not manually delete owned GPU resources | Probe and app use RAII owners only |
| Debug labels exist in development runs | Implemented through `DebugLabel` helper and labelled constructors |
| Existing assignment applications remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` running with OpenGL 4.6, debug context enabled;
- `Sandbox status` panel visible;
- `Ownership probe: passed` visible;
- every resource type reports both compile-time move-only contract and create/move/reset invariants as passed;
- developer confirmation that remaining legacy regression and sandbox control checks also passed.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m2-gpu-raii
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Manual launch checklist:

| Executable | Required observation | Result |
|---|---|---|
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Passed — developer confirmed |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Passed — developer confirmed |
| `SfmSandbox` | Window opens and the status panel is visible | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Milestone 2: GPU RAII ownership` | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Ownership probe: passed` | Passed — screenshot confirmed |
| `SfmSandbox` | Probe messages list every resource type as passed | Passed — screenshot confirmed |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc`, and resize behaviour still work | Passed — developer confirmed |

## Known limitations at completion

- The RAII classes are lifetime owners only; they do not yet provide high-level data upload, shader linking, texture storage, sampler configuration, or framebuffer attachment APIs.
- The ownership probe is a manual runtime check, not a replacement for automated CPU/GPU tests.
- No performance claim is made by this milestone.
- `SfmSandbox` still does not render scene geometry, point clouds, camera poses, trajectories or reconstructed meshes.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Clean Ninja configure/build succeeds in the VS 2026 x64 development environment | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` ownership probe passes | Passed — screenshot confirmed |
| New GPU resources use move-only RAII ownership | Passed — source implementation and probe |
| Completion note records commands and limitations | Passed — this document |

Milestone 2 is complete and may be merged into `feature/sfm-visualization-sandbox`.
