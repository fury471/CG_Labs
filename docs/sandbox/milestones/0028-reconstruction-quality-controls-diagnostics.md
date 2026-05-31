# Milestone 28 - Reconstruction quality controls and diagnostics

## Status

Complete.

## Delivered

- Extended point-to-mesh diagnostics with invalid input counts and quality
  warnings for sparse projected evidence, heavy welding and triangle filtering.
- Exposed input, unique projected, invalid, duplicate, rejected and output
  counters in the mesh-builder UI.
- Added multi-line OBJ export comments so generated meshes carry source,
  algorithm, projection and build-counter provenance.
- Added surface kind/source-format metadata to visual captures, with generated
  mesh counters recorded when the active surface is builder-generated.
- Extended export tests to verify multi-line OBJ comments are prefixed
  correctly.

## Validation

```bat
cmd /c '"D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build build --parallel && ctest --test-dir build --output-on-failure && build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj && build\src\apps\SfmSandbox\SfmSandbox.exe --help'
build\src\apps\SfmSandbox\SfmSandbox.exe --mesh-max-points 3 --build-mesh build\sandbox-output\should_not_build.obj
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The max-point command intentionally exits non-zero and reports the configured
safety-limit refusal.

## Evidence

- `build/sandbox-output/generated_surface.obj` begins with builder provenance
  comments for source, algorithm, projection and counters.
- `build/sandbox-captures/sfm-baseline.ppm.txt` records `surface_kind`,
  `surface_vertices`, `surface_triangles` and `surface_source_format`.

## Known limits

- The constrained triangulator still does not provide normals, outlier
  filtering, volumetric reasoning or closed-surface reconstruction.
- Rejected-triangle visualization remains deferred until the UI has a dedicated
  diagnostic overlay model.
