# Milestone 9 Record: Camera Pose / Frustum Visualization

- **Milestone:** 9
- **Status:** Implementation ready for local validation
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

## Local validation required

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
| Existing `EDAF80_Assignment1` launches and interaction still works | Pending |
| Existing `EDAN35_Assignment2` launches | Pending |
| `SfmSandbox` launches with status panel | Pending |
| Grid/axes and point cloud remain visible | Pending |
| Camera frustums are visible around/near the point cloud | Pending |
| Trajectory line between camera poses is visible | Pending |
| Panel shows `Milestone 9: Camera pose / frustum visualization` | Pending |
| Panel shows positive camera-pose count | Pending |
| Panel shows positive camera-line vertex count | Pending |
| Runtime point-cloud reload still succeeds on valid sample path | Pending |
| Invalid reload still preserves the previous visible cloud | Pending |
| Camera movement and resize/aspect behavior still work | Pending |
| Milestone 3 shader probe still passes | Pending |
| Milestone 2 ownership probe still passes | Pending |
| `F2`, `F3`, `F11`, `Esc` still work | Pending |

## Known limitations

- Camera poses are generated procedurally; no camera-pose file importer exists yet.
- Frustum scale is fixed in renderer code.
- Camera-pose visualization is static after startup.
- There is no selection, picking, pose labels, image thumbnails or real SfM track visualization yet.

## Completion gate

Milestone 9 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
