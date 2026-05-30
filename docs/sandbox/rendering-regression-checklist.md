# Rendering Regression Checklist

Use this checklist before merging rendering or data-visualization changes into `feature/sfm-visualization-sandbox`.

## Build and test gate

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Required launch checks

```text
SfmSandbox
EDAF80_Assignment1
EDAN35_Assignment2
```

## Default SfmSandbox visual baseline

After launching `SfmSandbox`, verify the default scene shows:

```text
point cloud
point-cloud bounds
surface mesh
camera frustums
camera trajectory
associated image plane
M19 marker stress scene
M18 render-target status
M18/M19 draw statistics
```

## Required UI checks

```text
point size slider changes point size
point colour mode changes point appearance
surface visibility toggle works
surface colour update works
surface reload failure keeps previous valid surface
associated image visibility toggle works
associated camera selector moves the image plane
image reload failure keeps previous valid image
marker instancing toggle changes active draw-call count
marker count slider updates reference/instanced draw-call comparison after rebuild
window resize keeps render target complete
```

## Screenshot baseline procedure

For any PR that changes rendering output:

1. Capture a screenshot of the default startup view.
2. Record:

```text
branch
commit or PR number
build type
framebuffer size
marker count
instancing on/off
point display settings
surface/image visibility
```

3. Compare against the previous milestone screenshot for missing major components.

Current strategy is manual visual regression. Automated screenshot comparison is deferred until the active rendering path and UI layout stabilize further.

## Failure policy

If a regression is found:

```text
preserve previous valid data on reload failure
show a visible diagnostic in the UI
avoid silent fallback that hides the problem
record the known limitation before merging
```
