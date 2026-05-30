# Roadmap and Quality Gates

## Purpose

This roadmap defines the current direction for the SfM Visualization Sandbox after the completion of Milestone 10. It records the validated foundation history and schedules the remaining work in dependency order.

The goal is not to make a disposable prototype. The goal is to build a powerful, best-practice and extensible SfM visualization tool. Early milestones are foundation-building milestones toward that tool.

The project remains intentionally conservative: each integration step must leave the repository understandable, buildable and locally runnable. Correctness, architectural clarity, regression safety, measured performance and honest documentation take priority over quickly accumulating features.

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

Milestones 0-10 are complete and form the first validated foundation phase. Future milestones now continue from Milestone 11, but only because this roadmap explicitly schedules them.

Do not introduce or rename milestone numbers by conversation alone. Future milestone changes must be made in this document first.

---

## Completed foundation phase

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

**Status:** Complete as shader-program foundation; material separation scheduled for Milestone 15.

Delivered:

- shader compilation/link diagnostics;
- linked program ownership;
- cached uniform lookup path;
- typed uniform setting used by renderer paths;
- shader-program probe visible in the sandbox status panel.

### Milestone 4 — Central renderer and first rendered geometry

**Status:** Complete as central renderer foundation; generic render queue scheduled for Milestone 15.

Delivered:

- `Renderer` as the central sandbox graphics boundary;
- first GPU geometry path;
- initial draw submission through renderer-owned GPU resources.

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

**Status:** Complete as milestone text format loader; full reconstruction point-cloud import scheduled for Milestone 12.

Delivered:

- text point-cloud loader for `xyz` and `xyzrgb` rows;
- checked-in sample point-cloud resource;
- loader diagnostics and malformed-line accounting;
- portable classic-locale float parsing after CI exposed `std::from_chars(float)` portability issues.

### Milestone 8 — Runtime point-cloud reload

**Status:** Complete.

Delivered:

- runtime editable point-cloud path field;
- `Load / Reload` and `Reset to sample` controls;
- transactional point-cloud GPU reload;
- failed reload preserves the previous visible cloud;
- immutable-buffer policy preserved by replacing point buffers/VAOs instead of reusing immutable storage.

### Milestone 9 — Camera pose / frustum visualization

**Status:** Complete as camera visualization foundation; instancing and pose metadata scheduled later.

Delivered:

- CPU-side `CameraPose` and `CameraPoseSet` model;
- deterministic camera orbit;
- camera frustum line rendering;
- trajectory line rendering;
- pose/frustum counts in the status panel.

### Milestone 10 — Camera pose loading from file

**Status:** Complete as milestone text format loader; common reconstruction-format imports scheduled for Milestone 14.

Delivered:

- camera-pose text loader for `eye target colour` rows;
- checked-in sample camera-pose resource;
- file-backed camera frustums and trajectory;
- skipped-line diagnostics;
- fallback to deterministic orbit if loading fails.

---

## Current capability map

The completed foundation now supports:

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

This is a strong tool foundation, not a finished reconstruction viewer.

---

## Scheduled roadmap after Milestone 10

The remaining work is ordered by dependency and risk. The schedule intentionally does not jump straight to mesh or imagery features before file-format validation, test coverage, renderer architecture and performance measurement are strong enough to support them.

Recommended cadence:

- one milestone per short-lived branch;
- one focused pull request per milestone;
- local validation plus CI before merge;
- no milestone may claim performance improvement without a reproducible measurement scene;
- no external dataset may be committed without redistribution/license review.

### Milestone 11 — Test and validation foundation

**Purpose:** Make the existing file/data layer trustworthy before adding more formats.

**Planned scope:**

- add test target infrastructure for new sandbox code;
- unit tests for point-cloud text parsing;
- unit tests for camera-pose text parsing;
- tests for malformed input, comments, commas, whitespace, normalized RGB and byte-style RGB;
- tests for degenerate camera pose rejection;
- document parser guarantees and known exclusions.

