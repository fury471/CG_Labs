# Milestone 13 Record: Point-Cloud Inspection Controls and UX Polish

- **Milestone:** 13
- **Status:** Complete — locally validated by the developer
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

## Local validation status

The developer reported successful completion of the build/test/manual validation on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- `sfm_sandbox_scene_tests.exe` passed all 7 tests;
- `ctest --test-dir build --output-on-failure` passed;
- screenshot of `SfmSandbox` running with the M13 panel;
- panel shows the M13 title;
- point-size control is visible and was tested;
- source colour mode is visible and was tested;
- bounds checkbox is visible and enabled;
- bounds line box is visible in the viewport;
- bounds line vertex count is positive (`24`);
- PLY sample remains loaded from `res/sandbox/sample_point_cloud_ascii.ply`;
- dataset statistics remain visible and accurate for the sample;
- camera-pose loading remains functional;
- renderer status is ready;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- after review, the UI scale control was added with a larger default and the developer confirmed it looks acceptable.

Reference build commands:

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
| Clean configure succeeds | Passed — developer confirmed |
| Clean build succeeds | Passed — developer confirmed |
| CTest succeeds | Passed — developer confirmed |
| `EDAF80_Assignment1` launches | Passed — developer confirmed |
| `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| M13 panel title is visible | Passed — screenshot confirmed |
| UI text is readable and UI scale slider works | Passed — developer confirmed |
| Point-size slider visibly changes point size | Passed — developer confirmed |
| Source colour mode works | Passed — developer confirmed |
| Height-gradient mode works | Passed — developer confirmed |
| Solid colour mode works | Passed — developer confirmed |
| Bounds checkbox toggles bounding box | Passed — developer confirmed |
| Bounds line vertex count is positive | Passed — screenshot confirmed (`24`) |
| PLY sample still loads | Passed — screenshot confirmed |
| Text sample reload still works | Passed — developer confirmed |
| Invalid reload preserves previous cloud | Passed — developer confirmed |
| Dataset statistics remain accurate after reload | Passed — developer confirmed |
| Camera-pose loading still works | Passed — screenshot confirmed |
| M3 shader probe passes | Passed — screenshot confirmed |
| M2 ownership probe passes | Passed — screenshot confirmed |

## Known limitations

- Native file picker is deferred.
- Height-gradient mode currently uses a simple world-height mapping.
- Bounds visualization is a simple line box without labels.
- Display settings are not persisted yet.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Configurable point size in UI | Passed — screenshot and developer confirmation |
| Basic point colour/display modes | Passed — developer confirmation |
| Optional bounding box visualization | Passed — screenshot confirmed |
| Active dataset summary panel | Passed — screenshot confirmed |
| Existing PLY/text reload behavior preserved | Passed — developer confirmation |
| Failed reload preserves previous visible data | Passed — developer confirmation |
| UI readability addressed | Passed — UI scale control added and developer confirmed |
| Local CTest passes | Passed — developer confirmation |
| Legacy launch checks pass | Passed — developer confirmation |
| Known limitations are documented | Passed — this document |

Milestone 13 is complete and may be merged into `feature/sfm-visualization-sandbox` after CI passes.
