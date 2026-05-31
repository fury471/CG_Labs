# Milestone 31 - Mesh-builder workflow and panel extraction

## Status

Complete.

## Delivered

- Added `SfmSandboxMeshWorkflow` as an app-local service for asynchronous mesh
  builds, point-limit refusal, export bookkeeping and diagnostics.
- Added `SfmSandboxMeshBuilderPanel` so the mesh-builder ImGui controls return
  explicit build/export actions instead of owning workflow or renderer state.
- Kept generated-surface GPU upload and prior-surface preservation in
  `SfmSandboxApp`, where renderer ownership already lives.
- Added `sfm_sandbox_mesh_workflow_tests` for the workflow service.

## Validation

```bat
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build build --parallel'
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && ctest --test-dir build --output-on-failure && build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj && build\src\apps\SfmSandbox\SfmSandbox.exe --help'
build\src\apps\SfmSandbox\SfmSandbox.exe --mesh-max-points 3 --build-mesh build\sandbox-output\should_not_build.obj
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The max-point guard intentionally exits non-zero and reports the configured
safety-limit refusal.

## Known limits

- Other status-panel sections are still app member functions. They should be
  split only when they gain workflow state or enough complexity to justify the
  extra files.
- Mesh quality diagnostics remain scheduled separately from this structural
  refactor.