**Acceptance criteria:**

- tests run in local Ninja build;
- tests run in CI where practical;
- parser behavior is documented and reproducible;
- legacy applications and `SfmSandbox` still launch.

### Milestone 12 — PLY point-cloud import and dataset statistics

**Purpose:** Move from milestone text point clouds toward real reconstruction data.

**Planned scope:**

- initial ASCII PLY coloured point-cloud loader;
- documented rejection of unsupported PLY properties or binary PLY if not implemented;
- point-cloud bounding box calculation;
- dataset statistics panel: point count, bounds, approximate memory size;
- sample dataset policy: use tiny synthetic repo-safe file first, document redistribution rules for real datasets.

**Acceptance criteria:**

- valid ASCII PLY sample loads and renders;
- malformed/unsupported PLY fails visibly without crashing;
- bounding box and dataset statistics are visible;
- existing text point-cloud loading/reload remains functional.

### Milestone 13 — Point-cloud inspection controls and UX polish

**Purpose:** Make point-cloud inspection more useful before expanding geometry types.

**Planned scope:**

- configurable point size in UI;
- basic point colour/display modes where data supports them;
- optional bounding box visualization;
- native or framework-supported file picker if feasible without destabilizing platform code;
- clearer loader diagnostics layout;
- active dataset summary panel.

**Acceptance criteria:**

- point size/display changes are visible at runtime;
- invalid file selection preserves previous visible data;
- dataset statistics remain accurate after reload;
- interaction and existing probes remain stable.

### Milestone 14 — Camera-pose conventions, import formats and metadata

**Purpose:** Make camera poses usable with real SfM outputs and remove ambiguity around coordinate systems.

**Planned scope:**

- coordinate-convention documentation for graphics camera space versus common computer-vision conventions;
- tests for pose transform conventions;
- runtime camera-pose reload with failed-reload preservation;
- at least one common camera-pose import path, selected after checking target dataset format;
- pose metadata display: index, position, source file, skipped-line count;
- optional pose selection if it can be implemented without disrupting renderer architecture.

**Acceptance criteria:**

- known synthetic camera sequence displays with verified orientation and order;
- camera-pose reload does not destroy previous visible poses on failure;
- transform convention tests pass;
- point-cloud reload remains functional.

### Milestone 15 — Renderer architecture consolidation

**Purpose:** Pay down architectural debt before adding mesh and image visualization.

**Planned scope:**

- shader source loading from files;
- improved shader diagnostics with file names;
- minimal material abstraction;
- mesh abstraction;
- renderable/submission model;
- render queue or equivalent explicit submission structure;
- draw-call and state-change statistics panel;
- keep raw OpenGL calls inside `sandbox/gfx` except narrow debug exceptions.

**Acceptance criteria:**

- grid, point cloud and camera frustums render through the consolidated submission path or a clearly documented transitional path;
- renderables do not issue raw OpenGL draw calls themselves;
- draw-call/state statistics are visible;
- no behavior regression in existing sandbox features.

### Milestone 16 — Reconstructed mesh loading and basic surface inspection

**Purpose:** Add the first reconstructed-surface inspection capability.

**Planned scope:**

- simple mesh loader for a selected, documented format;
- CPU-side mesh data model;
- GPU mesh upload path through the renderer architecture from Milestone 15;
- basic material suitable for reconstruction inspection;
- mesh visibility toggle with point cloud and camera poses still available;
- mesh bounds/statistics.

**Acceptance criteria:**

- sample mesh loads and renders;
- mesh, point cloud and camera frustums can be inspected together;
- malformed mesh input fails visibly;
- renderer statistics account for mesh draw calls.

### Milestone 17 — Imagery relationships and projection debugging

**Purpose:** Connect points, cameras and source imagery so reconstruction quality can be inspected visually.

**Planned scope:**

- image plane or image overlay representation;
- camera-image association data model;
- simple image resource loading with license-safe sample asset;
- optional projective texture or projection-debug visualization;
- projection sanity check against known camera parameters if calibration data is available.

