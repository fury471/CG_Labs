# SfM Visualization Sandbox

## Project charter

This development line evolves `CG_Labs` into a small, rigorous real-time **3D reconstruction and Structure-from-Motion (SfM) visualization sandbox**. It is intended to be both a learning project in modern real-time graphics and a credible foundation for a focused visualization product.

The work is developed on:

```text
feature/sfm-visualization-sandbox
```

The branch starts from the modernized `vs2026` course codebase. The original EDAF80 and EDAN35 executables remain reference implementations and regression applications; new product-oriented code will be introduced alongside them rather than by turning assignment files into an engine.

## Product statement

The intended application is an interactive viewer for reconstruction results:

- coloured point clouds;
- reconstructed triangle meshes;
- calibrated camera poses and camera frustums;
- frame-to-frame trajectories;
- RGB/depth imagery and image overlays;
- texture projection and reconstruction inspection views.

This is deliberately narrower and more useful than building a generic game engine. It connects computer graphics, camera geometry and computer vision in a form relevant to reconstruction, inspection and visualization work.

## Current baseline

The inherited repository currently provides:

- a CMake-based C++/OpenGL build;
- a small Bonobo framework in `src/core/`;
- EDAF80 assignments covering scene hierarchy, parametric geometry, shading, environment/water effects and an interactive sphere application;
- an EDAN35 deferred renderer using Sponza, G-buffers, spotlight shadow maps, light accumulation and a resolve pass;
- ImGui runtime tools, shader reloading and OpenGL/GPU debugging facilities.

The branch was created from `vs2026` at commit:

```text
e6f51ab0dd3e8019104b1c7d9a76170d6ec44c6a
```

Baseline validation status, recorded on 2026-05-29:

| Check | Status |
|---|---|
| Ninja/MSVC build environment established locally | User confirmed complete |
| Existing executables located and launched | User confirmed complete |
| `EDAF80_Assignment1` used as introductory regression application | Established |
| `EDAN35_Assignment2` used as advanced renderer regression application | Established |
| Detailed baseline timing values/screenshots archived | Pending capture |

## Non-negotiable engineering goals

1. **Robustness.** GPU resources have explicit ownership; invalid states fail visibly; errors contain actionable context.
2. **Efficiency.** Rendering design enables state reduction, batching, instancing and profiling; performance claims are measured.
3. **High cohesion and low coupling.** Backend graphics objects, scene data, SfM visualization features and application coordination are separate layers.
4. **High learning value.** Each milestone must explain which rendering or software-design concept is being learned and provide observable validation.
5. **Commercial extensibility.** The architecture should support a focused product, while avoiding premature claims that it is a general-purpose engine.
6. **Low cost.** Use stable open-source dependencies already present where suitable; avoid unnecessary infrastructure and premature complexity.

## What will not be done

- Existing assignments will not be rewritten wholesale.
- New product features will not be placed inside large assignment `run()` functions.
- `Node::render()` will not become the central rendering architecture for the new application.
- Multithreading, custom allocators, C++ modules or a new graphics API will not be introduced without a measured or architectural need.
- Commercial distribution will not be claimed until licensing for the inherited framework, course resources and dependencies has been audited.

## Documentation map

| Document | Purpose |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Current limitations, target layer model, core types and rendering data flow |
| [ROADMAP.md](ROADMAP.md) | Milestones, acceptance criteria and validation gates |
| [ENGINEERING_STANDARDS.md](ENGINEERING_STANDARDS.md) | C++/OpenGL/CMake/testing/performance/Git working rules |
| [adr/0001-build-alongside-bonobo.md](adr/0001-build-alongside-bonobo.md) | Decision to preserve labs and build a new sandbox beside them |
| [adr/0002-cpp20-opengl46-resource-and-renderer-policy.md](adr/0002-cpp20-opengl46-resource-and-renderer-policy.md) | Initial technology and ownership policy |

## Success definition

A successful outcome is a maintainable application that can load and inspect reconstruction data interactively, remains performant on large point clouds and many camera poses, exposes measurable rendering performance, and demonstrates clean modern C++ ownership and renderer architecture.

A successful outcome is **not** merely a window that displays a point cloud: the architecture, validation and documentation are part of the product.

## Primary technical references

- CMake compile features and standard requirements: <https://cmake.org/cmake/help/latest/manual/cmake-compile-features.7.html>
- C++ Core Guidelines: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
- OpenGL 4.6 Core Profile Specification: <https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf>
- Khronos OpenGL Direct State Access overview: <https://wikis.khronos.org/opengl/Direct_State_Access>
