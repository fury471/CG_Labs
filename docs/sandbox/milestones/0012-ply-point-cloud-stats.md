# Milestone 12 Record: PLY Point-Cloud Import and Dataset Statistics

- **Milestone:** 12
- **Status:** Implementation ready for local validation
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

## Local validation required

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
| Clean configure succeeds | Pending |
| Clean build succeeds | Pending |
| CTest succeeds | Pending |
| `sfm_sandbox_scene_tests` reports all tests passed | Pending |
| Existing `EDAF80_Assignment1` launches and interaction still works | Pending |
| Existing `EDAN35_Assignment2` launches | Pending |
| `SfmSandbox` launches | Pending |
| Default PLY sample loads and renders | Pending |
| Panel shows `Milestone 12: PLY point cloud import and dataset statistics` | Pending |
| Panel shows positive point count and GPU point vertices | Pending |
| Panel shows bounds min/max/extent | Pending |
| `Reset to text sample` then `Load / Reload` still works | Pending |
| Invalid or unsupported point-cloud reload preserves previous visible cloud | Pending |
| Existing camera-pose loading still works | Pending |
| Milestone 3 shader probe still passes | Pending |
| Milestone 2 ownership probe still passes | Pending |

## Known limitations

- Only ASCII PLY point clouds are supported.
- Mesh faces and topology are not imported by the point-cloud loader.
- Larger real datasets are not committed yet because dataset redistribution and license status must be checked first.
- Normals, confidence values, track IDs and image observation metadata are not parsed yet.
- Bounding box visualization is scheduled for Milestone 13.

## Completion gate

Milestone 12 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
