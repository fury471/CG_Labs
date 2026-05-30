# Milestone 10 Record: Camera Pose Loading from File

- **Milestone:** 10
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m10-camera-loader`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Load camera poses from a small text resource file and render the resulting frustums/trajectory through the existing camera-pose visualization path. The deterministic debug orbit remains a fallback only if loading fails.

## Implemented source boundaries

```text
src/sandbox/scene/
  CameraPoseLoader.hpp/.cpp      Camera-pose text loader and diagnostics
src/apps/SfmSandbox/main.cpp     Loads sample camera poses and reports diagnostics
res/sandbox/
  sample_camera_poses.txt        Checked-in validation pose file
```

## Supported camera-pose format

The loader accepts rows with exactly 9 numeric fields:

```text
eye_x eye_y eye_z target_x target_y target_z r g b
```

Commas are accepted as separators, text after `#` is treated as a comment, and colour values may be normalized `[0, 1]` floats or byte-style `[0, 255]` values.

The loader validates finite eye/target positions and rejects degenerate directions where the camera cannot form a stable `lookAt` matrix.

## Runtime behavior

`SfmSandbox` now loads:

```text
res/sandbox/sample_camera_poses.txt
```

through `config::resources_path(...)`. If the pose file loads successfully, the loaded file-backed poses are rendered. If loading fails, the procedural camera orbit from Milestone 9 is used as fallback.

The status panel now reports camera-pose source, pose count, skipped pose lines, loader diagnostics and camera-line vertex count.

## Local validation required

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m10-camera-loader
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
| Camera frustums and trajectory remain visible | Pending |
| Panel shows `Milestone 10: Camera pose loading from file` | Pending |
| Panel shows camera pose source as sample file path | Pending |
| Panel shows positive camera-pose count | Pending |
| Panel shows skipped pose lines as `0` for the clean sample file | Pending |
| Panel loader diagnostics report loaded file and valid pose count | Pending |
| Runtime point-cloud reload still succeeds on valid sample path | Pending |
| Invalid point-cloud reload still preserves the previous visible cloud | Pending |
| Camera movement and resize/aspect behavior still work | Pending |
| Milestone 3 shader probe still passes | Pending |
| Milestone 2 ownership probe still passes | Pending |
| `F2`, `F3`, `F11`, `Esc` still work | Pending |

## Known limitations

- Camera-pose loading is startup-only; there is no runtime camera-pose reload UI yet.
- The loader supports only the milestone eye-target-colour text format.
- No quaternion, full matrix, COLMAP, Bundler, OpenMVG or PLY camera import exists yet.
- Frustum scale is still fixed in renderer code.

## Completion gate

Milestone 10 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
