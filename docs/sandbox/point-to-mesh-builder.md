# Point-to-Mesh Builder

The sandbox is evolving from a viewer into a reconstruction workbench. Milestone
27 introduces the first point-cloud-to-mesh conversion path.

## Current Builder

The current builder is intentionally constrained:

```text
input: loaded PointCloud
algorithm: projected 2D triangulation lifted back to 3D
output: SurfaceMesh and optional OBJ export
```

It is suitable for small, surface-like point clouds where one projection plane
provides a mostly single-valued surface, for example terrain-like samples or
ordered inspection patches.

It is not suitable for:

```text
closed objects with front/back overlap in projection
thin shells viewed from arbitrary directions
noisy dense scans without filtering
point clouds that require normals or volumetric reasoning
general photogrammetry reconstruction quality claims
```

## CLI Workflow

Build an OBJ mesh from the point cloud in the selected project manifest:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj
```

Use a different manifest:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe --project path\to\project.sfmproj --build-mesh build\sandbox-output\generated_surface.obj
```

The command runs without opening a window. It fails rather than silently using a
different point cloud when the selected project or point-cloud input cannot be
loaded.

Optional command-line settings:

```bat
--mesh-projection auto|xy|xz|yz
--mesh-weld 0.0001
--mesh-min-area 0.000001
--mesh-max-edge 0.0
--mesh-max-points 1000
```

`--mesh-max-edge 0.0` disables long-edge rejection.
`--mesh-max-points` is a safety limit for the experimental triangulator. Raise
it only for small controlled data; for large scans, downsample/filter first or
use a later real reconstruction backend.

## Interactive Workflow

In `SfmSandbox`, use the point-cloud-to-mesh builder controls to build the
active surface from the active point cloud. A successful build replaces the
surface renderer transactionally. A failed build keeps the previous visible
surface.

The active surface can then be exported as OBJ.

Interactive builds run in the background and are guarded by `Max projected
points` so the UI does not freeze on large point clouds. If a cloud is above the
limit, the builder reports that explicitly and keeps the current surface.

## Diagnostics and Provenance

Builder diagnostics report:

```text
input point count
unique projected point count
invalid point count
welded duplicate count
rejected triangle count
generated vertex/triangle count
resolved projection plane
```

Quality warnings are emitted for sparse projected evidence, heavy duplicate
welding and triangle filtering. These warnings do not make the result
production-grade reconstruction; they explain why the constrained triangulator
may be unsuitable for the current point distribution.

Generated OBJ files include comment metadata for the source point cloud,
algorithm, projection and builder counters. Visual-regression capture metadata
records whether the active surface is imported or generated, plus generated
builder counters when the active surface came from the mesh builder.

## Settings

```text
projection: auto, XZ, XY or YZ
weld tolerance: projected duplicate merge threshold
minimum triangle area: projected degenerate triangle rejection
maximum edge length: optional projected long-edge rejection
maximum projected points: responsiveness/safety cap
```

`auto` projects onto the two axes with the largest point-cloud extents. This is
a heuristic, not semantic camera calibration.

## Future Work

ADR 0003 selects an optional Open3D-backed adapter as the first real
reconstruction-backend direction, with Screened Poisson as the first target and
Ball Pivoting / Alpha Shapes as secondary modes. The adapter must require or
derive oriented normals, record scale-sensitive parameters and remain optional
until packaging and runtime policy are validated.

The current M27 builder remains the deterministic built-in fallback for tiny
surface-like data and regression tests.
