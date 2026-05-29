# Milestone 4 Record: Central Renderer and First Rendered Geometry

- **Milestone:** 4
- **Status:** Implementation ready for local validation
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

## Local validation required

From the Visual Studio 2026 x64 Developer Command Prompt:

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
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Pending |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Pending |
| `SfmSandbox` | Window opens and the status panel is visible | Pending |
| `SfmSandbox` | A coloured triangle is visible in the viewport | Pending |
| `SfmSandbox` | Panel shows `Milestone 4: Central renderer and first geometry` | Pending |
| `SfmSandbox` | Panel shows `Renderer: ready` | Pending |
| `SfmSandbox` | Renderer messages report shader build, vertex-buffer upload and VAO layout success | Pending |
| `SfmSandbox` | Milestone 3 shader probe still passes | Pending |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Pending |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc`, and resize behaviour still work | Pending |

## Known limitations at this checkpoint

- The renderer draws one hard-coded triangle only.
- No scene graph, camera transform, mesh abstraction, material system, index buffer, or instancing path exists yet.
- The renderer uses source-string shaders; file-based shader loading remains deferred.
- `SfmSandbox` still does not render point clouds, camera poses, trajectories, reconstructed meshes or SfM data.

## Completion gate

Milestone 4 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
