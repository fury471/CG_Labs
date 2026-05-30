# Milestone 18 Record: Render Targets, Profiling and Performance Baseline

- **Milestone:** 18
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-31
- **Development branch:** `feature/sfm-m18-profiling-baseline`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Establish measurable rendering infrastructure before making optimization claims.

M18 adds resize-safe render-target validation, framebuffer completeness diagnostics, named CPU frame-pass timings, consolidated status-panel reporting and a baseline measurement procedure for the default SfM sandbox scene.

## Implemented source boundaries

```text
docs/sandbox/performance-baseline-m18.md
docs/sandbox/milestones/0018-render-targets-profiling-performance-baseline.md
src/sandbox/core/FrameProfiler.hpp
src/sandbox/gfx/RenderTarget.hpp
src/apps/SfmSandbox/main.cpp
```

## Implemented behavior

- Added `sfm::gfx::RenderTarget` as a resize-safe offscreen target probe.
- Added colour-target allocation using `GL_RGBA16F` storage.
- Added framebuffer completeness validation through `glCheckNamedFramebufferStatus`.
- Added human-readable framebuffer status messages.
- Added rebuild count tracking for resize validation.
- Added `sfm::core::FrameProfiler` for named CPU pass timings.
- Added M18 status-panel section in `SfmSandbox`.
- Added displayed offscreen target state, size, framebuffer id, colour texture id and rebuild count.
- Added displayed named CPU timings for input/update, resize/render-target validation, ImGui setup, scene drawing, status panel UI, ImGui submission and swap buffers.
- Added baseline scene description in the UI.
- Added `docs/sandbox/performance-baseline-m18.md` to document the measurement procedure.
- Explicitly avoids optimization claims in M18.

## Roadmap alignment

| M18 roadmap item | Status |
|---|---|
| Resize-safe offscreen render targets | Implemented as offscreen render-target probe |
| Framebuffer completeness diagnostics | Implemented and visible in UI |
| Optional HDR colour target | Implemented as `GL_RGBA16F` colour target for inspection/profiling infrastructure |
| GPU timing scopes and named pass timings | Named CPU pass timings implemented; GPU timing remains a documented later extension |
| CPU frame statistics and draw-call statistics consolidated | Implemented in M18 status panel |
| Archived baseline measurements for representative scenes | Baseline procedure documented; developer screenshot records representative values |
| Resize does not break render targets | Validated by developer |
| Named pass timings are visible | Validated by developer screenshot |
| Baseline scenes and measurement configuration documented | Implemented |
| No performance claim is made without evidence | Implemented in UI and docs |

## Local validation status

The developer reported successful local validation on 2026-05-31 using the intended VS 2026/Ninja workflow.

Evidence received:

- `SfmSandbox` launched successfully;
- screenshot shows the M18 panel title;
- screenshot shows `Offscreen target: complete`;
- screenshot shows target size matching a large framebuffer extent;
- screenshot shows framebuffer id and colour texture id;
- screenshot shows rebuild count after resize;
- screenshot shows `GL_FRAMEBUFFER_COMPLETE` diagnostics;
- screenshot shows named CPU pass timings;
- screenshot shows baseline scene description and no optimization-claim wording;
- screenshot shows point cloud, surface mesh, bounds, camera frustums and image-plane workflows still present;
- developer confirmed all other checks also work.

Reference validation commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m18-profiling-baseline
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
| M18 panel title visible | Passed — screenshot confirmed |
| Offscreen target reports complete | Passed — screenshot confirmed |
| Framebuffer completeness status visible | Passed — screenshot confirmed |
| Target size follows framebuffer size | Passed — screenshot confirmed |
| Rebuild count visible after resize | Passed — screenshot confirmed |
| Named CPU pass timings visible | Passed — screenshot confirmed |
| Draw statistics remain visible | Passed — developer confirmed all work |
| Point cloud remains visible | Passed — screenshot confirmed |
| Surface mesh remains visible | Passed — screenshot confirmed |
| Camera frustums remain visible | Passed — screenshot confirmed |
| Image plane workflow remains available | Passed — screenshot confirmed |
| Point-cloud reload remains functional | Passed — developer confirmed all work |
| Surface reload remains functional | Passed — developer confirmed all work |
| Camera-pose reload remains functional | Passed — developer confirmed all work |
| Image reload remains functional | Passed — developer confirmed all work |
| `EDAF80_Assignment1` launch check | Passed — developer confirmed all work |
| `EDAN35_Assignment2` launch check | Passed — developer confirmed all work |

## Known limitations

- M18 records CPU-side named timings only; GPU timer-query scopes remain a later extension.
- The offscreen render target is currently a validation/profiling probe, not yet the active presentation path.
- The `GL_RGBA16F` colour target is allocated to validate render-target infrastructure; tone mapping is not implemented in M18.
- Timing values are baseline instrumentation, not optimization evidence.
- Baselines should always be reported with build type, framebuffer size, hardware context and commit/PR reference.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Resizing does not break render targets | Passed |
| Named pass timings are visible | Passed |
| Baseline scenes and measurement configuration are documented | Passed |
| No performance claim is made without measurement evidence | Passed |
| Existing point-cloud, surface, camera-pose and image workflows remain available | Passed |
| Local validation completed | Passed |
| Known limitations documented | Passed |

Milestone 18 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
