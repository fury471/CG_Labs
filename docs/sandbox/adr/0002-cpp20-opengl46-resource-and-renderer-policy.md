# ADR-0002: C++20, OpenGL 4.6 Primary Target, and Central Renderer Policy

- **Status:** Accepted for initial sandbox development
- **Date:** 2026-05-29
- **Decision owners:** Project developer and architecture review with ChatGPT

## Context

The inherited laboratory framework currently declares C++17 at shared configuration level and creates an OpenGL context through the existing window layer. On Windows/Linux, the current framework requests OpenGL 4.6 Core; macOS retains a lower OpenGL version path.

The new sandbox requires a technology policy that supports:

- clear ownership and safe cleanup of GPU resources;
- low-overhead rendering submission suitable for point clouds and many pose visualizations;
- a stable toolchain on the project's current Windows/Visual Studio environment;
- modern but understandable C++ code;
- incremental learning rather than speculative engine infrastructure.

## Decision

### Language version

New sandbox targets will use **C++20**, declared on those targets through CMake compile features. Legacy assignment targets will not be globally migrated as part of this decision.

### Graphics API target

The primary sandbox development target will be **OpenGL 4.6 Core on Windows**, matching the modern Windows context requested in the current framework. New `sandbox/gfx` implementation may use stable OpenGL 4.5+ capabilities such as Direct State Access where they improve object creation and reduce binding-oriented mutation.

### Renderer responsibility

New scene and visualization objects do not issue their own draw calls. They provide renderable/submission data. A central renderer controls render ordering, GPU state and draw execution.

### Resource ownership

New owned GPU resources will be represented by move-only RAII wrappers introduced incrementally in the graphics layer. Application and visualization code shall not become responsible for scattered `glDelete*` operations for such resources.

## Reasons

### Why C++20

C++20 supplies mature features useful to this project, including improved type/interface tools, `std::span`, modern constrained interfaces where useful, and a strong basis for ownership-oriented design. It is supported in the current modern MSVC environment without making the project dependent on newly emerging language/toolchain facilities.

### Why not require C++23/C++26 initially

The project benefits more from clear ownership and renderer design than from using the newest possible language standard. A stable baseline lowers build friction and improves reproducibility while the architecture is established.

### Why OpenGL 4.6 as primary target

The current Windows development direction already runs through an OpenGL 4.6 context, and OpenGL provides direct access to the graphics concepts the project is intended to learn and demonstrate. It is cost-efficient for a focused desktop visualization prototype.

### Why Direct State Access internally

Direct State Access enables graphics objects to be initialized and modified through object-specific API calls rather than repeated global binding changes. Used within the graphics layer, it improves implementation clarity and reduces state-coupling hazards. This does not imply that high-level scene or visualization code depends directly on OpenGL calls.

### Why central submission

Central rendering is necessary for later render sorting, instancing, culling, draw-call statistics and GPU profiling. A point-cloud/SfM viewer must handle many points and potentially many repeated pose/frustum visualizations efficiently.

## Consequences

### Positive

- New code can demonstrate modern C++ ownership and interfaces cleanly.
- GPU object lifetime becomes easier to reason about.
- The render path is positioned for profiling and optimization.
- Modern OpenGL features can reduce internal backend complexity.

### Negative

- New sandbox targets may not run on hardware lacking the selected OpenGL capabilities unless fallback work is explicitly added later.
- Some compatibility considerations differ from the legacy coursework applications.
- A central renderer requires more initial structure than a direct draw-from-object prototype.

## Deferred decisions

This ADR does not yet decide:

- whether a PBR renderer is required;
- whether a deferred renderer will be used for reconstructed meshes;
- whether macOS compatibility is required for the new sandbox;
- whether an alternate graphics API will ever be supported;
- whether an ECS architecture is useful;
- whether asynchronous asset loading is needed.

Each of those requires actual viewer requirements or measured evidence.

## Validation

This decision is followed when:

- Milestone 1 sets C++20 only for new sandbox targets;
- Milestone 2 implements move-only GPU owners;
- Milestone 3 removes repeated per-object uniform-location lookup from the new rendering path;
- Milestone 4 routes normal scene rendering through a renderer/render queue;
- later performance work records measurable evidence.

## References

- CMake compile feature requirements: <https://cmake.org/cmake/help/latest/manual/cmake-compile-features.7.html>
- C++ Core Guidelines: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
- OpenGL 4.6 Core Specification: <https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf>
- Khronos Direct State Access overview: <https://wikis.khronos.org/opengl/Direct_State_Access>
