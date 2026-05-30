# Engineering Standards

## 1. Purpose

This document defines working rules for new code in the SfM Visualization Sandbox. The rules are intended to produce software that is easy to learn from, safe to change, measurable in performance and suitable for gradual development into a focused, powerful and extensible visualization tool.

The project goal is not a throwaway prototype. Prototype-like samples are allowed only when they reduce risk on the path toward a robust tool. Milestone implementations must avoid shortcuts that block later data formats, renderer architecture, validation, profiling or product-quality UX.

These standards apply to new code under:

```text
src/sandbox/
src/apps/SfmSandbox/
shaders/sandbox/
tests/
docs/sandbox/
```

Legacy EDAF80, EDAN35 and Bonobo code is preserved as reference/regression code unless a separately documented compatibility fix is required.

## 2. Language and build standards

### 2.1 New-code standard

New sandbox targets use C++20 through target-scoped CMake compile requirements:

```cmake
target_compile_features(<target> PUBLIC cxx_std_20)
```

Do not globally force legacy assignment targets to C++20 merely to satisfy new code.

### 2.2 Build-system policy

- CMake targets express dependencies; include directories and compile features propagate through target relationships.
- Ninja is the primary local generator for the VS 2026/MSVC workflow.
- A clean configure/build must remain possible from a Visual Studio x64 development terminal.
- Source lists are explicit in CMake; do not depend on automatic globbing for production sources.
- Dependencies are added only when they clearly remove more cost/risk than they introduce.

### 2.3 Compiler quality settings

Milestone 1 should establish warning settings for new sandbox targets without immediately imposing them on legacy lab code. The intended standard is high warning visibility and zero new warnings in new code. Warnings-as-errors may be enabled for sandbox targets once the first target compiles cleanly and the compiler matrix is known.

## 3. C++ design rules

### 3.1 Ownership is explicit

- Prefer values and scoped ownership.
- GPU resources use RAII owner types when introduced.
- Ownership-bearing types are non-copyable unless sharing is a deliberate, documented semantic.
- Moving a resource leaves the moved-from object valid and empty.
- Raw pointers and non-owning references must not imply ownership.

### 3.2 Interface design

- Keep public interfaces small and intention-revealing.
- Use `[[nodiscard]]` on operations whose failure or returned resource must not be ignored.
- Use `std::span` for non-owning contiguous data views rather than pointer/count pairs in new code.
- Use `std::string_view` for transient read-only name/path fragments where lifetime is unambiguous.
- Use `std::filesystem::path` for filesystem paths.
- Use `enum class` instead of unscoped integer flag conventions where practical.
- Avoid inheritance-heavy engine hierarchies; prefer composition and clear data associations.

### 3.3 Error policy

- Recoverable failures return an explicit result/error representation suitable to the milestone; they do not silently construct invalid render data.
- Programmer-contract violations are asserted in development builds and described clearly.
- Error messages identify the operation and relevant object/file/shader path.
- Do not catch exceptions merely to discard diagnostic information.

### 3.4 Scope-control policy

Do not introduce complex generalization before a real feature requires it. Examples:

- do not add an entity-component system before scene needs justify it;
- do not add task systems/multithreading before profiling identifies CPU bottlenecks;
- do not add Vulkan/Direct3D abstraction layers while the product is validating its viewer workflow;
- do not add custom allocation strategies without measured allocation pressure.

## 4. Module responsibility rules

### `sandbox/core`

May contain general facilities such as errors, timers, filesystem utilities and logging interfaces. It does not know OpenGL, mesh semantics, point clouds or UI.

### `sandbox/gfx`

Owns graphics-backend concepts: GPU resources, shader programs, render targets, submission and profiling. It may use OpenGL/GLFW integration as required; higher layers do not bypass it for normal rendering.

### `sandbox/scene`

Owns spatial and renderable scene data: transforms, cameras, materials, meshes/references and frustum concepts. It describes content but does not call raw OpenGL APIs.

### `sandbox/viz`

Owns reconstruction-viewer semantics: point clouds, pose sets, trajectories, source-image relationships and reconstruction inspection tools. It consumes scene/gfx services and does not initialize the platform window.

### `apps/SfmSandbox`

Owns application composition, runtime mode selection and development UI panels. Reusable rendering, data and import logic must migrate down to the appropriate library layer.

## 5. Graphics API and renderer standards

### 5.1 OpenGL baseline

The primary development environment targets OpenGL 4.6 Core on Windows with MSVC/VS 2026. Use stable OpenGL 4.5+ functionality, including Direct State Access, inside the new `gfx` layer where it simplifies ownership and avoids binding-oriented mutation.

