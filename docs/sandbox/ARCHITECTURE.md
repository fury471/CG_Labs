# Architecture: SfM Visualization Sandbox

## 1. Context

`CG_Labs` is currently a teaching repository: it provides the Bonobo helper framework, five EDAF80 programs and an EDAN35 deferred-rendering program. That code is valuable as an executable reference and rendering-learning asset. It is not the intended product architecture for the SfM sandbox.

The target system is a real-time 3D reconstruction viewer that can evolve without coupling computer-vision visualization features directly to OpenGL calls or assignment code.

## 2. Current constraints and motivations

The inherited code reveals several patterns that should not be scaled into new product code:

- `Node` combines geometry, material constants, textures, transform and shader association, and its render path executes immediate OpenGL submission for an object.
- The render path performs shader-program and texture bindings as part of object rendering, and queries uniforms by name during drawing.
- Assignment applications own substantial setup, rendering, UI and feature logic inside application-sized source files.
- The EDAN35 renderer contains valuable multi-pass concepts and GPU timing practices, but its pass construction is application-specific rather than reusable.
- Resource handles are predominantly raw OpenGL identifiers; robust ownership in new code needs to be explicit and automatic.

The new sandbox will coexist with this reference code rather than forcing a risky, broad refactor.

## 3. Architectural objectives

The target architecture shall:

1. Separate GPU backend details from scene meaning and SfM-specific visualization.
2. Use explicit resource ownership and deterministic cleanup.
3. Make the fast rendering path centrally controlled and measurable.
4. Permit incremental implementation with a runnable executable after each milestone.
5. Support high-volume visualization, especially point clouds and camera trajectories.
6. Preserve an educational mapping from architectural types to graphics concepts.

## 4. Layered dependency rule

Dependencies flow downward only:

```text
src/apps/SfmSandbox
        |
        v
src/sandbox/viz       Reconstruction/SfM visualization features
        |
        v
src/sandbox/scene     Spatial meaning and renderable scene data
        |
        v
src/sandbox/gfx       GPU resource and renderer implementation
        |
        v
src/sandbox/core      Error, logging, timing, file and common utilities
        |
        v
OpenGL / GLFW / GLM / Assimp / stb / ImGui
```

### Dependency prohibitions

- `core` does not include OpenGL types or visualization concepts.
- `gfx` does not depend on point clouds, trajectories, SfM or application UI.
- `scene` does not perform raw OpenGL API calls.
- `viz` expresses reconstruction data and builds render submissions using lower layers; it does not own window/platform initialization.
- `apps/SfmSandbox` composes systems and UI but does not become a dumping ground for reusable implementations.

## 5. Planned source tree

```text
src/
  sandbox/
    core/
      Error.hpp
      Log.hpp
      Timer.hpp
      FileSystem.hpp
    gfx/
      Buffer.hpp/.cpp
      VertexArray.hpp/.cpp
      Texture2D.hpp/.cpp
      Sampler.hpp/.cpp
      ShaderProgram.hpp/.cpp
      Framebuffer.hpp/.cpp
      RenderTarget.hpp/.cpp
      RenderCommand.hpp
      RenderQueue.hpp/.cpp
      Renderer.hpp/.cpp
      GpuProfiler.hpp/.cpp
    scene/
      Transform.hpp
      Camera.hpp/.cpp
      Mesh.hpp
      Material.hpp
      Renderable.hpp
      Scene.hpp/.cpp
      Frustum.hpp/.cpp
    viz/
      PointCloud.hpp/.cpp
      PointCloudRenderer.hpp/.cpp
      CameraPose.hpp
      CameraFrustumRenderer.hpp/.cpp
      TrajectoryRenderer.hpp/.cpp
      ImageOverlay.hpp/.cpp
      ReconstructedMesh.hpp/.cpp
      ProjectionOverlay.hpp/.cpp
  apps/
    SfmSandbox/
      main.cpp
```

This is an intended structure, not a requirement to add empty files prematurely. Components are introduced only when their milestone needs them.

## 6. Core domain model

### 6.1 Graphics resources

`gfx` owns GPU-facing objects. These objects use move-only RAII ownership:

