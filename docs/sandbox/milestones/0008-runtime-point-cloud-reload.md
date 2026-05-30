# Milestone 8 Record: Runtime File Selection / Reload Path

- **Milestone:** 8
- **Status:** Implementation ready for local validation
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

## Local validation required

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
| Existing `EDAF80_Assignment1` launches and interaction still works | Pending |
| Existing `EDAN35_Assignment2` launches | Pending |
| `SfmSandbox` launches with status panel | Pending |
| Grid/axes and sample point cloud are visible on startup | Pending |
| Panel shows Milestone 8 runtime reload section | Pending |
| Pressing `Load / Reload` on the valid sample path succeeds | Pending |
| Successful reload counter increments | Pending |
| CPU and GPU point counts remain positive and match | Pending |
| Entering an invalid path reports failure | Pending |
| Invalid reload keeps the previous point cloud visible | Pending |
| `Reset to sample` restores the path field | Pending |
| Camera movement and resize/aspect behavior still work | Pending |
| Milestone 3 shader probe still passes | Pending |
| Milestone 2 ownership probe still passes | Pending |
| `F2`, `F3`, `F11`, `Esc` still work | Pending |

## Known limitations

- The runtime UI uses a text path field, not a native file picker.
- Reload is synchronous on the UI thread.
- Point buffers and vertex arrays are rebuilt on successful reload.
- The loader still supports only the small milestone text format.

## Completion gate

Milestone 8 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
