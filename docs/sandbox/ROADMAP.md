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

## Scheduled productization roadmap after Milestone 20

Milestones 0-20 completed the original foundation roadmap. The sandbox is now a
useful development viewer, but its own release-readiness notes still identify
prototype-era limits: hard-coded startup samples, header-only milestone import
code, debug-oriented image display, manual screenshot regression, no GPU timer
queries, no packaged project/session format and incomplete license audit.

The following milestones turn the sandbox into a more robust, extensible tool.
They must be scheduled here before implementation and completed with the same
build/test/documentation gates as earlier milestones.

### Milestone 21 - Project manifest and importer module cleanup

**Status:** Complete.

**Purpose:** Replace hard-coded startup assumptions and header-only milestone
leftovers with a registered scene-library import/config boundary.

**Planned scope:**

- add a small sandbox project manifest format for startup dataset paths;
- load the default manifest from `res/sandbox/default_project.sfmproj`;
- allow command-line `--project <path>` startup override;
- keep manifest paths relative to the manifest file unless absolute;
- split surface and image import implementations out of header-only code;
- register import implementation files in `sfm_sandbox_scene`;
- expand CPU tests for manifest parsing, relative path resolution, surface
  validation and image validation;
- document the manifest format and current limitations.

**Acceptance criteria:**

- `SfmSandbox` starts from the manifest-defined point cloud, camera poses,
  surface and image;
- invalid project files fail visibly and keep the documented sample fallback;
- scene importers are compiled as normal library implementation files;
- CTest covers manifest and importer failure cases;
- root README and sandbox docs describe the project manifest workflow.

### Milestone 22 - Application state and UI composition cleanup

**Status:** Complete.

**Purpose:** Move from a monolithic application loop toward maintainable
application services without changing renderer behavior.

**Planned scope:**

- introduce small app-side state structs for point clouds, poses, surfaces,
  images, render/profiling settings and stress scenes;
- group reload transactions into focused helpers;
- split status-panel drawing into named functions or app-local components;
- keep reusable parsing/rendering logic below the app layer;
- preserve all existing UI controls and failure-preservation behavior.

**Acceptance criteria:**

- `src/apps/SfmSandbox/main.cpp` no longer owns large unrelated blocks of data
  and reload logic inline;
- every reload path still preserves previous valid data on failure;
- CTest and launch checks pass;
- documentation records the new app composition boundary.

### Milestone 23 - Textured image planes and calibrated image groundwork

**Status:** Complete.

**Purpose:** Replace the debug corner-colour image card with a real texture path
and prepare for calibrated image/reprojection workflows.

**Planned scope:**

- add texture storage/upload policy to `Texture2D`;
- add sampler configuration used by the image plane renderer;
- add image-plane shaders with UV attributes;
- render the PPM sample as a textured quad rather than interpolated corner
  colours;
- preserve CPU image importer tests and transactional reload behavior;
- document current colour-space and calibration limitations.

**Acceptance criteria:**

- the associated image plane displays image content through an OpenGL texture;
- invalid image reloads keep the previous image plane;
- renderer statistics continue to account for image-plane draws;
- CTest and launch checks pass.

### Milestone 24 - Automated visual regression capture path

**Status:** Complete.

**Purpose:** Reduce reliance on purely manual screenshot checks.

**Planned scope:**

- add a deterministic startup scene metadata report;
- add `--capture-baseline <output.ppm>` to render the startup scene once,
  capture the back buffer as a binary PPM and exit;
- archive visual regression metadata next to captures;
- keep manual screenshot comparison as the fallback when platform/window
  support prevents capture automation.

**Acceptance criteria:**

- documented command can capture the default sandbox visual baseline;
- capture metadata includes build mode, framebuffer size, enabled layers,
  dataset paths and scene counts;
- failure to capture reports a clear reason without crashing.

### Milestone 25 - GPU timing scopes and renderer pass telemetry

**Status:** Complete.

**Purpose:** Complete the profiling foundation promised by earlier milestones.

**Planned scope:**

- add move-only RAII query/timer wrappers in `sandbox/gfx`;
- measure named GPU scopes for active renderer paths where supported;
- report unavailable timer-query support honestly;
- separate CPU and GPU timing displays;
- document measurement caveats and repeatability rules.

**Acceptance criteria:**

- GPU pass timings are visible when timer queries are available;
- unsupported timer-query paths show a diagnostic instead of fake timing;
- M19 optimization interpretation can use real GPU data where available.

### Milestone 26 - Release candidate packaging and license closure

**Status:** Complete for development-release candidacy.

**Purpose:** Decide what the sandbox can honestly claim as a distributable
focused tool.