**Acceptance criteria:**

- at least one camera can display or reference an associated image;
- image/frustum/point relationship is visually understandable;
- projection behavior is documented and checked against a known synthetic case.

### Milestone 18 — Render targets, profiling and performance baseline

**Purpose:** Establish measurable rendering infrastructure before optimization claims.

**Planned scope:**

- resize-safe offscreen render targets;
- framebuffer completeness diagnostics;
- optional HDR colour target and tone-mapping pass if justified by inspection value;
- GPU timing scopes and named pass timings;
- CPU frame statistics and draw-call statistics consolidated in the status panel;
- archived baseline measurements for representative scenes.

**Acceptance criteria:**

- resizing does not break render targets;
- named pass timings are visible;
- baseline scenes and measurement configuration are documented;
- no performance claim is made without measurement evidence.

### Milestone 19 — Instancing and workload stress scenes

**Purpose:** Optimize repeated visualization objects only after profiling infrastructure exists.

**Planned scope:**

- instanced rendering for repeated frustums, markers or debug glyphs;
- controlled stress scene with many camera poses/markers;
- before/after draw-call counts;
- before/after CPU/GPU timing captures;
- documented limitations and hardware context.

**Acceptance criteria:**

- repeated-object scene avoids one draw call per object where instancing applies;
- timing results demonstrate or honestly refute the expected improvement;
- visual output matches non-instanced reference behavior.

### Milestone 20 — Product hardening, packaging and release readiness

**Purpose:** Move from a strong foundation to a maintainable, distributable focused tool.

**Planned scope:**

- automated tests included in CI;
- rendering regression strategy or screenshot-baseline plan;
- asset and dataset validation workflow;
- packaging/configuration handling;
- end-user documentation;
- developer documentation;
- dependency, resource and license audit;
- performance target scenes and repeatable measurement procedure;
- cleanup of milestone-era debug wording where it should become product wording.

**Acceptance criteria:**

- CI covers build and available tests;
- documented run instructions are correct on the supported local workflow;
- sample resources have clear license/redistribution status;
- performance baselines are reproducible;
- the project does not claim commercial readiness beyond what license review supports.

---

## Coverage check against original deferred backlog

All deferred original roadmap items are scheduled:

| Backlog item | Scheduled milestone |
|---|---|
| parser tests and data validation | M11 |
| PLY point-cloud import | M12 |
| bounding box and point-cloud statistics | M12/M13 |
| configurable point display modes | M13 |
| file picker / improved reload UX | M13 |
| coordinate-convention tests and documentation | M14 |
| runtime camera-pose reload | M14 |
| common camera-pose import formats | M14 |
| pose selection and metadata | M14 |
| shader file loading and material abstraction | M15 |
| mesh/renderable model and render queue | M15 |
| draw-call/state statistics | M15/M18 |
| reconstructed mesh loading | M16 |
| image planes/overlays and camera-image relationships | M17 |
| projection-debug visualization | M17 |
| resize-safe render targets | M18 |
| HDR/tone mapping, if justified | M18 |
| GPU timing scopes and profiling | M18 |
| instanced repeated-object rendering | M19 |
| controlled stress scene | M19 |
| before/after timing evidence | M19 |
| automated tests in CI | M11/M20 |
| rendering regression strategy | M20 |
| packaging/configuration handling | M20 |
| user/developer documentation | M20 |
| dependency/resource/license audit | M20 |
| performance target scenes | M18/M20 |

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
- Optimization work must have a before/after measurement plan before implementation begins.

### Documentation gate

- Architectural policy changes receive an ADR or update to an existing ADR when they change design direction.
- Completed milestones receive a completion note and known limitations.
- Roadmap changes must distinguish completed work from deferred or planned work.
- Future milestone changes must update this roadmap before implementation starts.

### Commercial-readiness gate

- Do not distribute inherited course assets or claim commercial readiness without license review.
- External datasets, images, meshes and reconstruction files need documented redistribution status before being committed.
