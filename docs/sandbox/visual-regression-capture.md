# Visual Regression Capture

Milestone 24 adds a deterministic capture path for the default sandbox startup
scene. The path is intended for visual regression evidence, not for polished
image export.

## Command

From the repository root:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

The app opens the normal platform window, renders one startup frame, captures
the back buffer before the ImGui overlay is submitted, writes the image, writes
metadata next to it and exits.

The metadata path is the image path with `.txt` appended:

```text
build\sandbox-captures\sfm-baseline.ppm.txt
```

## Captured Evidence

The capture metadata records:

```text
build mode
framebuffer size
project manifest path
project load status
point-cloud path and point count
camera-pose path and pose count
surface path, visibility and triangle count
image path, visibility and size
marker visibility, marker count and instancing mode
point display settings
```

The image is written as binary PPM (`P6`) with rows flipped into top-left image
order. This keeps the writer dependency-free and easy to inspect with common
image tools.

## Failure Policy

Capture failures must report a clear reason and exit with failure. Expected
failure causes include:

```text
window/context creation failure
invalid framebuffer size
output directory creation failure
OpenGL readback failure
image or metadata write failure
```

Manual screenshot comparison remains the fallback on platforms where the window
cannot be created or framebuffer readback is not available.