**Planned scope:**

- complete the project-owned resource inventory and record dependency license
  evidence from the local dependency sources;
- add `--validate-install` for a no-window startup/resource/shader check;
- add CTest coverage for startup validation;
- verify installed shader/resource lookup for `SfmSandbox`;
- document release notes, known limitations and redistribution boundaries;
- remove stale milestone wording from product-facing docs where appropriate.

**Acceptance criteria:**

- install/package smoke check launches or reports an actionable local blocker;
- license status is explicit for dependencies, inherited assets and sandbox
  samples;
- the project claims only the readiness level supported by evidence.

---

## Scheduled reconstruction-workbench roadmap after Milestone 26

Milestones 0-26 made the sandbox a robust focused viewer and development
release candidate. The next product direction is broader: it should become a
reconstruction workbench that can derive, inspect and export geometry from point
data rather than only display assets produced elsewhere.

The workbench direction must stay honest. A complete point-cloud-to-mesh
product needs normals, outlier filtering, scale-aware reconstruction settings,
quality diagnostics and multiple algorithms. The first milestones therefore add
constrained, testable builder capabilities before claiming general 3D
reconstruction.

### Milestone 27 - Point-cloud-to-mesh builder foundation

**Status:** Complete.

**Purpose:** Add the first conversion path from a loaded point cloud to a
surface mesh, with clear algorithm limits and export support.

**Planned scope:**

- add a CPU-side point-to-mesh builder module below the app layer;
- triangulate suitable surface-like point clouds by projecting to a selected
  plane, running deterministic 2D triangulation and lifting triangles back to
  3D;
- reject degenerate input, duplicate projected samples and invalid triangles
  explicitly;
- expose build settings for projection plane, weld tolerance, minimum triangle
  area, maximum edge length and maximum projected points;
- allow building the active viewer surface from the active point cloud;
- keep the interactive app responsive by refusing oversized builds and running
  accepted builds asynchronously;
- add a no-window `--build-mesh <output.obj>` command for point-cloud to OBJ
  conversion from the selected project manifest;
- add OBJ export for generated meshes;
- document algorithm constraints, suitable data and unsuitable data.

**Acceptance criteria:**

- a synthetic point cloud can be converted to a valid `SurfaceMesh`;
- generated mesh rendering preserves previous visible surface on build/upload
  failure;
- oversized interactive builds report a clear diagnostic instead of freezing the
  UI;
- `SfmSandbox --build-mesh output.obj` writes a valid OBJ or exits with clear
  diagnostics;
- CTest covers successful triangulation, degenerate input and export/import
  round-trip plus the no-window CLI builder;
- root README and sandbox docs describe the builder workflow without claiming
  general-purpose reconstruction.

### Milestone 28 - Reconstruction quality controls and diagnostics

**Status:** Complete.

**Purpose:** Make generated meshes inspectable and debuggable instead of opaque
black-box output.

**Planned scope:**

- report unique point count, duplicate count, rejected triangle count and output
  triangle count;
- visualize rejected/long-edge thresholds where practical;
- add mesh/source provenance metadata to captures and exports;
- add quality warnings for sparse, collinear, duplicate-heavy or unsuitable
  point distributions;
- keep generated and imported surface states clearly distinguishable.

**Acceptance criteria:**

- build diagnostics are visible in UI and CLI output;
- exported OBJ includes generation comments;
- invalid or poor-quality input explains why output is empty or limited;
- visual baseline metadata records whether the surface is imported or generated.

**Delivered:**

- builder diagnostics include input points, unique projected points, welded
  duplicates, invalid skipped points, rejected triangles and output mesh size;
- quality warnings flag sparse projected evidence, heavy welding and triangle
  filtering;
- mesh-builder UI shows the resolved projection and all core build counters;
- generated OBJ files include multi-line provenance comments for source point
  cloud, algorithm, projection and build counters;
- visual capture metadata records surface kind, source format, vertex/triangle
  counts and generated-mesh build counters when applicable.

### Milestone 29 - Real reconstruction algorithm integration decision

**Status:** Complete.

**Purpose:** Decide the next reconstruction algorithm based on real data needs
instead of hand-rolling a fragile general solver.

**Planned scope:**

- evaluate dependency-backed options such as screened Poisson, ball pivoting,
  alpha shapes or CGAL/Open3D-style workflows;
- define required input metadata: normals, confidence, scale, cameras or dense
  samples;
- decide whether integration happens in-process, as an optional tool adapter or
  through a command-line pipeline;
- document license, packaging and performance implications before committing to
  a dependency.

**Acceptance criteria:**

