# M19 Instancing and Workload Stress Scene

Milestone 19 evaluates repeated-object rendering after the M18 profiling baseline exists.

The goal is not to claim universal performance improvement. The goal is to compare a controlled repeated-marker workload with and without instancing, using the same visual output and the same scene.

## Stress scene

M19 adds a synthetic repeated-marker workload to `SfmSandbox`.

Each marker is a small three-axis line glyph. The marker positions are generated deterministically in a spiral/ring distribution around the existing sample reconstruction scene.

Default settings:

```text
marker count: 400
radius: 4.0
height: 1.2
marker scale: 0.06
active path: instanced
```

## Rendering paths

The same marker workload can be drawn in two modes:

```text
reference non-instanced path: one draw call per marker
instanced path: one instanced draw call for all markers
```

The UI reports:

```text
stress marker count
active marker draw calls
reference non-instanced draw calls
instanced draw calls
marker vertices drawn
draw-call reduction
active path
```

## Validation procedure

1. Launch `SfmSandbox`.
2. Confirm the M19 section is visible.
3. Confirm the marker renderer is ready.
4. Confirm markers are visible around the reconstruction scene.
5. Toggle `Use instanced marker path` on and off.
6. Confirm the visual output remains equivalent.
7. Confirm active draw calls switch between:

```text
instanced: 1 draw call
reference: marker_count draw calls
```

8. Change `Stress marker count` and rebuild/observe the new counts.
9. Record the named CPU pass timings from the M18 profiler section.
10. Record M25 GPU pass timings when timer queries are available, especially
    the marker stress scene pass.

## Interpretation rule

M19 may report draw-call reduction directly because it is counted by the renderer. Runtime speedup must be described carefully:

```text
If local timings improve: say the improvement was observed in this local configuration.
If timings do not improve: document that instancing reduced draw calls but did not produce a clear CPU-frame improvement on this workload/hardware.
```

Do not claim a general performance improvement without repeated measurements under the same scene, camera, build type and framebuffer size.

## Known limitations

- GPU timer queries are available only when the runtime OpenGL context reports
  `GL_TIME_ELAPSED` support.
- The stress scene uses synthetic debug markers, not real SfM feature tracks.
- The non-instanced reference path uses repeated draw calls over the same instanced vertex layout for comparison; it is a controlled baseline, not an optimized non-instanced renderer.
- Timing results depend on hardware, driver, build type, framebuffer size and UI visibility.
