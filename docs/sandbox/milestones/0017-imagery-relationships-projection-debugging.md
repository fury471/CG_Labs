# Milestone 17 Record: Imagery Relationships and Projection Debugging

- **Milestone:** 17
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m17-imagery-projection-debug`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Connect camera poses with source imagery so the relationship between points, cameras and image evidence can be inspected visually.

M17 deliberately introduces a small, inspectable image-association and projection-debug path rather than a full image-processing or calibrated reprojection pipeline.

## Implemented source boundaries

```text
docs/sandbox/image-projection-debugging.md
docs/sandbox/milestones/0017-imagery-relationships-projection-debugging.md
res/sandbox/sample_camera_image.ppm
src/sandbox/scene/ImageImport.hpp
src/sandbox/gfx/ImagePlaneRenderer.hpp
src/apps/SfmSandbox/main.cpp
tests/sandbox/ImageImportSmokeTest.cpp
tests/sandbox/CMakeLists.txt
```

## Implemented behavior

- Added `ImageResource` as a small CPU-side image representation.
- Added `ImageImportResult` for explicit image-import success/failure reporting.
- Added dependency-free ASCII PPM P3 image import.
- Added `res/sandbox/sample_camera_image.ppm` as a synthetic, project-owned sample image.
- Added `ImagePlaneRenderer` under `sandbox/gfx`.
- Kept image-plane OpenGL calls inside the graphics layer.
- Reused the existing file-backed colour shader path for the debug image card.
- Added a camera-image association panel in `SfmSandbox`.
- Added image path reload and reset controls.
- Added selected-camera association control.
- Added image-plane visibility toggle.
- Added image-plane distance and height controls.
- Added image-plane draw statistics to the renderer status panel.
- Added image import messages and image-plane renderer messages to the UI.
- Added `docs/sandbox/image-projection-debugging.md` to document the supported sample format and projection sanity check.
- Added `sfm_image_import_smoke_test` for deterministic CPU-side importer coverage.

## Supported sample image format

M17 supports one deliberately tiny, license-safe image format:

```text
ASCII PPM P3
```

Example structure:

```text
P3
width height
max_value
r g b ...
```

This avoids external image-library dependency and avoids sample-asset license ambiguity for the first image-association milestone.

## Projection sanity check

The image plane is built in the selected camera's local coordinate system, using the convention established by M14:

```text
+X = camera right
+Y = camera up
-Z = camera forward
```

The image card centre is placed at:

```text
local centre = (0, 0, -image_plane_distance)
world centre = camera_to_world * local centre
```

The expected visual result is that the image card lies along the selected camera frustum's forward ray. This is documented in `docs/sandbox/image-projection-debugging.md` and displayed in the UI as the M17 projection sanity note.

## Roadmap alignment

| M17 roadmap item | Status |
|---|---|
| Image plane or image overlay representation | Implemented as coloured debug image plane |
| Camera-image association data model | Implemented as one loaded image associated with one selected camera index |
| Simple image resource loading with license-safe sample asset | Implemented through ASCII PPM P3 sample |
| Optional projection-debug visualization | Implemented through camera-local image card placement |
| Projection sanity check | Implemented and documented |

## Local validation status

The developer reported successful local validation on 2026-05-30 using the intended VS 2026/Ninja workflow.

Evidence received:

- `SfmSandbox` launched successfully;
- screenshot shows the M17 camera image association panel;
- screenshot shows the synthetic image loaded from `res/sandbox/sample_camera_image.ppm`;
- screenshot shows image size and pixel count;
- screenshot shows the image-plane renderer ready;
- screenshot shows a coloured image card near the selected camera frustum;
- screenshot shows point cloud, surface mesh, bounds and camera frustums still visible;
- developer confirmed the other checks also work.

Reference validation commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m17-imagery-projection-debug
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure/build | Passed — developer confirmed validation |
| CTest | Passed — developer confirmed validation |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| M17 image association panel visible | Passed — screenshot confirmed |
| Sample image loads | Passed — screenshot confirmed |
| Image-plane renderer ready | Passed — screenshot confirmed |
| Image card visible near associated camera frustum | Passed — screenshot confirmed |
| Associated camera control works | Passed — developer confirmed all work |
| Image-plane distance/height controls work | Passed — developer confirmed all work |
| Image-plane draw statistics visible | Passed — screenshot confirmed |
| Point cloud remains visible | Passed — screenshot confirmed |
| Surface mesh remains visible | Passed — screenshot confirmed |
| Camera frustums remain visible | Passed — screenshot confirmed |
| Point-cloud reload remains functional | Passed — developer confirmed all work |
| Surface reload remains functional | Passed — developer confirmed all work |
| Camera-pose reload remains functional | Passed — developer confirmed all work |
| `EDAF80_Assignment1` launch check | Passed — developer confirmed all work |
| `EDAN35_Assignment2` launch check | Passed — developer confirmed all work |

## Known limitations

- This is not a full image-processing pipeline.
- Only ASCII PPM P3 is supported.
- The image plane is a coloured debug card using sampled corner colours, not a textured OpenGL sampler path.
- Intrinsics are not imported yet.
- Dense point reprojection errors are not computed yet.
- Only one image is associated with one selected camera at a time.
- The projection sanity check validates camera-local placement visually, not calibrated photogrammetric reprojection accuracy.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| At least one camera can display or reference an associated image | Passed |
| Image/frustum/point relationship is visually understandable | Passed |
| Projection behavior is documented and checked against a known synthetic case | Passed |
| Existing point-cloud, surface and camera-pose workflows remain available | Passed |
| Local validation completed | Passed |
| Known limitations documented | Passed |

Milestone 17 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