- decision record exists before adding a heavy reconstruction dependency;
- selected algorithm has test data and failure cases;
- licensing and packaging implications are recorded;
- M27 remains the fallback deterministic builder for small surface-like data.

**Delivered:**

- ADR 0003 selects an optional Open3D-backed adapter as the first real
  reconstruction path, with Screened Poisson first and Ball Pivoting / Alpha
  Shapes as secondary modes;
- CGAL and PCL remain comparison/future options because they need additional
  license, packaging and parameter-policy work;
- generated synthetic test-data requirements and failure cases are recorded
  before implementation;
- M27 remains the built-in deterministic fallback.

---

## Scheduled production-hardening track after Milestone 27

M27 proved the workbench direction, but it also exposed a product risk: the
application layer can become messy if each new workflow adds parsing,
validation, UI, rendering and export code to one large class. The next hardening
track keeps feature work extendable before the reconstruction algorithms become
more complex.

### Milestone 30 - Application command and runtime decomposition

**Status:** Complete.

**Purpose:** Split app-local workflows by responsibility so startup commands,
no-window conversion and the live viewer can evolve independently.

**Planned scope:**

- move command-line parsing, startup validation and no-window mesh export out of
  `SfmSandboxApp`;
- keep `main.cpp` limited to platform/window/input/frame orchestration and
  runtime mode dispatch;
- keep `SfmSandboxApp` focused on interactive viewer state, reload
  transactions, renderer instances and UI composition;
- route command workflows through explicit result structs with diagnostics;
- preserve `--help`, `--validate-install`, `--capture-baseline` and
  `--build-mesh` behavior.

**Acceptance criteria:**

- app startup/no-window command code lives in an app-local startup module;
- live viewer code no longer owns command-line parsing or process-output
  formatting;
- build, CTest, `--build-mesh`, `--mesh-max-points` guard and `--help` pass;
- architecture and engineering standards document the new app boundary.

**Delivered:**

- `SfmSandboxStartup` owns command-line parsing, startup validation and
  no-window mesh export;
- `main.cpp` dispatches startup modes before entering the windowed frame loop;
- `SfmSandboxApp` keeps the interactive viewer state, renderer ownership and UI
  composition;
- validation covered build, CTest, CLI mesh export, max-point guard, help output
  and a one-frame baseline capture smoke.

### Milestone 31 - Mesh-builder workflow and panel extraction

**Status:** Complete.

**Purpose:** Continue shrinking the live app class by splitting the stable
mesh-builder UI and asynchronous workflow state without creating speculative
framework code.

**Planned scope:**

- extract mesh-builder workflow state from raw UI drawing;
- split the mesh-builder status-panel section into a focused app-local panel
  helper;
- keep panel helpers thin and data-driven, with scene/gfx work below the app
  layer;
- add focused tests for any extracted CPU-side workflow service.

**Acceptance criteria:**

- mesh builder UI no longer mixes long-running job orchestration, rendering
  upload and widget layout in one function;
- failed reload/build/export transactions still preserve prior visible state;
- no reusable CPU algorithm remains trapped in `apps/SfmSandbox`.

**Delivered:**

- `SfmSandboxMeshWorkflow` owns async build state, point-limit refusal, export
  bookkeeping, status messages and counters;
- `SfmSandboxMeshBuilderPanel` owns the ImGui controls and returns explicit
  build/export actions;
- `SfmSandboxApp` keeps renderer upload and prior-surface preservation at the
  live viewer boundary;
- `sfm_sandbox_mesh_workflow_tests` covers synchronous refusal, async success
  and export bookkeeping.

### Milestone 32 - Tooling, regression and release discipline hardening

**Status:** Complete.

**Purpose:** Raise the project from "works locally" toward repeatable product
engineering.

**Planned scope:**

- add command-level tests for startup parsing and no-window workflows where
  feasible without a GL context;
- document manual launch checks and automated checks per workflow;
- tighten release notes around experimental versus supported features;
- review CMake target source lists and dependencies for explicit target-scoped
  ownership.

**Acceptance criteria:**

- command workflows have deterministic tests or documented launch checks;
- CMake source ownership remains explicit and target-scoped;
- README and sandbox docs describe supported workflows without overclaiming
  reconstruction quality or release readiness.

**Delivered:**

- added app-local workflow test coverage for mesh-builder refusal, async success
  and export bookkeeping;
- kept startup validation and no-window mesh export under CTest;
- modernized local FetchContent dependency helpers away from deprecated direct
  `FetchContent_Populate` calls;
- verified build, CTest, CLI mesh export, max-point refusal and visual baseline
  capture after the tooling change.

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
