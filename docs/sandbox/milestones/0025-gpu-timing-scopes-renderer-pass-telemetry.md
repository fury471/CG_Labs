# Milestone 25 Record: GPU Timing Scopes and Renderer Pass Telemetry

- **Milestone:** 25
- **Status:** Complete
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Complete the profiling foundation with named GPU pass timings and honest
diagnostics when timer queries are unavailable.

## Implemented Scope

- Added move-only `GpuTimerQuery`.
- Added `GpuFrameProfiler` with named scopes and delayed query polling.
- Added GPU pass timing display to the status panel.
- Timed clear, point/camera/grid, surface, image and marker passes.
- Documented measurement caveats and repeatability rules.

## Validation

Validated with:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --help
```

## Known Limitations

- GPU query visibility requires launching the interactive app on hardware with
  timer-query support.
- Query results can lag behind by a frame or more.
- The profiler intentionally reports missing support instead of substituting CPU
  timings.
