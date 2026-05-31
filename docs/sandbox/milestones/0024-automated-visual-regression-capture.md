# Milestone 24 Record: Automated Visual Regression Capture Path

- **Milestone:** 24
- **Status:** Complete
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Reduce reliance on purely manual screenshot checks by adding a deterministic
capture command and metadata report for the default startup scene.

## Implemented Scope

- Added `--capture-baseline <output.ppm>`.
- Captured the rendered back buffer as binary PPM after the scene pass and
  before ImGui submission.
- Wrote metadata beside the capture.
- Moved framebuffer readback into `sandbox/gfx`.
- Kept manual screenshot comparison as the platform fallback.

## Validation

Validated with:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --help
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The local capture produced `build\sandbox-captures\sfm-baseline.ppm` and
`build\sandbox-captures\sfm-baseline.ppm.txt`. OpenGL reported the expected
synchronous pixel-transfer performance warning during readback.

## Known Limitations

- Capture still requires a platform window and OpenGL context.
- The PPM output is an evidence artifact, not a polished export format.
- Pixel comparison thresholds are not automated yet.
