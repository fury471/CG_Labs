# Milestone 13 Record: Point-Cloud Inspection Controls and UX Polish

- **Milestone:** 13
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m13-pointcloud-ui`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Make point-cloud inspection more useful before adding new geometry types. M13 adds runtime point display controls, point-cloud bounds visualization, a readable UI scale control and clearer active-dataset information while preserving the M12 PLY/text loading path.

## Implemented source boundaries

```text
src/sandbox/gfx/Renderer.hpp/.cpp
src/apps/SfmSandbox/main.cpp
```

## Implemented behavior

- Runtime UI scale control with a larger default for readability.
- Runtime point-size control.
- Runtime colour mode control.
- Source colour mode.
- Height-gradient colour mode.
- Solid colour mode.
- Runtime solid-colour picker.
- Runtime point-cloud bounds toggle.
- Bounds line rendering from `PointCloud::statistics()`.
- Bounds resources are rebuilt transactionally with successful point-cloud reloads.
- Failed point-cloud reloads preserve the previous visible cloud and previous bounds.

## Roadmap alignment

| M13 roadmap item | Status |
|---|---|
| Configurable point size in UI | Implemented |
| Basic point colour/display modes | Implemented |
| Optional bounding box visualization | Implemented |
| File picker if feasible | Deferred; current path field is safer and stable |
| Clearer loader diagnostics layout | Partially implemented |
| Active dataset summary panel | Implemented |
| UX polish/readability | Implemented with runtime UI scale |

## Local validation required

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m13-pointcloud-ui
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure succeeds | Pending |
| Clean build succeeds | Pending |
| CTest succeeds | Pending |
| `EDAF80_Assignment1` launches | Pending |
| `EDAN35_Assignment2` launches | Pending |
| `SfmSandbox` launches | Pending |
| M13 panel title is visible | Pending |
| UI text is readable and UI scale slider works | Pending |
| Point-size slider visibly changes point size | Pending |
| Source colour mode works | Pending |
| Height-gradient mode works | Pending |
| Solid colour mode works | Pending |
| Bounds checkbox toggles bounding box | Pending |
| Bounds line vertex count is positive | Pending |
| PLY sample still loads | Pending |
| Text sample reload still works | Pending |
| Invalid reload preserves previous cloud | Pending |
| Dataset statistics remain accurate after reload | Pending |
| Camera-pose loading still works | Pending |
| M3 shader probe passes | Pending |
| M2 ownership probe passes | Pending |

## Known limitations

- Native file picker is deferred.
- Height-gradient mode currently uses a simple world-height mapping.
- Bounds visualization is a simple line box without labels.
- Display settings are not persisted yet.

## Completion gate

M13 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
