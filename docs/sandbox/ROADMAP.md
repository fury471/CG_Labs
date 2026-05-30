# Roadmap and Quality Gates

## Purpose

This roadmap defines the current direction for the SfM Visualization Sandbox after the completion of Milestone 10. It replaces the earlier speculative milestone list with the actual validated milestone history and a realistic backlog for the next phase.

The project remains intentionally conservative: each integration step must leave the repository understandable, buildable and locally runnable. Correctness, architectural clarity, regression safety and honest documentation take priority over quickly accumulating features.

## Current integration branch

Long-lived integration branch:

```text
feature/sfm-visualization-sandbox
```

Completed work is merged into this branch only after:

- local validation in the intended VS 2026/Ninja development workflow;
- launch checks for `EDAF80_Assignment1`, `EDAN35_Assignment2` and `SfmSandbox`;
- a milestone completion note under `docs/sandbox/milestones/`;
- pull request review/merge into the integration branch;
- CI passing on Windows, Ubuntu and macOS when CI is available for the PR.

## Roadmap status after Milestone 10

There is no active Milestone 11 in this roadmap. Any future milestone number must be added deliberately by updating this document first.

The completed M1-M10 sequence is now treated as the first prototype phase: an interactive, file-backed SfM visualization sandbox foundation. The original roadmap items that were not completed are retained below as explicit backlog, not silently counted as done.

---

## Completed prototype phase

### Milestone 0 — Baseline and documentation

**Status:** Complete.

Established the starting point before production source changes:

- `feature/sfm-visualization-sandbox` exists from the VS 2026 baseline;
- the user confirmed the Ninja/MSVC local build/run environment;
- sandbox charter, target architecture, engineering standards and initial documentation were recorded under `docs/sandbox/`.

### Milestone 1 — Application shell and target boundaries

**Status:** Complete.

Delivered:

- new `SfmSandbox` executable target;
- sandbox-specific CMake structure;
- target-local C++20 policy for new sandbox code;
- valid OpenGL window and ImGui status panel;
- clean separation from legacy assignment implementation files.

### Milestone 2 — Move-only RAII GPU resource layer

**Status:** Complete.

Delivered move-only GPU ownership foundations for new sandbox graphics code:

- buffers;
- vertex arrays;
- textures;
- samplers;
- framebuffers;
- shader-program ownership foundation;
- ownership probe visible in the sandbox status panel.

### Milestone 3 — ShaderProgram interface and cached bindings

**Status:** Complete as shader-program foundation; material separation deferred.

Delivered:

- shader compilation/link diagnostics;
- linked program ownership;
- cached uniform lookup path;
- typed uniform setting used by renderer paths;
- shader-program probe visible in the sandbox status panel.

Deferred from the earlier speculative plan:

- full material abstraction;
- file-based shader source loading;
- development shader hot reload.

### Milestone 4 — Central renderer and first rendered geometry

**Status:** Complete as central renderer foundation; full render queue deferred.

Delivered:

- `Renderer` as the central sandbox graphics boundary;
- first GPU geometry path;
- initial draw submission through renderer-owned GPU resources.

Deferred:

- generic `Mesh`, `Material`, `Renderable` model;
- render queue sorting;
- state-change statistics.

### Milestone 5 — Camera transform path and grid/axes primitive

**Status:** Complete.

Delivered:

- renderer path driven by `camera.GetWorldToClipMatrix()`;
- world-space grid and RGB axes;
- framebuffer-size based camera aspect update;
- visible 3D viewport behavior.

### Milestone 6 — PointCloud data model and first point renderer

**Status:** Complete.

Delivered:

- CPU-side `sfm::scene::PointCloud` and `PointSample` model;
- deterministic debug point cloud;
- dedicated GPU point-rendering path using `GL_POINTS`;
- CPU/GPU point-count reporting.

### Milestone 7 — Point cloud loading from file

**Status:** Complete as milestone text format loader; full `.ply` import deferred.

Delivered:

- text point-cloud loader for `xyz` and `xyzrgb` rows;
- checked-in sample point-cloud resource;
- loader diagnostics and malformed-line accounting;
- portable classic-locale float parsing after CI exposed `std::from_chars(float)` portability issues.

Deferred:

- PLY/LAS/OBJ import;
- large real datasets and redistribution review;
- bounding boxes and dataset statistics;
- configurable point display modes.

### Milestone 8 — Runtime point-cloud reload

**Status:** Complete.

Delivered:

- runtime editable point-cloud path field;
- `Load / Reload` and `Reset to sample` controls;
- transactional point-cloud GPU reload;
- failed reload preserves the previous visible cloud;
- immutable-buffer policy preserved by replacing point buffers/VAOs instead of reusing immutable storage.

### Milestone 9 — Camera pose / frustum visualization

**Status:** Complete as camera visualization foundation.

Delivered:

- CPU-side `CameraPose` and `CameraPoseSet` model;
- deterministic camera orbit;
- camera frustum line rendering;
- trajectory line rendering;
- pose/frustum counts in the status panel.

Deferred:

- instanced frustum rendering;
- pose selection;
- pose metadata panel;
- coordinate-convention tests and documentation for CV-to-graphics conversions.

### Milestone 10 — Camera pose loading from file

