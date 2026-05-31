# Milestone 22 Record: Application State and UI Composition Cleanup

- **Milestone:** 22
- **Status:** Complete - locally validated
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Move the sandbox application away from a monolithic `main.cpp` while preserving
the existing renderer behavior, UI controls and reload safety rules.

## Implemented source boundaries

```text
src/apps/SfmSandbox/main.cpp
src/apps/SfmSandbox/SfmSandboxApp.hpp
src/apps/SfmSandbox/SfmSandboxApp.cpp
src/apps/SfmSandbox/CMakeLists.txt
docs/sandbox/ARCHITECTURE.md
docs/sandbox/milestones/0022-application-state-ui-composition.md
```

## Implemented behavior

- Added `SfmSandboxApp` as the app-local composition boundary.
- Kept `main.cpp` focused on platform/window creation, input, frame timing,
  camera update, ImGui frame setup, scene rendering and presentation.
- Moved startup project handling into the app class.
- Grouped point-cloud, camera-pose, surface and image state into focused
  app-side structs.
- Grouped reload transactions into helper methods.
- Split status-panel drawing into named app-local section methods.
- Preserved the existing `--project` and `--help` startup behavior.
- Preserved point-cloud, pose, surface and image failed-reload preservation.
- Preserved M18 render-target/profiling and M19 marker-stress workflows.

## Local validation

Validated locally on 2026-05-31 through the Visual Studio 2026 developer
environment wrapper:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --help
```

## Known limitations

- `SfmSandboxApp.cpp` still contains several UI sections in one implementation
  file. This is intentional for M22 because the panel behavior is still evolving.
- The app class is not a general framework; it is an app-local composition
  boundary for this focused viewer.
- UI settings are still not persisted.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| `main.cpp` no longer owns large unrelated data/reload/UI blocks inline | Passed |
| App state is grouped into focused app-side structs | Passed |
| Reload transactions are grouped into helper methods | Passed |
| Status-panel drawing is split into named app-local sections | Passed |
| Existing failed-reload preservation behavior is preserved | Passed |
| Documentation records the new app composition boundary | Passed |

Milestone 22 is complete. Next scheduled work is Milestone 23: textured image
planes and calibrated image groundwork.
