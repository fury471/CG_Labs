# Milestone 32 - Tooling, regression and release discipline hardening

## Status

Complete.

## Delivered

- Added `sfm_sandbox_mesh_workflow_tests` to cover the app-local mesh workflow.
- Kept command workflows under CTest through startup validation and CLI mesh
  export tests.
- Updated dependency helper CMake files to use `FetchContent_MakeAvailable`
  instead of deprecated direct `FetchContent_Populate` calls.
- Used `SOURCE_SUBDIR` for source-only third-party fetches so upstream CMake
  projects are not accidentally added where the sandbox owns integration.

## Validation

```bat
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build build --parallel'
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && ctest --test-dir build --output-on-failure && build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj && build\src\apps\SfmSandbox\SfmSandbox.exe --help'
build\src\apps\SfmSandbox\SfmSandbox.exe --mesh-max-points 3 --build-mesh build\sandbox-output\should_not_build.obj
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The max-point command intentionally exits non-zero and reports the configured
safety-limit refusal. The capture path still reports the expected OpenGL
pixel-transfer synchronization performance warning during framebuffer readback.

## Known limits

- Dependency versions remain pinned to the branch's existing versions. This
  milestone modernizes the FetchContent integration path; it does not upgrade
  third-party libraries.
