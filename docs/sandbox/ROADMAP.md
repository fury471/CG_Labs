# Roadmap and Quality Gates

## Purpose

This roadmap defines incremental work for the SfM Visualization Sandbox. Each milestone must leave the repository in a comprehensible, buildable state. It is intentionally conservative: correctness, architectural clarity and measured performance take priority over quickly accumulating features.

## Branch strategy

Long-lived integration branch:

```text
feature/sfm-visualization-sandbox
```

Suggested short-lived milestone branches, created from the integration branch:

```text
feature/sfm-m1-app-shell
feature/sfm-m2-gpu-raii
feature/sfm-m3-shader-material
feature/sfm-m4-render-queue
feature/sfm-m5-instancing
feature/sfm-m6-render-targets-profiling
feature/sfm-m7-point-cloud
feature/sfm-m8-camera-poses
feature/sfm-m9-mesh-projection
feature/sfm-m10-product-hardening
```

A milestone is merged into the integration branch only after its acceptance criteria are met locally and its changes are documented.

---

## Milestone 0 — Baseline and documentation

### Goal

Establish the starting point before introducing new source code.

### Delivered or confirmed

- `feature/sfm-visualization-sandbox` exists from the `vs2026` branch baseline.
- The user established a working Ninja/MSVC development terminal and confirmed the current build/run baseline.
- The sandbox charter, target architecture, roadmap, engineering standards and initial architecture-decision records are recorded under `docs/sandbox/`.

### Still desirable to archive

- a screenshot of `EDAN35_Assignment2` including its Render Time panel;
- actual observed CPU/GPU timing values at the baseline scene/camera position;
- local GPU and driver identity used for measurements.

### Exit gate

Documentation exists and no production source has been modified before the architecture is recorded.

---

## Milestone 1 — Application shell and target boundaries

### Goal

Create a new runnable application target named `SfmSandbox`, with new library boundaries, without modifying the behaviour of existing EDAF80/EDAN35 applications.

### Planned implementation

- Add `src/sandbox/core/`, `src/sandbox/gfx/`, `src/sandbox/scene/` and `src/sandbox/viz/` CMake target structure only as required by compiled code.
- Add `src/apps/SfmSandbox/` executable target.
- Set C++20 as a target-local requirement for new sandbox targets; do not globally change legacy assignment requirements.
- Reuse the existing platform/window facilities only where doing so reduces initial cost without making new reusable layers depend on assignment-specific rendering.
- Display a valid OpenGL window, clear colour and minimal ImGui status panel.
- Establish an application-side startup/shutdown flow that will later own the new renderer.

### Learning objectives

- CMake target composition and transitive dependencies;
- the difference between platform setup and renderer architecture;
- preserving regression applications while adding a new product executable.

### Acceptance criteria

- Clean Ninja configure/build succeeds in the VS 2026 x64 development environment.
- Existing `EDAF80_Assignment1` launches.
- Existing `EDAN35_Assignment2` launches.
- New `SfmSandbox` launches, opens a window and presents a frame.
- New targets are visibly separated from `src/core/` and assignment sources.
- A short milestone completion note records build/run commands and known limitations.

---

## Milestone 2 — Move-only RAII GPU resource layer

### Goal

Introduce safe ownership for all new GPU resources.

### Planned implementation

- `gfx::Buffer`
- `gfx::VertexArray`
- `gfx::Texture2D`
- `gfx::Sampler`
- `gfx::ShaderProgram` foundation
- `gfx::Framebuffer` foundation

### Required properties

- non-copyable and move-enabled;
- safe default/empty state;
- deterministic resource destruction;
- OpenGL debug labelling in development builds;
- no manual deletion of owned GPU resources in application code.

### Validation

- construct, move and destroy resources during controlled application execution;
- run under OpenGL debug callback with no ownership-induced API errors;
- add CPU-side tests where behaviour can be tested independently of an OpenGL context.

---

## Milestone 3 — Shader interface and material separation

### Goal

Create shader/material facilities that remove repeated string-based binding work from object drawing.

### Planned implementation

- shader source loading and diagnostic reporting;
- linked program ownership;
- cached shader uniform/binding locations;
- minimal `scene::Material` or equivalent immutable material description;
- development-only shader reload strategy.

### Acceptance criteria

