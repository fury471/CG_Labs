# Milestone 4 Record: Central Renderer and First Rendered Geometry

- **Milestone:** 4
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m4-central-renderer`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Introduce the first central renderer boundary and render visible geometry from the new sandbox graphics layer. The application should continue to own frame lifecycle and UI, while the renderer owns GPU pipeline setup and draw submission.

## Implemented source boundaries

```text
src/sandbox/gfx/
  Renderer.hpp/.cpp             Central renderer boundary and first triangle draw path
  Buffer.hpp/.cpp               Adds immutable static buffer storage upload helper
  VertexArray.hpp/.cpp          Adds DSA vertex-buffer and attribute-layout helpers
```

`SfmSandbox` now creates a renderer after OpenGL context creation and calls `renderer.render()` after the frame clear pass.

## Renderer responsibilities

`Renderer` currently owns:

- one `ShaderProgram` for the first triangle pipeline;
- one `Buffer` containing immutable static vertex data;
- one `VertexArray` describing the position and colour vertex attributes;
- one `render()` method that submits the draw call.

The rendered primitive is a simple RGB triangle in clip space. This is intentionally minimal: the purpose of this milestone is to establish the draw boundary, not to introduce a full scene graph, camera transform system, material system, or reconstruction data model.

## Buffer and VertexArray API added

`Buffer` now supports:

- `set_storage(std::span<std::byte const>, GLbitfield)`;
- templated `set_storage(std::span<T const>, GLbitfield)` for typed static data upload;
- immutable storage through `glNamedBufferStorage`.

`VertexArray` now supports:

- DSA vertex-buffer binding through `glVertexArrayVertexBuffer`;
- DSA floating-point attribute configuration through `glEnableVertexArrayAttrib`, `glVertexArrayAttribFormat`, and `glVertexArrayAttribBinding`.

These helpers are deliberately narrow. They support the first renderer path without exposing dynamic streaming, index buffers, instancing, integer attributes, or high-level mesh loading before those requirements exist.

## Runtime status panel

The `Sandbox status` panel now includes:

```text
Milestone 4: Central renderer and first geometry
Renderer: ready/failed
```

It also keeps the previous probes visible:

```text
Milestone 3 regression: ShaderProgram cache
Milestone 2 regression: GPU RAII ownership
```

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| Application no longer owns first draw-call setup | `Renderer` owns shader, buffer, vertex array and draw submission |
| First visible geometry is rendered through sandbox/gfx | Implemented as an RGB triangle in `Renderer` |
| Buffer upload remains narrow and explicit | Immutable static storage only |
| Vertex-array setup uses DSA | Implemented without binding the VAO during configuration |
| Shader and ownership probes remain visible | `SfmSandbox` still displays Milestone 3 and Milestone 2 probes |
| Legacy assignments remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` running with the coloured triangle visible;
- `Sandbox status` panel visible;
- `Renderer: ready` visible;
- renderer messages report shader build, immutable vertex-buffer upload, DSA vertex-array layout and central renderer readiness;
- Milestone 3 shader probe remains passed in the same run;
- Milestone 2 ownership probe remains passed in the same run;
- developer confirmation that remaining legacy regression and sandbox control checks also passed.

Reference build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m4-central-renderer
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Manual launch checklist:

| Executable | Required observation | Result |
|---|---|---|
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Passed — developer confirmed |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Passed — developer confirmed |
| `SfmSandbox` | Window opens and the status panel is visible | Passed — screenshot confirmed |
| `SfmSandbox` | A coloured triangle is visible in the viewport | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Milestone 4: Central renderer and first geometry` | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Renderer: ready` | Passed — screenshot confirmed |
| `SfmSandbox` | Renderer messages report shader build, vertex-buffer upload and VAO layout success | Passed — screenshot confirmed |
| `SfmSandbox` | Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc`, and resize behaviour still work | Passed — developer confirmed |

## Known limitations at completion

- The renderer draws one hard-coded triangle only.
- No scene graph, camera transform, mesh abstraction, material system, index buffer, or instancing path exists yet.
- The renderer uses source-string shaders; file-based shader loading remains deferred.
- `SfmSandbox` still does not render point clouds, camera poses, trajectories, reconstructed meshes or SfM data.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Clean Ninja configure/build succeeds in the VS 2026 x64 development environment | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` renders first visible geometry | Passed — screenshot confirmed |
| Central renderer owns first draw-call setup | Passed — source implementation and screenshot |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 4 is complete and may be merged into `feature/sfm-visualization-sandbox`.
