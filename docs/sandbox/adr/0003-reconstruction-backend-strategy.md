# ADR 0003 - Reconstruction Backend Strategy

## Status

Accepted.

## Context

The sandbox now has a constrained projected point-to-mesh builder. It is useful
for small surface-like point sets, but it is not a general reconstruction
algorithm. A product-grade workbench needs a path toward normals, outlier
filtering, scale-aware parameters and algorithms that are already validated by
the geometry-processing community.

The next backend must also fit this repository's current constraints:

- no heavy dependency is added without license and packaging review;
- inherited course assets and external datasets cannot be used as release
  evidence without redistribution review;
- the built-in M27 builder must remain deterministic and testable;
- command-line workflows should work without opening the viewer.

## Decision

Do not add an in-process reconstruction dependency yet.

The first real reconstruction backend should be designed as an optional adapter
around Open3D, with Screened Poisson as the first target algorithm and Ball
Pivoting / Alpha Shapes considered secondary modes. The adapter may initially
run as a command-line/offline workflow that reads a point cloud with normals and
writes an OBJ/PLY mesh, then the sandbox imports and inspects the result.

Keep the M27 projected triangulation builder as the built-in fallback for tiny
surface-like data and regression tests.

## Rationale

- Open3D documents Alpha Shapes, Ball Pivoting and Poisson surface
  reconstruction in one toolkit, including normal-estimation/orientation
  workflows needed by real point-cloud reconstruction.
- Open3D is MIT licensed, which is easier to evaluate for an optional tool
  adapter than a GPL-only reconstruction package.
- CGAL's Poisson package is technically strong, but the official package
  reference marks it GPL. That makes it unsuitable as the first in-process
  dependency until licensing and distribution policy are resolved.
- PCL's Greedy Projection Triangulation is relevant, but it requires point
  normals and many scale-sensitive parameters. It remains a comparison option
  rather than the first selected backend.
- An adapter boundary avoids forcing Python/Open3D/C++ ABI/package decisions
  into the viewer before representative datasets and UX expectations are known.

## Required Inputs

The Open3D adapter specification must require:

- point positions;
- oriented normals or a documented normal-estimation/orientation step;
- scale metadata or explicit reconstruction-depth/radius parameters;
- optional confidence/outlier filtering policy;
- output mesh path and provenance metadata.

## Test Data and Failure Cases

Before implementing the adapter, add small generated datasets rather than
committing external scans:

- planar patch with normals for a simple success case;
- sphere or closed synthetic surface with outward normals for Poisson/BPA;
- missing normals, inconsistent normals and sparse point sets as expected
  failures;
- duplicate-heavy and outlier-heavy point sets for diagnostic behavior;
- scale-varied datasets to verify parameter documentation.

## Consequences

- The sandbox does not claim general reconstruction quality after M29.
- M27 remains available for deterministic local tests and tiny controlled data.
- Future Open3D work must be optional, documented and packaged separately until
  dependency/runtime policy is validated.
- Any CGAL/PCL integration requires a new ADR covering license, packaging,
  platform and test-data implications.

## References

- Open3D surface reconstruction documentation:
  <https://open3d.org/docs/release/tutorial/geometry/surface_reconstruction.html>
- Open3D license:
  <https://github.com/isl-org/Open3D/blob/main/LICENSE>
- CGAL Poisson Surface Reconstruction reference:
  <https://doc.cgal.org/latest/Poisson_surface_reconstruction_3/group__PkgPoissonSurfaceReconstruction3Ref.html>
- PCL Greedy Projection Triangulation tutorial:
  <https://pcl.readthedocs.io/projects/tutorials/en/pcl-1.14.1/greedy_projection.html>
