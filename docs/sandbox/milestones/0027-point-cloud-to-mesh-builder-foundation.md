# Milestone 27 Record: Point-Cloud-to-Mesh Builder Foundation

- **Milestone:** 27
- **Status:** Complete
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Move the sandbox beyond passive viewing by adding the first constrained
point-cloud-to-mesh conversion path.

## Implemented Scope

- CPU-side builder module for point-cloud triangulation.
- OBJ export for generated meshes.
- CLI conversion through `--build-mesh <output.obj>`.
- Interactive build/export controls in `SfmSandbox`.
- Background interactive builds plus a max projected point guard to prevent UI
  stalls on large clouds.
- Tests for triangulation, degenerate input and export/import round-trip.
- Documentation that states algorithm limits honestly.

## Validation

Validated with:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj
build\src\apps\SfmSandbox\SfmSandbox.exe --validate-install
cmake --install build --prefix build\install-sfm-sandbox
pushd build\install-sfm-sandbox\bin
SfmSandbox.exe --validate-install
SfmSandbox.exe --build-mesh ..\generated_from_install.obj
popd
```

The default project point cloud generated:

```text
21 vertices
36 triangles
OBJ output: build\sandbox-output\generated_surface.obj
```

## Known Limits

The first builder handles surface-like point clouds that project cleanly onto a
2D plane. It is not a general dense-scan, closed-surface or Poisson
reconstruction algorithm.

The default safety cap is 1000 projected points. Large scans should be
downsampled or processed by a later dedicated reconstruction backend.
