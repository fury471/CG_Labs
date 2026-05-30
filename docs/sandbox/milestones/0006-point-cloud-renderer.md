# Milestone 6 Record: PointCloud Data Model and First Point Renderer

- **Milestone:** 6
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m6-point-cloud-renderer`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Introduce the first CPU-side point-cloud data model and render it as coloured GPU points through the existing camera transform path. This milestone starts the transition from pure debug reference geometry toward SfM-style visualization data.

## Implemented source boundaries

```text
src/sandbox/scene/
  CMakeLists.txt
  PointCloud.hpp/.cpp           CPU-side point cloud model and deterministic debug sample cloud
src/sandbox/gfx/
  Renderer.hpp/.cpp             Adds point-cloud GPU upload and GL_POINTS draw path
src/apps/SfmSandbox/main.cpp    Creates debug point cloud and reports CPU/GPU point counts
```

`scene` is introduced because Milestone 6 now has renderer-independent CPU data. Point samples are not stored in `gfx` because they are semantic visualization data, not OpenGL resources.

## PointCloud model

`sfm::scene::PointCloud` currently stores:

- `glm::vec3 position`;
- `glm::vec3 colour`.

The model deliberately avoids speculative SfM metadata such as normals, confidence, track id, reprojection error, and source image observations. Those fields should be added when a real importer or reconstruction pipeline requires them.

For validation, `PointCloud::make_debug_cluster()` creates a deterministic coloured point cluster above the grid. It is not random, so screenshots and validation behavior remain reproducible.

## Renderer behavior

The renderer now owns two draw paths:

- grid/axes line path using `GL_LINES`;
- point-cloud path using `GL_POINTS`.

Both paths use the same camera transform convention:

```cpp
renderer.render(camera.GetWorldToClipMatrix());
```

The point path owns its own shader program, vertex buffer, vertex array, cached `u_world_to_clip` uniform, and point count. This keeps point-cloud rendering separate from the grid/axes reference primitive and leaves room for later point-specific styling.

## Runtime status panel

The `Sandbox status` panel now includes:

```text
Milestone 6: PointCloud data model and first point renderer
Renderer: ready/failed
Line vertices: ...
CPU point samples: ...
GPU point vertices: ...
```

It also keeps the previous probes visible:

```text
Milestone 3 regression: ShaderProgram cache
Milestone 2 regression: GPU RAII ownership
```

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| CPU point data is renderer-independent | Implemented in `sandbox/scene/PointCloud` |
| Renderer uploads point data to GPU | Implemented through a dedicated point vertex buffer |
| Points use the existing camera transform path | Point shader uses cached `u_world_to_clip` |
| Grid and point drawing are separate GPU paths | Separate programs, buffers and vertex arrays |
| Application owns data selection and camera/frame lifecycle | `SfmSandbox` creates the sample cloud and passes it to renderer setup |
| Milestone 3 and 2 probes remain visible | `SfmSandbox` still displays both regression probes |
| Legacy assignments remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` running with grid/axes and coloured point cloud visible;
- `Sandbox status` panel visible;
- `Renderer: ready` visible;
- line vertex count reported as positive;
- CPU point samples and GPU point vertices both reported as `288`;
- renderer messages report grid shader/cache setup, grid GPU layout, point shader/cache setup, point GPU layout and central renderer readiness;
- Milestone 3 shader probe remains passed in the same run;
- Milestone 2 ownership probe remains passed in the same run;
- developer confirmation that camera movement, resize/aspect behavior, legacy regression and sandbox control checks also passed.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m6-point-cloud-renderer
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
| `SfmSandbox` | Grid/axes remain visible | Passed — screenshot confirmed |
| `SfmSandbox` | Coloured point cloud is visible above the grid | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Milestone 6: PointCloud data model and first point renderer` | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Renderer: ready` | Passed — screenshot confirmed |
| `SfmSandbox` | CPU point sample count and GPU point vertex count are positive and match | Passed — screenshot confirmed (`288`) |
| `SfmSandbox` | Camera movement changes both grid and point-cloud view consistently | Passed — developer confirmed |
| `SfmSandbox` | Resize/aspect behavior still works | Passed — developer confirmed |
| `SfmSandbox` | Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc` still work | Passed — developer confirmed |

## Known limitations at completion

- Point-cloud data is generated procedurally; no file importer exists yet.
- Points contain only position and colour.
- Point size is fixed in renderer setup.
- No spatial indexing, streaming, LOD, picking, point splatting, camera pose rendering or trajectory rendering exists yet.
- The point shader is still source-string based; file-based shader loading remains deferred.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Clean Ninja configure/build succeeds in the VS 2026 x64 development environment | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| CPU-side `PointCloud` model exists outside `gfx` | Passed — source implementation |
| `SfmSandbox` renders coloured point cloud through camera transform | Passed — screenshot confirmed |
| CPU/GPU point counts match | Passed — screenshot confirmed (`288`) |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 6 is complete and may be merged into `feature/sfm-visualization-sandbox`.
