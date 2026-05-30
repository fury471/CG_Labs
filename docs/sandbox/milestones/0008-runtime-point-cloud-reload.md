# Milestone 8 Record: Runtime File Selection / Reload Path

- **Milestone:** 8
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m8-runtime-point-cloud-reload`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Allow the sandbox to load or reload a point-cloud text file at runtime while preserving the existing grid/axes view, camera transform path, diagnostics and regression probes.

## Implemented source boundaries

```text
src/apps/SfmSandbox/main.cpp
src/sandbox/gfx/Renderer.hpp/.cpp
src/sandbox/scene/PointCloud.hpp
src/sandbox/scene/PointCloudLoader.hpp
```

## Runtime UI

The status panel now contains an editable point-cloud path field, a `Load / Reload` button, a `Reset to sample` button, active-source text, last-reload status, successful reload count, loader diagnostics, CPU point count and GPU point count.

`Reset to sample` only restores the path text field. The developer still presses `Load / Reload` to make the reload explicit.

## Safe reload policy

`Renderer` now exposes:

```cpp
[[nodiscard]] RendererBuildResult reload_point_cloud(sfm::scene::PointCloud const& point_cloud);
```

The renderer keeps the grid/axes pipeline alive and replaces only the point-cloud GPU resources. Because current buffers use immutable storage, reload creates replacement point buffers and vertex arrays instead of trying to reuse an existing immutable buffer.

Reload is transactional: replacement GPU resources are committed only after the full replacement path succeeds. A failed reload keeps the previous visible point cloud.

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot showing the runtime reload UI;
- valid sample reload succeeds after `Reset to sample` then `Load / Reload`;
- `Last reload: Reload succeeded` is visible;
- successful reload count increments after repeated valid reloads;
- active source is `res/sandbox/sample_point_cloud.xyzrgb`;
- CPU point samples and GPU point vertices both report `21`;
- skipped input lines report `0`;
- invalid/malformed reload reports failure while the previous point cloud remains visible;
- renderer remains ready after failed reload;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- developer confirmation that camera movement, resize/aspect behavior, legacy regression and sandbox control checks also passed.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m8-runtime-point-cloud-reload
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
| Grid/axes and sample point cloud are visible on startup | Passed — screenshot confirmed |
| Panel shows Milestone 8 runtime reload section | Passed — screenshot confirmed |
| Pressing `Load / Reload` on the valid sample path succeeds | Passed — screenshot confirmed |
| Successful reload counter increments | Passed — screenshot confirmed |
| CPU and GPU point counts remain positive and match | Passed — screenshot confirmed (`21`) |
| Entering an invalid path reports failure | Passed — screenshot confirmed |
| Invalid reload keeps the previous point cloud visible | Passed — screenshot confirmed |
| `Reset to sample` restores the path field | Passed — developer confirmed |
| Camera movement and resize/aspect behavior still work | Passed — developer confirmed |
| Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `F2`, `F3`, `F11`, `Esc` still work | Passed — developer confirmed |

## Known limitations

- The runtime UI uses a text path field, not a native file picker.
- Reload is synchronous on the UI thread.
- Point buffers and vertex arrays are rebuilt on successful reload.
- The loader still supports only the small milestone text format.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Valid runtime reload updates the active point cloud | Passed — screenshot confirmed |
| Failed runtime reload preserves the previous visible cloud | Passed — screenshot confirmed |
| CPU/GPU point counts match after reload | Passed — screenshot confirmed (`21`) |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 8 is complete and may be merged into `feature/sfm-visualization-sandbox`.