- a simple mesh renders through the new path;
- render loop does not call `glGetUniformLocation` repeatedly per submitted object;
- shader compilation errors identify the failed source and preserve a usable debugging workflow.

---

## Milestone 4 — Scene submissions and render queue

### Goal

Centralize rendering decisions.

### Planned implementation

- `Transform`, `Mesh`, `Material`, `Renderable` separation;
- renderer submission interface;
- render queue sorting by pipeline/material/mesh identifiers;
- draw-call/statistics panel.

### Acceptance criteria

- renderables contain data but do not issue raw OpenGL draw calls themselves;
- sorting can be demonstrated and its number of state changes observed;
- baseline and new executable continue to launch.

---

## Milestone 5 — Instancing and workload stress scene

### Goal

Support efficient repeated geometry and verify performance improvement.

### Planned implementation

- instanced transform buffers;
- instanced debug geometry appropriate to future camera-frustum/marker display;
- controlled stress scene.

### Acceptance criteria

- before/after draw-call counts are reported;
- before/after CPU and GPU timing results are archived;
- large repeated-object scene does not rely on one draw per object.

---

## Milestone 6 — Resize-safe render targets, HDR and profiling

### Goal

Provide a robust screen-dependent renderer foundation.

### Planned implementation

- render-target recreation on framebuffer resize;
- HDR colour render target;
- basic tone-mapping pass;
- reusable GPU profiler/timing scopes;
- debug views and frame statistics.

### Acceptance criteria

- resizing the window does not break rendering or produce incomplete framebuffers;
- profiling presents named pass times;
- render target formats and memory/performance consequences are documented.

---

## Milestone 7 — Point-cloud visualization

### Goal

Deliver the first direct reconstruction-viewer capability.

### Planned implementation

- initial `.ply` coloured point-cloud import path;
- large point-buffer GPU upload;
- point rendering with configurable display modes and point size;
- bounding box and dataset statistics;
- practical test datasets with documented redistribution status.

### Acceptance criteria

- load and interactively view a real point cloud;
- report point count, draw count and frame timing;
- failures such as unsupported or malformed input are handled visibly.

---

## Milestone 8 — Camera poses and trajectories

### Goal

Make SfM camera solutions visually inspectable.

### Planned implementation

- camera-pose data model;
- frustum visualization using efficient repeated geometry;
- trajectory polyline display;
- selectable pose/camera metadata panel;
- coordinate-convention documentation.

### Acceptance criteria

- known synthetic camera sequence displays with verified orientation/order;
- convention transforms between CV and graphics coordinates are tested and documented.

---

## Milestone 9 — Reconstructed mesh and imagery relationships

### Goal

Inspect reconstruction quality using surfaces and source imagery.

### Planned implementation

- reconstructed mesh loading;
- PBR-oriented material path only where justified by visible inspection value;
- image planes/overlays;
- projective texture or association visualization.

### Acceptance criteria

- a dataset can be inspected through points, poses and mesh representation;
- projection behaviour is checked against known camera calibration data.

---

## Milestone 10 — Product hardening

### Goal

Move from strong prototype to a distributable focused product foundation.

### Planned implementation

- continuous integration builds;
- automated tests and rendering regression strategy;
- asset/dataset validation;
- packaging and configuration handling;
- documentation for users and developers;
- dependency/resource/license audit;
- measured performance target scenes.

---

## Permanent quality gates

These gates apply throughout the roadmap.

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
SfmSandbox       (after Milestone 1)
```

### Architecture gate

- New product code goes into `src/sandbox/` or `src/apps/SfmSandbox/`.
- No SfM feature is embedded into legacy assignment implementation files.
- Higher layers do not issue raw OpenGL calls outside explicit backend/debug exceptions.

### Ownership gate

- New GPU ownership uses RAII owners once the relevant wrapper exists.
- Ownership-bearing types are move-only unless an explicit shared-lifetime design is documented.

### Performance gate

- No optimization claim without a reproducible workload and measurement.
- Profiling measurements record configuration, scene/data scale and relevant machine/GPU context.

### Documentation gate

- Architectural policy changes receive an ADR or update to an existing ADR.
- Completed milestones receive a completion note and known limitations.

### Commercial-readiness gate

- Do not distribute inherited course assets or claim commercial readiness without license review.