```text
Buffer       owns one GPU buffer object
VertexArray  owns vertex input state
Texture2D    owns one texture and describes its format/extent
Sampler      owns sampling state
ShaderProgram owns a linked program and cached binding interface
Framebuffer  owns attachment aggregation/validation
RenderTarget owns resizable screen-dependent framebuffer resources
```

A valid GPU owner is responsible for releasing its own resource. New code must not distribute manual `glDelete*` calls among callers.

### 6.2 Scene types

`scene` states what is visible and where it exists:

```text
Transform   local/world spatial transformation
Mesh        reusable geometry reference
Material    appearance/pipeline inputs
Renderable  Mesh + Material + Transform association
Camera      view/projection state
Scene       collection/lifetime and queries for submitted content
Frustum     visibility-test volume
```

A `Renderable` is data to submit; it does not draw itself.

### 6.3 Visualization types

`viz` introduces semantics specific to reconstruction:

```text
PointCloud          points plus optional colour/metadata
CameraPose          calibrated camera transformation/intrinsics association
Trajectory          ordered pose path
ReconstructedMesh   reconstructed surface representation
ImageOverlay        source-frame inspection data
ProjectionOverlay   projective association between imagery and geometry
```

## 7. Rendering data flow

The application loop will eventually follow this form:

```text
Platform/input update
        |
Scene and visualization update
        |
Camera/frustum update
        |
Collect visible render submissions
        |
Build RenderQueue
        |
Sort/group submissions by pipeline, material and mesh
        |
Execute Renderer passes
        |
Collect CPU/GPU profiling statistics
        |
Draw development UI
        |
Present frame
```

This differs deliberately from self-rendering scene nodes. Central submission is needed for render sorting, instancing, culling and useful instrumentation.

## 8. Render path evolution

### Initial forward path

Milestone 1 begins with a minimal executable and clear background/debug UI. Subsequent early rendering starts with a simple forward path because it is easy to validate and sufficient for initial geometry.

### High-volume point path

Point-cloud data will receive a purpose-built path. It must support large contiguous point buffers and avoid one object/draw call per point. Point size, colour modes and visibility statistics are part of the visualization contract.

### Optional advanced passes

Deferred rendering concepts from EDAN35 may later inform opaque reconstructed-mesh shading and debugging, but an inherited deferred renderer is not assumed to be the default product renderer. Render passes shall be justified by viewer requirements and measurements.

## 9. Performance architecture

The primary efficiency model is:

- upload static data once where practical;
- avoid per-draw string lookup and repeated redundant state work;
- sort render submissions;
- use instancing for repeated geometry such as camera frustums or markers;
- use contiguous GPU buffers for large point clouds and trajectories;
- use frustum culling where scene scale warrants it;
- profile before and after any claimed optimisation.

Required eventual metrics:

| Metric | Reason |
|---|---|
| CPU frame time | Detect application and submission bottlenecks |
| GPU frame time by pass | Detect shader/fill/bandwidth bottlenecks |
| Draw calls | Track batching/instancing effectiveness |
| Visible/submitted object counts | Explain workload |
| Point count / triangle count | Make comparisons reproducible |
| Render-target extent and key formats | Interpret memory/bandwidth changes |

## 10. Robustness architecture

New code shall treat failures explicitly:

- shader compile/link errors report paths and driver diagnostics;
- invalid framebuffer states are reported with context;
- failed asset loads produce error results instead of malformed renderables;
- resource wrappers remain valid when moved and safe when empty;
- window resize triggers dependent render-target recreation before use;
- development builds preserve OpenGL object labels and debug callbacks.

## 11. Extension boundaries

A later product may add file importers, SfM dataset adapters, render techniques or UI tools without violating layer direction. For example:

```text
New COLMAP importer -> viz/asset-facing code
New PLY point format -> asset/viz data loader
New point splat shader -> gfx pipeline + viz renderer integration
New analysis panel -> application UI consuming viz/renderer statistics
```

A later graphics backend replacement is not promised, but the architecture avoids unnecessarily exposing raw OpenGL operations above `gfx`.

## 12. Commercial-readiness limits

This architecture is designed to be commercially extensible for a focused visualization product. Distribution and production claims additionally require:

- complete license audit of inherited source, resources and dependencies;
- automated builds and tests;
- representative performance datasets and regression baselines;
- packaging and crash/error-handling policy;
- user-facing workflow validation.
