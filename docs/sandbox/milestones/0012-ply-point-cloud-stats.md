# Milestone 12 Record: PLY Point-Cloud Import and Dataset Statistics

- **Milestone:** 12
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m12-ply-point-cloud-stats`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Move from milestone-only text point clouds toward real reconstruction data by adding an initial ASCII PLY point-cloud importer and dataset statistics, while preserving the existing text point-cloud loading and runtime reload behavior.

This milestone follows the roadmap goal of building a powerful, best-practice and extensible SfM visualization tool. The importer is deliberately narrow and honest: unsupported PLY features fail visibly instead of being partially or silently misread.

## Implemented source boundaries

```text
src/sandbox/scene/PointCloud.hpp/.cpp        Point-cloud statistics API
src/sandbox/scene/PointCloudLoader.hpp/.cpp  Format dispatcher and ASCII PLY loader
src/apps/SfmSandbox/main.cpp                 PLY default sample and statistics panel
res/sandbox/sample_point_cloud_ascii.ply     Tiny repo-safe ASCII PLY sample
tests/sandbox/SceneParserTests.cpp           PLY loader and statistics tests
```

## Supported PLY subset

The M12 PLY loader supports:

- `ply` magic header;
- `format ascii 1.0`;
- one `element vertex <count>` section;
- scalar vertex properties containing at least `x`, `y` and `z`;
- optional colour properties `red`, `green`, `blue` or `r`, `g`, `b`;
- colour values as normalized floats or byte-style values.

The loader deliberately rejects:

- non-ASCII PLY formats;
- vertex list properties;
- non-vertex elements with non-zero counts;
- missing `x`, `y` or `z` position fields;
- malformed headers;
- files with no valid points.

## Dataset statistics

`PointCloud::statistics()` now reports:

- point count;
- approximate CPU storage in bytes;
- whether bounds are available;
- axis-aligned minimum bounds;
- axis-aligned maximum bounds;
- bounds extent.

The `SfmSandbox` status panel displays these statistics for the active point cloud. Statistics update after successful point-cloud reloads because they are computed from the active CPU-side `PointCloud` model.

## Runtime behavior

The default point-cloud resource is now:

```text
res/sandbox/sample_point_cloud_ascii.ply
```

The earlier text sample remains available:

```text
res/sandbox/sample_point_cloud.xyzrgb
```

The runtime loader now dispatches by extension:

- `.ply` loads through the ASCII PLY loader;
- `.xyz`, `.xyzrgb` and `.txt` load through the text loader;
- unsupported extensions fail visibly.

The UI exposes both reset buttons:

- `Reset to PLY sample`;
- `Reset to text sample`.

Failed reloads still preserve the previous visible point cloud through the existing transactional renderer path.

## Test coverage

M12 extends the M11 parser tests with:

- valid ASCII PLY loading;
- byte-style and normalized RGB handling in PLY rows;
- PLY bounds and storage statistics;
- rejection of unsupported PLY format declarations;
- rejection of non-zero non-vertex elements;
- rejection of missing position fields;
- verification that the extension dispatcher preserves existing text loading;
- verification that unsupported extensions fail.

## Local validation status

The developer reported successful completion of the clean build, CTest run and manual regression checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- `ctest --test-dir build --output-on-failure` passed;
- `sfm_sandbox_scene_tests` passed in 0.07 seconds;
- screenshot of `SfmSandbox` running with the M12 status panel;
- default ASCII PLY sample loaded from `res/sandbox/sample_point_cloud_ascii.ply`;
- panel shows `Dataset point count: 21`;
- panel shows approximate CPU storage and bounds min/max/extent;
- panel shows `GPU point vertices: 21`;
- panel shows `Loaded ASCII PLY point cloud` diagnostics;
- camera-pose loading remains functional with `Camera poses: 8`;
- renderer status is ready;
- Milestone 3 shader probe remains passed;
- Milestone 2 ownership probe remains passed;
- developer confirmed all checklist items passed.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m12-ply-point-cloud-stats
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure succeeds | Passed — developer confirmed |
| Clean build succeeds | Passed — developer confirmed |
| CTest succeeds | Passed — developer confirmed |
| `sfm_sandbox_scene_tests` reports all tests passed | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches and interaction still works | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| Default PLY sample loads and renders | Passed — screenshot confirmed |
| Panel shows `Milestone 12: PLY point cloud import and dataset statistics` | Passed — screenshot confirmed |
| Panel shows positive point count and GPU point vertices | Passed — screenshot confirmed (`21`) |
| Panel shows bounds min/max/extent | Passed — screenshot confirmed |
| `Reset to text sample` then `Load / Reload` still works | Passed — developer confirmed |
| Invalid or unsupported point-cloud reload preserves previous visible cloud | Passed — developer confirmed |
| Existing camera-pose loading still works | Passed — screenshot confirmed |
| Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| Milestone 2 ownership probe still passes | Passed — screenshot confirmed |

## Known limitations

- Only ASCII PLY point clouds are supported.
- Mesh faces and topology are not imported by the point-cloud loader.
- Larger real datasets are not committed yet because dataset redistribution and license status must be checked first.
- Normals, confidence values, track IDs and image observation metadata are not parsed yet.
- Bounding box visualization is scheduled for Milestone 13.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Valid ASCII PLY sample loads and renders | Passed — screenshot confirmed |
| Malformed/unsupported PLY fails visibly without crashing | Passed — test coverage and developer validation |
| Bounding box and dataset statistics are visible | Passed — screenshot confirmed |
| Existing text point-cloud loading/reload remains functional | Passed — developer confirmed |
| Local CTest passes | Passed — developer confirmed |
| Legacy launch checks pass | Passed — developer confirmed |
| Known limitations are documented | Passed — this document |

Milestone 12 is complete and may be merged into `feature/sfm-visualization-sandbox` after CI passes.