**Status:** Complete as milestone text format loader; product hardening remains a separate backlog item.

Delivered:

- camera-pose text loader for `eye target colour` rows;
- checked-in sample camera-pose resource;
- file-backed camera frustums and trajectory;
- skipped-line diagnostics;
- fallback to deterministic orbit if loading fails.

Deferred:

- runtime camera-pose reload;
- COLMAP/Bundler/OpenMVG import;
- quaternion/full-matrix pose formats;
- automated loader tests;
- packaging and end-user documentation.

---

## Realigned capability map

The completed prototype now supports:

- a separate `SfmSandbox` product executable;
- target-local sandbox build policy;
- RAII GPU ownership wrappers;
- shader-program ownership and cached uniform locations;
- central renderer boundary;
- 3D camera transform path;
- world grid and axes;
- point-cloud CPU data model;
- point-cloud GPU rendering;
- text point-cloud file loading;
- runtime point-cloud reload with failure preservation;
- camera-pose CPU data model;
- camera frustum and trajectory rendering;
- text camera-pose file loading;
- visible diagnostics and regression probes;
- CI-validated builds on Windows, Ubuntu and macOS for completed PRs.

This is a strong prototype foundation, not a finished reconstruction viewer.

---

## Deferred backlog from the original roadmap

The following items were present in the original roadmap but are not yet complete. They should be handled as future planned work only after choosing and documenting the next milestone sequence.

### Rendering architecture backlog

- real material abstraction;
- file-based shader source loading;
- shader reload workflow;
- generic mesh abstraction;
- renderable/submission model;
- render queue and sorting;
- draw-call and state-change statistics.

### Performance and renderer infrastructure backlog

- instanced rendering for repeated frustums/markers;
- controlled stress scene;
- draw-call comparison before/after instancing;
- resize-safe offscreen render targets;
- HDR colour render target;
- tone-mapping pass;
- GPU timing scopes and named pass profiling;
- documented timing captures with machine/GPU context.

### SfM data backlog

- PLY point-cloud import;
- larger real point-cloud datasets with documented redistribution status;
- bounding boxes and dataset statistics;
- configurable point size and display modes;
- camera-pose import from common reconstruction formats;
- coordinate-convention tests and documentation;
- pose selection and metadata inspection.

### Mesh and imagery backlog

- reconstructed mesh loading;
- mesh/point/pose joint inspection;
- image planes or overlays;
- camera-image association display;
- projective texture or projection-debug visualization.

### Product hardening backlog

- automated tests for parsers and math conventions;
- rendering regression strategy;
- asset and dataset validation;
- packaging and configuration handling;
- user/developer documentation;
- dependency, resource and license audit;
- performance target scenes and repeatable measurement procedure.

---

## Recommended next roadmap decision

Before creating another feature branch, choose one of these directions and update this document accordingly:

### Option A — Architecture consolidation

Focus on renderer structure before adding more SfM features.

Candidate work:

- material abstraction;
- mesh/renderable model;
- render queue;
- renderer statistics;
- tests for parser and transform conventions.

This is the best option if the next priority is long-term code quality.

### Option B — Data realism

Focus on importing more realistic reconstruction data.

Candidate work:

- PLY point-cloud loader;
- point-cloud bounding box and statistics;
- real sample dataset policy;
- configurable point size/display mode;
- camera-pose convention documentation.

This is the best option if the next priority is making the sandbox feel like a real SfM viewer.

### Option C — Product hardening

Focus on reliability and distributability.

Candidate work:

- parser unit tests;
- CI test targets;
- resource validation;
- documentation cleanup;
- dependency/license audit;
- performance measurement baseline.

This is the best option if the next priority is trustworthiness and maintainability.

## Rule for future milestones

Do not introduce a new milestone number by conversation alone. Future milestone numbers must be added to this roadmap first, with:

- goal;
- planned source boundaries;
- acceptance criteria;
- validation checklist;
- known exclusions.

---

## Permanent quality gates

These gates apply throughout the project.

### Build and regression gate

At all meaningful integration points:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Launch at minimum:

```text
EDAF80_Assignment1
EDAN35_Assignment2
SfmSandbox
```

### Architecture gate

- New product code goes into `src/sandbox/` or `src/apps/SfmSandbox/`.
- No SfM feature is embedded into legacy assignment implementation files.
- Higher layers do not issue raw OpenGL calls outside explicit backend/debug exceptions.
- Renderer-facing data models stay in `sandbox/scene`; OpenGL ownership stays in `sandbox/gfx`.

### Ownership gate

- New GPU ownership uses RAII owners once the relevant wrapper exists.
- Ownership-bearing types are move-only unless an explicit shared-lifetime design is documented.
- Immutable GPU storage is not reused for reload; create replacement resources or introduce an explicit dynamic-buffer path.

### Performance gate

- No optimization claim without a reproducible workload and measurement.
- Profiling measurements record configuration, scene/data scale and relevant machine/GPU context.

### Documentation gate

- Architectural policy changes receive an ADR or update to an existing ADR when they change design direction.
- Completed milestones receive a completion note and known limitations.
- Roadmap changes must distinguish completed work from deferred or planned work.

### Commercial-readiness gate

- Do not distribute inherited course assets or claim commercial readiness without license review.
