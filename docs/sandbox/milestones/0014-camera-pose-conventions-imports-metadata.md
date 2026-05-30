# Milestone 14 Record: Camera-Pose Conventions, Import Formats and Metadata

- **Milestone:** 14
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m14-camera-pose-conventions`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Make camera poses usable with real SfM outputs and remove ambiguity around coordinate systems before later milestones connect cameras, points, images and projections.

M14 turns the earlier camera-pose visualization and milestone text loader into a clearer pose-data foundation. It keeps the renderer-facing contract simple while preserving enough metadata for inspection and debugging.

## Implemented source boundaries

```text
docs/sandbox/camera-pose-conventions.md
src/sandbox/scene/CameraPose.hpp/.cpp
src/sandbox/scene/CameraPoseLoader.hpp/.cpp
src/sandbox/gfx/Renderer.hpp/.cpp
src/apps/SfmSandbox/main.cpp
tests/sandbox/SceneParserTests.cpp
```

## Internal camera-pose contract

All camera poses are stored internally as:

```text
CameraPose::camera_to_world
```

The local camera-space convention is:

```text
+X = camera right
+Y = camera up
-Z = camera forward
```

For a pose matrix `M`:

```text
position =  M[3].xyz
right    =  M[0].xyz
up       =  M[1].xyz
forward  = -M[2].xyz
```

This contract is documented in `docs/sandbox/camera-pose-conventions.md` and is now visible in the runtime pose metadata panel.

## Implemented behavior

- Preserved the existing milestone eye/target text pose format.
- Added `CameraPoseMetadata` beside each pose.
- Added source metadata for procedural debug poses and file-backed poses.
- Added a camera-pose format dispatcher.
- Added COLMAP text `images.txt` pose import.
- Converts COLMAP/OpenCV world-to-camera poses into the sandbox graphics camera-to-world convention.
- Added runtime camera-pose reload controls.
- Camera-pose reload is transactional: failed file load or failed GPU upload keeps the previous visible pose set.
- Added selected-pose metadata display in the ImGui panel.
- Added pose index, source id, camera id, source line, image/name, format, source file, convention, position, forward, up and right display.
- Updated the app panel title to Milestone 14.

## COLMAP import scope

Supported from `images.txt`:

```text
IMAGE_ID QW QX QY QZ TX TY TZ CAMERA_ID NAME
```

The following 2D observation line is skipped in M14 because this milestone imports pose metadata only. Point observations and image projection debugging are scheduled for later milestones.

COLMAP/OpenCV camera axes are interpreted as:

```text
+X = camera right
+Y = camera down
+Z = camera forward
```

The conversion into the sandbox graphics convention flips Y and Z before storing `camera_to_world`.

## Test coverage

M14 extends `sfm_sandbox_scene_tests` with:

- preservation of the existing milestone text pose loader;
- camera-pose dispatcher rejection for unsupported extensions;
- text-pose metadata checks;
- COLMAP `images.txt` identity-pose conversion check;
- COLMAP camera-center computation from `-R^T t`;
- existing point-cloud parser, PLY and dispatcher tests preserved.

## Roadmap alignment

| M14 roadmap item | Status |
|---|---|
| Coordinate-convention documentation | Implemented in `docs/sandbox/camera-pose-conventions.md` |
| Tests for pose transform conventions | Implemented in `SceneParserTests.cpp` |
| Runtime camera-pose reload with failed-reload preservation | Implemented |
| At least one common camera-pose import path | Implemented: COLMAP text `images.txt` |
| Pose metadata display | Implemented |
| Optional pose selection | Implemented as a simple selected-pose slider, without renderer architecture changes |

## Local validation status

The developer reported successful completion of local build/test/manual validation on 2026-05-30 using the intended VS 2026/Ninja workflow.

Evidence received:

- `ctest --test-dir build --output-on-failure` passed;
- `sfm_sandbox_scene_tests` passed 1/1 tests;
- screenshot of `SfmSandbox` running with the M14 camera-pose metadata panel;
- panel shows default camera-pose sample loaded from `res/sandbox/sample_camera_poses.txt`;
- panel shows `Pose format: milestone eye-target text`;
- panel shows `Camera poses: 8`;
- panel shows selected-pose metadata including position, forward, up and right vectors;
- camera frustums and trajectory render in the viewport;
- point-cloud bounds and point-cloud sample remain visible;
- renderer status is ready;
- developer confirmed `EDAF80_Assignment1` launches;
- developer confirmed `EDAN35_Assignment2` launches.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m14-camera-pose-conventions
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure succeeds | Passed — developer validation implied by successful build/test |
| Clean build succeeds | Passed — developer validation implied by successful app/test run |
| CTest succeeds | Passed — developer provided output |
| `sfm_sandbox_scene_tests` passes | Passed — 1/1 tests passed |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| M14 panel title and metadata are visible | Passed — screenshot confirmed |
| Default camera-pose sample loads | Passed — screenshot confirmed |
| Camera frustums render | Passed — screenshot confirmed |
| Point-cloud reload path remains present | Passed — screenshot confirmed |
| Renderer status is ready | Passed — screenshot confirmed |
| `EDAF80_Assignment1` launches | Passed — developer confirmed |
| `EDAN35_Assignment2` launches | Passed — developer confirmed |

## Known limitations

- COLMAP binary models are not supported.
- COLMAP `cameras.txt` intrinsics are not imported yet.
- COLMAP `points3D.txt` track and observation metadata are not imported yet.
- `images.txt` 2D observation rows are skipped in M14.
- OpenMVG JSON and Nerfstudio `transforms.json` are not supported yet.
- Image loading, image planes and projection/reprojection debugging remain scheduled for later milestones.
- The selected-pose UI is a simple index slider, not a full picking system.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Known synthetic camera sequence displays with verified orientation and order | Passed — default sample and metadata panel validated; convention tests added |
| Camera-pose reload does not destroy previous visible poses on failure | Implemented through transactional file-load/GPU-upload path |
| Transform convention tests pass | Passed — CTest reported success |
| Point-cloud reload remains functional | Passed — point-cloud controls and dataset remain visible in the M14 panel |
| Local CTest passes | Passed — developer provided output |
| Legacy launch checks pass | Passed — developer confirmed |
| Known limitations are documented | Passed — this document and convention document |

Milestone 14 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
