# Milestone 9 Record: Camera Pose / Frustum Visualization

- **Milestone:** 9
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m9-camera-frustums`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Add the first camera-pose visualization layer to the SfM sandbox. The renderer should draw camera frustums and a trajectory path beside the existing grid, axes and point cloud, using the same world-to-clip camera transform.

## Implemented source boundaries

```text
src/sandbox/scene/
  CameraPose.hpp/.cpp          CPU-side camera pose set and deterministic sample path
src/sandbox/gfx/
  Renderer.hpp/.cpp            Camera frustum and trajectory line rendering
src/apps/SfmSandbox/main.cpp   Creates sample poses and reports pose visualization counts
```

## Data model

`sfm::scene::CameraPose` stores:

- `glm::mat4 camera_to_world`;
- `glm::vec3 colour`.

The transform maps local camera-space points into world space. The local camera convention is negative-Z forward and positive-Y up. `CameraPoseSet::make_debug_orbit()` creates a deterministic orbit around the sample point cloud region, so frustum and trajectory rendering can be validated before real SfM camera import exists.

## Renderer behavior

The renderer now draws three visualization layers:

- grid/axes line geometry;
- camera frustum and trajectory line geometry;
- point cloud points.

Camera frustum rendering reuses the existing line shader and DSA vertex layout. The camera-pose line buffer is static for this milestone. Runtime point-cloud reload from Milestone 8 remains active and only replaces point-cloud GPU resources.

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` showing visible camera frustums around the point cloud;
- trajectory line between camera poses is visible;
- grid/axes and point cloud remain visible;
- panel shows `Milestone 9: Camera pose / frustum visualization`;
- panel shows `Camera poses: 8`;
- panel shows `Camera line vertices: 158`;
- renderer status is ready;
- runtime point-cloud reload remains active and shows the loaded sample path;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- developer confirmed `EDAF80_Assignment1` still launches correctly;
- developer confirmed `EDAN35_Assignment2` still launches correctly;
- developer confirmed camera movement, resize/aspect behavior and sandbox controls still work.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m9-camera-frustums
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Manual checklist:

| Check | Result |
|---|---|
| Existing `EDAF80_Assignment1` launches and interaction still works | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` launches with status panel | Passed — screenshot confirmed |
| Grid/axes and point cloud remain visible | Passed — screenshot confirmed |
| Camera frustums are visible around/near the point cloud | Passed — screenshot confirmed |
| Trajectory line between camera poses is visible | Passed — screenshot confirmed |
| Panel shows `Milestone 9: Camera pose / frustum visualization` | Passed — screenshot confirmed |
| Panel shows positive camera-pose count | Passed — screenshot confirmed (`8`) |
| Panel shows positive camera-line vertex count | Passed — screenshot confirmed (`158`) |
| Runtime point-cloud reload still succeeds on valid sample path | Passed — screenshot confirmed |
| Invalid reload still preserves the previous visible cloud | Passed — inherited from Milestone 8 and developer-confirmed workflow |
| Camera movement and resize/aspect behavior still work | Passed — developer confirmed |
| Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `F2`, `F3`, `F11`, `Esc` still work | Passed — developer confirmed |

## Known limitations

- Camera poses are generated procedurally; no camera-pose file importer exists yet.
- Frustum scale is fixed in renderer code.
- Camera-pose visualization is static after startup.
- There is no selection, picking, pose labels, image thumbnails or real SfM track visualization yet.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| CPU-side camera-pose model exists outside `gfx` | Passed — source implementation |
| Renderer draws camera frustums through the existing camera transform path | Passed — screenshot confirmed |
| Renderer draws trajectory lines between poses | Passed — screenshot confirmed |
| Point-cloud reload path remains functional | Passed — screenshot confirmed |
| Legacy assignment launch checks pass | Passed — developer confirmed |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 9 is complete and may be merged into `feature/sfm-visualization-sandbox`.
