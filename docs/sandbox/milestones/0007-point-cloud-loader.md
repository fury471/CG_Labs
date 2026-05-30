# Milestone 7 Record: Point Cloud Loading from File

- **Milestone:** 7
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m7-point-cloud-loader`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Add the first file-backed point-cloud loading path. The sandbox should load a small text point-cloud resource, validate malformed input safely, and render the loaded points through the existing `PointCloud` to `Renderer` path.

## Implemented source boundaries

```text
src/sandbox/scene/
  PointCloudLoader.hpp/.cpp       Safe text point-cloud loader and diagnostics
src/apps/SfmSandbox/main.cpp      Loads resource point cloud with procedural fallback
res/sandbox/
  sample_point_cloud.xyzrgb       Small checked-in validation point cloud
```

The loader lives in `sandbox/scene` because parsing creates CPU-side semantic point data. It does not depend on OpenGL.

## Supported file format

The loader accepts small text files with comments and either whitespace or comma separators:

```text
x y z
x y z r g b
x,y,z
x,y,z,r,g,b
```

Rules:

- text after `#` is treated as a comment;
- empty/comment-only lines are ignored;
- each data line must contain exactly 3 or 6 numeric fields;
- positions must be finite;
- rows with 3 fields receive a default light colour;
- rows with 6 fields use RGB colour;
- RGB values may be normalized `[0, 1]` or byte-style `[0, 255]`;
- out-of-range colours are clamped after normalization;
- malformed lines are skipped and counted.

This is intentionally not a full PLY/LAS/OBJ importer. It is the smallest useful loader for validating the point-cloud pipeline before a real SfM data format is selected.

## Application behavior

`SfmSandbox` now tries to load:

```text
res/sandbox/sample_point_cloud.xyzrgb
```

through:

```cpp
config::resources_path("sandbox/sample_point_cloud.xyzrgb")
```

If loading succeeds, the loaded file-backed cloud is rendered. If loading fails, the previous deterministic procedural cloud is used as a fallback so renderer validation remains possible during development.

## Runtime status panel

The status panel now includes:

```text
Milestone 7: Point cloud loading from file
Point source: resource file/procedural fallback
CPU point samples: ...
Skipped input lines: ...
```

Loader diagnostics are also shown in the panel, followed by renderer status and the existing Milestone 3/2 regression probes.

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| File loading is independent of OpenGL | Implemented in `sandbox/scene/PointCloudLoader` |
| Malformed input is handled safely | Bad lines are skipped and reported |
| Application owns source selection and fallback | Implemented in `SfmSandbox` |
| Renderer still receives a `PointCloud` model | Existing `Renderer::initialise(PointCloud const&)` path is reused |
| Existing camera transform path is preserved | Renderer still uses `camera.GetWorldToClipMatrix()` |
| Milestone 3 and 2 probes remain visible | `SfmSandbox` still displays both regression probes |
| Legacy assignments remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation required

From the Visual Studio 2026 x64 Developer Command Prompt:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m7-point-cloud-loader
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Manual launch checklist:

| Executable | Required observation | Result |
|---|---|---|
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Pending |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Pending |
| `SfmSandbox` | Window opens and the status panel is visible | Pending |
| `SfmSandbox` | Grid/axes remain visible | Pending |
| `SfmSandbox` | Loaded coloured point cloud is visible above the grid | Pending |
| `SfmSandbox` | Panel shows `Milestone 7: Point cloud loading from file` | Pending |
| `SfmSandbox` | Panel shows `Point source: resource file` | Pending |
| `SfmSandbox` | Panel shows positive CPU point sample and GPU point vertex counts | Pending |
| `SfmSandbox` | Loader diagnostics report loaded file and valid point count | Pending |
| `SfmSandbox` | Skipped input lines are reported as expected | Pending |
| `SfmSandbox` | Camera movement changes both grid and point-cloud view consistently | Pending |
| `SfmSandbox` | Resize/aspect behavior still works | Pending |
| `SfmSandbox` | Milestone 3 shader probe still passes | Pending |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Pending |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc` still work | Pending |

## Known limitations at this checkpoint

- The loader supports only a small text format, not PLY/LAS/OBJ.
- No file picker or hot reload exists yet.
- Malformed rows are skipped, but the application does not yet expose detailed per-line inspection beyond diagnostics.
- The renderer still uploads a static point cloud once at startup.
- No streaming, LOD, spatial indexing, point picking, camera pose rendering or trajectory rendering exists yet.

## Completion gate

Milestone 7 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
