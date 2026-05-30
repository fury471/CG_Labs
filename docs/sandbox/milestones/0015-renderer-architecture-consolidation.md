# Milestone 15 Record: Renderer Architecture Consolidation

- **Milestone:** 15
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m15-renderer-architecture`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Pay down renderer architecture debt before adding reconstructed mesh loading and image visualization.

M15 consolidates the current grid, point-cloud, point-cloud bounds and camera-frustum rendering path around clearer renderer-owned submission concepts. It also moves the active sandbox shader path toward file-backed shader sources so later rendering features can use normal shader files instead of embedded source strings.

## Implemented source boundaries

```text
src/sandbox/gfx/Material.hpp
src/sandbox/gfx/Mesh.hpp
src/sandbox/gfx/RenderSubmission.hpp
src/sandbox/gfx/ShaderProgram.hpp/.cpp
src/sandbox/gfx/Renderer.hpp/.cpp
src/apps/SfmSandbox/main.cpp
shaders/sandbox/line.vert
shaders/sandbox/line.frag
shaders/sandbox/point_cloud.vert
```

## Implemented behavior

- Added `ShaderFileSource` and `ShaderProgram::build_from_files(...)`.
- Shader build logs now report file paths for file-backed shader stages.
- Moved the active line shader source to `shaders/sandbox/line.vert` and `shaders/sandbox/line.frag`.
- Moved the active point-cloud vertex shader source to `shaders/sandbox/point_cloud.vert`.
- Added a minimal `MaterialDescriptor` with current line and point-cloud material categories.
- Added a minimal non-owning `MeshHandle` for renderer-owned VAO/count/primitive submission records.
- Added `RenderSubmission` and `RenderQueue` as the explicit renderer submission boundary.
- Routed grid, bounds, camera frustums and point cloud drawing through the M15 submission path.
- Kept raw OpenGL draw calls inside `sandbox/gfx/Renderer.cpp`.
- Added `RendererFrameStatistics` for the current frame.
- Exposed renderer statistics in the `SfmSandbox` panel:
  - submitted items;
  - draw calls;
  - program binds;
  - vertex-array binds;
  - line vertices drawn;
  - point vertices drawn.
- Updated the sandbox panel title to Milestone 15.

## Roadmap alignment

| M15 roadmap item | Status |
|---|---|
| Shader source loading from files | Implemented for active sandbox line shader and point-cloud vertex shader |
| Improved shader diagnostics with file names | Implemented through `ShaderProgram::build_from_files(...)` |
| Minimal material abstraction | Implemented as `MaterialDescriptor` / `MaterialKind` |
| Mesh abstraction | Implemented as non-owning `MeshHandle` |
| Renderable/submission model | Implemented as `RenderSubmission` |
| Render queue or equivalent explicit submission structure | Implemented as `RenderQueue` |
| Draw-call and state-change statistics panel | Implemented as `RendererFrameStatistics` panel output |
| Keep raw OpenGL calls inside `sandbox/gfx` | Preserved for the active sandbox render path |

## Local validation status

The developer reported successful completion of local build/test/manual validation on 2026-05-30 using the intended VS 2026/Ninja workflow.

Evidence received:

- `SfmSandbox` launched successfully;
- screenshot shows the M15 renderer statistics panel;
- grid, point cloud, point-cloud bounds and camera frustums are visible;
- renderer status is ready;
- submitted items are visible and nonzero;
- draw calls are visible and nonzero;
- program bind and vertex-array bind statistics are visible;
- line and point vertex statistics are visible;
- point-cloud reload remained functional;
- camera-pose reload remained functional;
- developer confirmed all required local checks passed, including the existing test and legacy launch checks.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m15-renderer-architecture
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure succeeds | Passed — developer confirmed all local checks passed |
| Clean build succeeds | Passed — developer confirmed all local checks passed |
| CTest succeeds | Passed — developer confirmed all local checks passed |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| M15 panel title is visible | Passed — screenshot confirmed |
| Grid renders | Passed — screenshot confirmed |
| Point cloud renders | Passed — screenshot confirmed |
| Point-cloud bounds render | Passed — screenshot confirmed |
| Camera frustums render | Passed — screenshot confirmed |
| Renderer status is ready | Passed — screenshot confirmed |
| Renderer statistics are visible and nonzero | Passed — screenshot confirmed |
| Point-cloud reload remains functional | Passed — screenshot confirmed |
| Camera-pose reload remains functional | Passed — screenshot confirmed |
| `EDAF80_Assignment1` launches | Passed — developer confirmed all checks passed |
| `EDAN35_Assignment2` launches | Passed — developer confirmed all checks passed |

## Known limitations

- This is a transitional consolidation, not a full render-graph or asset-system rewrite.
- The current `MeshHandle` is non-owning and points to renderer-owned VAOs and counts.
- The material abstraction only distinguishes current line-colour and point-cloud materials.
- The point-cloud fragment stage currently reuses `shaders/sandbox/line.frag` because the current line and point fragment stages are functionally identical.
- `shaders/sandbox/point_cloud.frag` was not added in this milestone because the GitHub connector rejected that small shader file upload; this does not affect runtime behavior.
- Draw statistics count submitted/drawn items in the current explicit submission path, not full GPU pipeline timings.
- State-change statistics are conservative CPU-side counts, not driver-level state-change introspection.
- More complete mesh assets, materials, render sorting and image-plane rendering remain for later milestones.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Grid, point cloud and camera frustums render through the consolidated submission path or documented transitional path | Passed — routed through `RenderQueue`/`RenderSubmission` transitional path |
| Renderables do not issue raw OpenGL draw calls themselves | Passed — active sandbox raw draw calls remain inside `sandbox/gfx/Renderer.cpp` |
| Draw-call/state statistics are visible | Passed — M15 UI statistics confirmed by screenshot |
| No behavior regression in existing sandbox features | Passed — developer confirmed all local checks and screenshot shows expected visuals |
| Local CTest passes | Passed — developer confirmed all checks passed |
| Legacy launch checks pass | Passed — developer confirmed all checks passed |
| Known limitations are documented | Passed — this record lists the transitional limitations |

Milestone 15 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
