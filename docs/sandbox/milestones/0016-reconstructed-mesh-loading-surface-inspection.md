# Milestone 16 Record: Reconstructed Mesh Loading and Surface Inspection

- **Milestone:** 16
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m16-mesh-inspection`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Add the first reconstructed-surface inspection capability without turning the sandbox into a general asset importer.

M16 introduces a small CPU-side surface mesh model, a documented ASCII OBJ subset importer, a sample surface resource, a graphics-layer surface renderer, and UI controls for viewing the surface together with the existing point cloud and camera-pose visualization.

## Implemented source boundaries

```text
docs/sandbox/AI_COMMIT_CI_POLICY.md
docs/sandbox/surface-mesh-format.md
docs/sandbox/milestones/0016-reconstructed-mesh-loading-surface-inspection.md
res/sandbox/sample_surface.obj
src/sandbox/scene/Mesh.hpp
src/sandbox/scene/Mesh.cpp
src/sandbox/scene/SurfaceImport.hpp
src/sandbox/gfx/Material.hpp
src/sandbox/gfx/SurfaceRenderer.hpp
src/apps/SfmSandbox/main.cpp
tests/sandbox/SurfaceImportSmokeTest.cpp
tests/sandbox/CMakeLists.txt
```

## Supported mesh format

M16 supports a deliberately tiny ASCII OBJ subset:

```text
v x y z
f i j k
```

OBJ indices are one-based in the file and converted to zero-based indices internally.

The following features remain out of scope for M16:

```text
materials
texture coordinates
normals
quads
negative indices
smoothing groups
object/group semantics
binary mesh formats
```

The format is documented in `docs/sandbox/surface-mesh-format.md`.

## Implemented behavior

- Added a CPU-side surface mesh model in the scene layer.
- Added a small OBJ-subset surface importer.
- Added `res/sandbox/sample_surface.obj` as the default sample surface.
- Added `SurfaceRenderer` in `sandbox/gfx`.
- Kept surface rendering OpenGL calls inside `sandbox/gfx`.
- Surface rendering uses the existing file-backed colour shader path.
- Added surface loading at sandbox startup.
- Added surface reload UI.
- Failed surface reloads keep the previous visible surface.
- Added surface visibility toggle.
- Added surface colour editing.
- Updated colour editing so changes apply immediately to the visible surface.
- Kept the manual surface-colour rebuild button as a fallback/debug action.
- Added surface vertex, triangle, bounds and CPU-storage statistics to the UI.
- Added surface draw statistics beside existing renderer statistics.
- Added a CPU-side smoke test for surface import.
- Preserved point-cloud reload and camera-pose reload behavior.

## Roadmap alignment

| M16 roadmap item | Status |
|---|---|
| Simple mesh loader for one documented format | Implemented: M16 ASCII OBJ subset |
| CPU-side mesh data model | Implemented |
| GPU mesh upload path | Implemented through `sandbox/gfx/SurfaceRenderer` |
| Basic reconstruction-inspection material | Implemented as colour-shaded surface triangles |
| Mesh visibility toggle | Implemented |
| Mesh bounds/statistics | Implemented |
| Surface visible with point cloud and camera frustums | Validated by developer screenshot |
| Malformed input has visible feedback | Implemented through surface import messages and reload status |
| Renderer stats include surface draw calls/triangles | Implemented |

## Local validation status

The developer reported successful local validation on 2026-05-30 using the intended VS 2026/Ninja workflow.

Evidence received:

- `SfmSandbox` launched successfully;
- screenshot shows the M16 panel title;
- screenshot shows point cloud, camera frustums, bounds and surface mesh visible together;
- screenshot shows surface source loaded from `res/sandbox/sample_surface.obj`;
- screenshot shows surface vertex count, triangle count, bounds and source messages;
- screenshot shows `Surface renderer: ready`;
- screenshot shows surface draw statistics with surface triangles drawn;
- developer confirmed the application works;
- developer identified that surface colour did not initially update live;
- the colour-update behavior was fixed;
- developer confirmed the surface colour now updates correctly.

Reference validation commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m16-mesh-inspection
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual checklist:

| Check | Result |
|---|---|
| Clean configure/build | Passed — developer confirmed local validation |
| CTest | Passed — developer confirmed local validation |
| `SfmSandbox` launches | Passed — screenshot confirmed |
| M16 panel title visible | Passed — screenshot confirmed |
| Surface mesh visible | Passed — screenshot confirmed |
| Surface appears with point cloud and camera frustums | Passed — screenshot confirmed |
| Surface renderer ready | Passed — screenshot confirmed |
| Surface statistics visible | Passed — screenshot confirmed |
| Surface draw statistics visible | Passed — screenshot confirmed |
| Surface visibility toggle works | Passed — developer confirmed behavior works |
| Surface reload works | Passed — screenshot confirmed successful reload state |
| Surface colour update works live | Passed — developer confirmed after fix |
| Point-cloud reload remains functional | Passed — developer confirmed local validation |
| Camera-pose reload remains functional | Passed — developer confirmed local validation |
| `EDAF80_Assignment1` launch check | Passed — developer confirmed local validation |
| `EDAN35_Assignment2` launch check | Passed — developer confirmed local validation |

## Known limitations

- This is not a general OBJ importer.
- Normals, UVs, materials, quads and negative indices are intentionally unsupported.
- The surface material is a basic colour material, not lit/PBR shading.
- Surface colour is currently baked into the uploaded vertex data and refreshed when the colour changes; a uniform-based material path can replace this later.
- `SurfaceImport.hpp` is currently header-only to avoid changing scene-library source registration during this milestone. A later cleanup can split it once the model stabilizes.
- `SurfaceRenderer` is a focused M16 graphics-layer renderer rather than a full asset/render-graph integration.
- Surface depth, transparency and occlusion behavior are basic and may need refinement when image planes and richer reconstruction inspection arrive.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Sample reconstructed mesh loads | Passed |
| Sample mesh renders in `SfmSandbox` | Passed |
| Mesh, point cloud and camera frustums visible together | Passed |
| Malformed input feedback is visible | Implemented |
| Failed reload preserves previous visible surface | Implemented |
| Renderer statistics include surface drawing | Passed |
| Local validation completed | Passed |
| Known limitations documented | Passed |

Milestone 16 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
