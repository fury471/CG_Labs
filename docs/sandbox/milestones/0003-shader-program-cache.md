# Milestone 3 Record: ShaderProgram Interface and Cached Bindings

- **Milestone:** 3
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m3-shader-cache`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Introduce a usable shader-program interface for the SfM Visualization Sandbox. The interface must provide compile/link diagnostics, retain move-only GPU ownership, cache uniform locations, and avoid repeated uniform-name lookups in later rendering code.

## Implemented source boundaries

```text
src/sandbox/gfx/
  ShaderProgram.hpp/.cpp        Compile/link interface, move-only program owner, cached uniforms
  ShaderProgramProbe.hpp/.cpp   Runtime validation probe for shader build and uniform cache behavior
```

Existing Milestone 2 files remain in place. `SfmSandbox` now displays both the Milestone 3 shader probe and the Milestone 2 ownership probe as regression evidence.

## ShaderProgram responsibilities

`ShaderProgram` now supports:

- compile/link construction from `std::span<ShaderSource const>`;
- structured `ShaderProgramBuildResult` containing the program, success flag, and diagnostic log;
- explicit shader-stage enum values instead of arbitrary stage `GLenum` input from high-level code;
- temporary shader cleanup after link by detaching and deleting shader objects;
- move-only ownership of the OpenGL program name;
- `bind()` for draw-time program binding;
- cached `UniformLocation` lookup;
- typed uniform setters for `int`, `float`, `glm::vec2`, `glm::vec3`, `glm::vec4`, and `glm::mat4`;
- `glProgramUniform*`-based uniform writes that do not require rebinding the program.

## Uniform caching policy

Uniform lookup follows this rule:

- first lookup for a uniform name calls `glGetUniformLocation`;
- the result is cached in the program object;
- `-1` is cached for missing or optimized-away uniforms;
- repeated lookup of the same name returns the cached value;
- uniform setters ignore invalid `UniformLocation` values.

This is the intended path for later rendering code: resolve locations during setup and keep `UniformLocation` handles for repeated updates rather than doing string lookup every frame.

## Move semantics and cache behavior

`ShaderProgram` keeps the Milestone 2 move-only ownership model. The OpenGL program name transfers on move; uniform-location caches are cleared rather than moved.

Reason: the cache is rebuildable, while the move operation is part of GPU ownership transfer and should remain genuinely `noexcept`. Moving an associative container could hide allocation or allocator-dependent behavior, so the safer design is to drop cached metadata and lazily rebuild it when the program is used again.

## Runtime validation probe

`SfmSandbox` now calls `sfm::gfx::run_shader_program_probe()` after creating the OpenGL context.

The probe checks:

- vertex and fragment shader compilation;
- program linking;
- successful lookup of active uniforms;
- cached lookup of a missing uniform;
- repeated lookup does not grow the cache;
- typed uniform setters execute through cached locations;
- clearing the cache leaves the program with zero cached entries.

The result appears in the `Sandbox status` panel under:

```text
Milestone 3: ShaderProgram interface and cached bindings
Shader probe: passed/failed
```

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| Shader compile/link diagnostics are explicit | `ShaderProgramBuildResult` returns success flag and log |
| Temporary shader objects are cleaned after link | Implemented with detach/delete helper |
| Uniform locations are cached | Implemented with `std::unordered_map<std::string, UniformLocation>` |
| Missing uniforms are cached too | `-1` locations are stored and reused |
| Uniform setters avoid binding dependency | Implemented with `glProgramUniform*` helpers |
| Move-only GPU ownership remains intact | Program name moves; caches are cleared |
| Legacy assignments remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation required

From the Visual Studio 2026 x64 Developer Command Prompt:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m3-shader-cache
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
| `SfmSandbox` | Panel shows `Milestone 3: ShaderProgram interface and cached bindings` | Pending |
| `SfmSandbox` | Panel shows `Shader probe: passed` | Pending |
| `SfmSandbox` | Shader probe messages report compile/link and cache checks as passed | Pending |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Pending |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc`, and resize behaviour still work | Pending |

## Known limitations at this checkpoint

- The shader interface supports source strings only; file-based shader loading is deferred.
- The probe validates shader/program interface behavior but does not draw geometry.
- Program binary caching, reflection of all active uniforms, shader hot reload, and automatic include/preprocessor systems are deliberately deferred.
- `SfmSandbox` still does not render scene geometry, point clouds, camera poses, trajectories or reconstructed meshes.

## Completion gate

Milestone 3 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
