# Milestone 19 Record: Instancing and Workload Stress Scenes

- **Milestone:** 19
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-31
- **Development branch:** `feature/sfm-m19-instancing-stress`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Evaluate repeated-object rendering after the M18 profiling baseline exists.

M19 adds a controlled synthetic repeated-marker workload and an instanced rendering path so draw-call reduction can be compared directly against a reference non-instanced path.

## Implemented source boundaries

```text
docs/sandbox/instancing-stress-m19.md
docs/sandbox/milestones/0019-instancing-workload-stress-scenes.md
shaders/sandbox/instanced_marker.vert
src/sandbox/gfx/InstancedMarkerRenderer.hpp
src/apps/SfmSandbox/main.cpp
```

## Implemented behavior

- Added a deterministic synthetic marker stress scene.
- Added `InstancedMarkerRenderer` under `sandbox/gfx`.
- Kept raw OpenGL calls inside the graphics layer.
- Added a small three-axis marker glyph rendered as line primitives.
- Added per-marker instance offsets and colours.
- Added an instanced marker vertex shader.
- Added M19 controls in `SfmSandbox`.
- Added marker visibility control.
- Added toggle between reference non-instanced mode and instanced mode.
- Added marker count, radius, height and scale controls.
- Added stress-scene rebuild action.
- Added active draw-call count, reference draw-call count, instanced draw-call count, vertex count and draw-call reduction reporting.
- Integrated marker draw statistics into the renderer status panel.
- Preserved the M18 frame profiler and render-target diagnostics.
- Documented the stress-scene measurement procedure in `docs/sandbox/instancing-stress-m19.md`.

## Rendering paths

M19 compares two paths for the same repeated-marker workload:

```text
reference non-instanced path: one draw call per marker
instanced path: one instanced draw call for all markers
```

The comparison is intentionally focused on repeated debug glyphs. It does not claim that every future SfM visualization object should use this exact path.

## Roadmap alignment

| M19 roadmap item | Status |
|---|---|
| Instanced rendering for repeated frustums, markers or debug glyphs | Implemented for repeated debug markers |
| Controlled stress scene with many camera poses/markers | Implemented as deterministic marker spiral/ring scene |
| Before/after draw-call counts | Implemented in UI |
| Before/after CPU/GPU timing captures | CPU timing visible through M18 profiler; GPU timing remains a known limitation |
| Documented limitations and hardware context | Implemented in M19 documentation and this record |
| Repeated-object scene avoids one draw call per object where instancing applies | Passed in instanced mode |
| Timing results demonstrate or honestly refute expected improvement | UI and docs require honest interpretation; no broad speedup claim is made |
| Visual output matches non-instanced reference behavior | Validated by developer |

## Local validation status

The developer reported successful local validation on 2026-05-31 using the intended VS 2026/Ninja workflow.

Evidence received:

- `SfmSandbox` launched successfully;
- screenshot shows the M19 panel title;
- screenshot shows `Marker renderer: ready`;
- screenshot shows a large repeated-marker stress scene visible in the viewport;
- screenshot shows `Use instanced marker path` enabled;
- screenshot shows active marker draw calls equal to 1;
- screenshot shows reference non-instanced draw calls equal to the marker count;
- screenshot shows instanced draw calls equal to 1;
- screenshot shows draw-call reduction calculated from the reference count;
- screenshot shows M18 profiling and render-target baseline still visible;
- screenshot shows point cloud, surface mesh, bounds, camera frustums and image-plane workflows still present;
- developer confirmed all other checks work.

Reference validation commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m19-instancing-stress
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
| M19 panel title visible | Passed — screenshot confirmed |
| Marker renderer ready | Passed — screenshot confirmed |
| Marker stress scene visible | Passed — screenshot confirmed |
| Instanced path reports one active draw call | Passed — screenshot confirmed |
| Reference path count equals marker count | Passed — screenshot confirmed |
| Draw-call reduction visible | Passed — screenshot confirmed |
| Visual output remains equivalent between modes | Passed — developer confirmed all work |
| Marker count/radius/height/scale controls work | Passed — developer confirmed all work |
| M18 CPU timings remain visible | Passed — screenshot confirmed |
| Render-target diagnostics remain visible | Passed — screenshot confirmed |
| Point cloud remains visible | Passed — screenshot confirmed |
| Surface mesh remains visible | Passed — screenshot confirmed |
| Camera frustums remain visible | Passed — screenshot confirmed |
| Image plane workflow remains available | Passed — screenshot confirmed |
| Existing reload workflows remain functional | Passed — developer confirmed all work |
| `EDAF80_Assignment1` launch check | Passed — developer confirmed all work |
| `EDAN35_Assignment2` launch check | Passed — developer confirmed all work |

## Known limitations

- M19 records CPU-side named timings through the M18 profiler; GPU timer-query scopes remain a later extension.
- The stress scene uses synthetic debug markers, not real SfM feature tracks.
- The reference non-instanced path is a controlled comparison path, not a fully optimized alternative renderer.
- Runtime timing results depend on hardware, driver, build type, framebuffer size, UI visibility and camera view.
- M19 can directly claim draw-call reduction for this repeated-marker workload, but it must not claim general performance improvement without repeated controlled measurements.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Repeated-object scene avoids one draw call per object where instancing applies | Passed |
| Timing results demonstrate or honestly refute the expected improvement | Supported through M18 profiler and M19 documentation |
| Visual output matches non-instanced reference behavior | Passed |
| Existing point-cloud, surface, camera-pose, image and profiling workflows remain available | Passed |
| Local validation completed | Passed |
| Known limitations documented | Passed |

Milestone 19 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
