# Milestone 10 Record: Camera Pose Loading from File

- **Milestone:** 10
- **Status:** Complete — locally validated by the developer
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

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` showing camera frustums and trajectory rendered from the loaded pose file;
- panel shows `Milestone 10: Camera pose loading from file`;
- panel shows camera pose source as `res/sandbox/sample_camera_poses.txt`;
- panel shows `Camera poses: 8`;
- panel shows `Skipped pose lines: 0`;
- loader diagnostics report the loaded camera-pose file and valid pose count;
- panel shows `Camera line vertices: 158`;
- grid/axes and point cloud remain visible;
- runtime point-cloud reload remains functional and shows valid sample point-cloud diagnostics;
- renderer status is ready;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- developer confirmed `EDAF80_Assignment1` still launches correctly;
- developer confirmed `EDAN35_Assignment2` still launches correctly;
- developer confirmed camera movement, resize/aspect behavior and sandbox controls still work.

Reference build commands:

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
| Existing `EDAF80_Assignment1` launches and interaction still works | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` launches with status panel | Passed — screenshot confirmed |
| Grid/axes and point cloud remain visible | Passed — screenshot confirmed |
| Camera frustums and trajectory remain visible | Passed — screenshot confirmed |
| Panel shows `Milestone 10: Camera pose loading from file` | Passed — screenshot confirmed |
| Panel shows camera pose source as sample file path | Passed — screenshot confirmed |
| Panel shows positive camera-pose count | Passed — screenshot confirmed (`8`) |
| Panel shows skipped pose lines as `0` for the clean sample file | Passed — screenshot confirmed |
| Panel loader diagnostics report loaded file and valid pose count | Passed — screenshot confirmed |
| Runtime point-cloud reload still succeeds on valid sample path | Passed — screenshot confirmed |
| Invalid point-cloud reload still preserves the previous visible cloud | Passed — inherited from Milestone 8 and developer-confirmed workflow |
| Camera movement and resize/aspect behavior still work | Passed — developer confirmed |
| Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `F2`, `F3`, `F11`, `Esc` still work | Passed — developer confirmed |

## Known limitations

- Camera-pose loading is startup-only; there is no runtime camera-pose reload UI yet.
- The loader supports only the milestone eye-target-colour text format.
- No quaternion, full matrix, COLMAP, Bundler, OpenMVG or PLY camera import exists yet.
- Frustum scale is still fixed in renderer code.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Camera-pose text loader exists outside `gfx` | Passed — source implementation |
| Sample camera-pose file loads successfully | Passed — screenshot confirmed |
| Renderer visualizes file-backed camera frustums and trajectory | Passed — screenshot confirmed |
| Point-cloud reload remains functional | Passed — screenshot confirmed |
| Legacy assignment launch checks pass | Passed — developer confirmed |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 10 is complete and may be merged into `feature/sfm-visualization-sandbox`.