Any intentional requirement that excludes older contexts must be documented when introduced. The legacy applications may continue to retain their existing cross-platform assumptions.

### 5.2 GPU resource handling

- Every newly owned GPU object receives an RAII owner as the resource layer is introduced.
- Debug builds label GPU objects with meaningful names.
- Creation failures and invalid framebuffer states are reported immediately.
- Object destruction happens exactly once through its owner.

### 5.3 Render submission

- Scene/renderable objects supply submission data; they do not draw themselves.
- The renderer controls program, material, texture and vertex-state binding.
- Shader uniform/binding locations are cached after linkage/reflection, not looked up per object per frame.
- Redundant state changes should be minimized through render ordering and state caching when measurable workloads exist.

### 5.4 Large-data visualization

- Point clouds are represented as bulk GPU data, not a `Node` per point.
- Repeated markers/frustums use instancing once the relevant milestone is reached.
- Trajectories use efficient line/segment representations suitable for many poses.
- Data upload frequency must reflect mutability: immutable reconstruction data is not re-uploaded each frame.

## 6. Shader standards

- Sandbox shaders live under `shaders/sandbox/`.
- Shader inputs, uniforms and resource bindings are named consistently and matched by cached program interfaces.
- Shader compilation/link diagnostics must be visible in development runs.
- Coordinate spaces must be stated for nontrivial values, for example `world_normal`, `view_position` or `clip_position`.
- Colour-space/HDR/tone-mapping assumptions must be documented when those features are introduced.
- Shader hot reload is a development aid, not a substitute for deterministic startup validation.

## 7. Performance and profiling standards

### 7.1 Measurement before optimization

Every significant performance claim must record:

- executable and build configuration;
- renderer feature settings;
- dataset or synthetic workload size;
- GPU/model/driver context where available;
- CPU frame time;
- GPU pass time where applicable;
- draw-call count and relevant primitive/point counts;
- before/after values for an optimization.

### 7.2 Development statistics

Once the new renderer exists, its debug UI should gradually expose:

```text
CPU frame time
GPU frame/pass time
Draw calls
Submitted/visible item count
Point count or triangle count
Framebuffer extent
```

### 7.3 Benchmark discipline

A performance result is not meaningful if camera position, dataset, build type or enabled debug features changed without being noted.

## 8. Testing and validation standards

### 8.1 Regression applications

Until superseded by automated checks, the following are manual regression references:

```text
EDAF80_Assignment1
EDAN35_Assignment2
SfmSandbox (after Milestone 1)
```

### 8.2 Automated tests

Add CPU-side tests early for deterministic components that do not require a live GL context, including eventually:

- transform/camera mathematics;
- frustum tests;
- file parsing and malformed-input errors;
- coordinate convention conversion;
- render-queue ordering keys.

GPU/render regression tests are introduced after the render path and representative datasets stabilize.

### 8.3 Definition of done for a milestone

A milestone is done only when:

- it builds cleanly in the intended environment;
- required regression programs run;
- acceptance criteria in `ROADMAP.md` are checked;
- new design decisions are documented;
- known limitations are listed honestly.

## 9. Git and review standards

- The integration branch is `feature/sfm-visualization-sandbox`.
- Develop cohesive milestones in short-lived branches where practical.
- Commit messages identify intent, for example `gfx: add move-only buffer owner` rather than `update code`.
- Avoid commits mixing unrelated formatting, refactoring and behavioural changes.
- Never commit generated `build/` output or downloaded dependency build products unless a deliberate packaging decision is recorded.

### 9.1 AI-assisted intermediate commit policy

AI-assisted online edits may create several small commits before a milestone is locally validated. To avoid consuming GitHub Actions minutes or flooding status checks during that exploratory phase, every AI-created intermediate implementation or documentation commit must include one GitHub Actions skip marker in the commit message, preferably:

```text
[skip ci]
```

Example:

```text
scene: add OBJ mesh loader [skip ci]
```

The final pull request is still opened normally after local validation. Human-authored milestone commits may omit the marker when CI should run immediately.

## 10. Commercial and legal standards

The project may be designed for eventual commercial extension, but it is not considered distributable as a commercial product until:

- original framework licensing is verified;
- third-party dependency licenses are recorded and complied with;
- downloaded course resources and test datasets have verified redistribution rights;
- packaging excludes or replaces any non-redistributable assets;
- user-facing behaviour and failure handling have been validated.

## 11. References

- CMake compile features: <https://cmake.org/cmake/help/latest/manual/cmake-compile-features.7.html>
- C++ Core Guidelines, including RAII and resource management: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
- OpenGL 4.6 Core Profile Specification: <https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf>
- Khronos OpenGL Direct State Access overview: <https://wikis.khronos.org/opengl/Direct_State_Access>
