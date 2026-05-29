# ADR-0001: Build the SfM Sandbox Alongside Bonobo

- **Status:** Accepted
- **Date:** 2026-05-29
- **Decision owners:** Project developer and architecture review with ChatGPT

## Context

The repository begins as Lund University computer-graphics laboratory code. It already contains working assignment programs and a small support framework (`src/core/`, commonly referred to as Bonobo). The existing code is useful because it exposes graphics techniques directly and provides known runnable scenes:

- EDAF80 assignments for foundational scene/rendering exercises;
- EDAN35 Assignment 2 for deferred shading, shadow maps and pass timing.

The intended new direction is not a general game engine. It is a real-time viewer for Structure-from-Motion and 3D reconstruction results, including point clouds, camera poses, trajectories, reconstructed meshes and imagery relationships.

A choice is required between:

1. progressively changing Bonobo and assignment applications until they serve as the new product framework; or
2. preserving the existing laboratory/reference code while developing a new application and reusable libraries beside it.

## Decision

Adopt option 2.

New product-oriented code will be created under:

```text
src/sandbox/
src/apps/SfmSandbox/
shaders/sandbox/
tests/
docs/sandbox/
```

The existing Bonobo framework and assignment programs will be retained as reference implementations and manual regression programs. Their behaviour will not be altered merely to make the new architecture cleaner.

New product features, including point-cloud display and SfM visualization, shall not be added to assignment `run()` implementations.

## Reasons

### Preserve learning value

The existing assignments remain readable examples of specific graphics concepts. Rewriting them around a more abstract product renderer would obscure their educational role.

### Reduce regression risk

The original applications provide useful launch-and-view checks while the new renderer is built incrementally. Keeping them separate prevents architectural experiments from immediately destabilising the existing exercises.

### Avoid scaling known limitations

The existing immediate per-node rendering style is appropriate for assignments but is not the preferred foundation for centralized submission, batching, instancing, robust resource ownership and performance instrumentation.

### Establish a credible product boundary

A focused application architecture should contain only abstractions needed for the reconstruction viewer. It should not inherit every educational shortcut or assignment concern by default.

## Consequences

### Positive

- Existing assignments remain usable and comparable.
- New architecture can use stricter standards without first repairing all legacy code.
- Refactoring risk is controlled.
- Product features gain explicit, coherent module ownership.

### Negative

- Some initial duplication may occur, for example platform startup or camera concepts.
- Two architectural styles will coexist temporarily in the repository.
- Improvements made in the new sandbox do not automatically improve legacy assignments.

### Mitigations

- Document intentional coexistence clearly.
- Reuse third-party dependencies and compatible utilities where doing so does not introduce coupling.
- Treat any future shared extraction as a deliberate decision rather than incidental reuse.

## Alternatives rejected

### Convert `Node` into the new scene/rendering foundation

Rejected because the existing `Node` combines responsibilities and is based around self-submitting draw behaviour. Retrofitting it would risk breaking coursework examples while compromising the new render-queue design.

### Start an entirely separate repository immediately

Rejected for now because this repository already provides working OpenGL setup, dependencies, rendering reference code and a development context valuable for learning. A future repository split may be reconsidered if packaging, licensing or product ownership demands it.

## Validation

This decision is upheld when:

- new feature source files are placed in sandbox/application locations;
- the legacy EDAF80/EDAN35 programs continue to build and run through milestones;
- the new application can develop independently without embedding product logic in assignment files.
