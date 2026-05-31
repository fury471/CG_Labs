# Milestone 30 - Application command and runtime decomposition

## Status

Complete.

## Delivered

- Added `SfmSandboxStartup.hpp/.cpp` for app-local startup parsing, startup
  validation and no-window mesh export commands.
- Kept `main.cpp` focused on process mode dispatch and platform/frame
  orchestration.
- Removed command-line parsing and startup validation from `SfmSandboxApp.cpp`
  so the live viewer class owns interactive state, renderer instances and UI
  composition only.
- Updated the architecture, engineering standards, roadmap and root README to
  document the app boundary.

## Validation

```bat
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build build --parallel'
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && ctest --test-dir build --output-on-failure && build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj && build\src\apps\SfmSandbox\SfmSandbox.exe --help'
build\src\apps\SfmSandbox\SfmSandbox.exe --mesh-max-points 3 --build-mesh build\sandbox-output\should_not_build.obj
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The max-point guard intentionally exits non-zero and reports that the input
exceeds the configured safety limit.

## Known limits

- `SfmSandboxApp.cpp` is still large because interactive panel extraction is
  scheduled for M31 after M28 mesh-quality diagnostics stabilize.
- Startup parsing remains app-local rather than promoted to `sandbox/core`
  because only one executable currently consumes it.
