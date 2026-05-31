# Milestone 29 - Real reconstruction algorithm integration decision

## Status

Complete.

## Delivered

- Added ADR 0003 for the reconstruction backend strategy.
- Selected an optional Open3D-backed adapter as the first real reconstruction
  direction, with Screened Poisson first and Ball Pivoting / Alpha Shapes as
  secondary modes.
- Recorded why CGAL and PCL remain comparison/future options rather than the
  first dependency to integrate.
- Defined generated synthetic test-data requirements and failure cases for the
  future adapter.
- Preserved the M27 projected triangulation builder as the built-in
  deterministic fallback.

## Validation

This milestone is a design/integration decision. It was validated by checking
official documentation for candidate algorithms and licenses, then recording
the selected dependency boundary and test expectations in ADR 0003.

## Known limits

- No Open3D adapter is implemented in this milestone.
- The sandbox still does not claim general-purpose reconstruction quality.
