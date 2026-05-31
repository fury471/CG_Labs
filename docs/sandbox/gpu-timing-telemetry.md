# GPU Timing Telemetry

Milestone 25 adds GPU pass timing to the existing CPU frame profiler. The goal
is honest measurement, not universal performance claims.

## Implementation Boundary

GPU timing lives in `sandbox/gfx`:

```text
GpuTimerQuery     move-only OpenGL query owner
GpuFrameProfiler  named pass collection and delayed result polling
```

The application composes scopes around the active render passes:

```text
clear
grid, points and cameras
surface mesh
image plane
marker stress scene
```

The UI reports CPU timings and GPU timings separately. GPU query results are
polled after later frames instead of forcing an immediate synchronous readback.

## Availability Rules

The profiler checks `GL_TIME_ELAPSED` counter bits after an OpenGL context
exists. If timer queries are unavailable, the UI reports that state and does not
invent timings.

Unsupported or delayed cases are expected to show diagnostics such as:

```text
GPU timer queries are unavailable
Waiting for GPU timing frame(s) to complete
Dropped stale GPU timing frame because query results did not become available quickly enough
```

## Measurement Rules

When using GPU timings for M19 or later optimization analysis, record:

```text
build mode
framebuffer size
camera/viewpoint
project manifest
point count
surface triangle count
image visibility
marker count
instancing mode
CPU pass timings
GPU pass timings
GPU/context if known
```

Do not compare timings across different scenes, cameras, build types,
framebuffer sizes or UI/debug settings unless those differences are explicitly
called out.
