# Rendering Regression Checklist

Use this checklist before merging rendering or data-visualization changes into `feature/sfm-visualization-sandbox`.

## Build and test gate

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --validate-install
build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj
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
default project manifest loaded or sample fallback reported
point cloud
point-cloud bounds
surface mesh
camera frustums
camera trajectory
associated image plane
M19 marker stress scene
M18 render-target status
M18/M19 draw statistics
M25 GPU timing diagnostics or unavailable-timer message
```

## Required UI checks

```text
point size slider changes point size
point colour mode changes point appearance
camera pose visibility toggle works
surface visibility toggle works
surface colour update works
surface reload failure keeps previous valid surface
associated image visibility toggle works
associated camera selector moves the image plane
image reload failure keeps previous valid image
point-cloud mesh build failure keeps previous valid surface
large point-cloud mesh builds report the max-point guard instead of freezing UI
active surface OBJ export writes a reloadable OBJ
marker instancing toggle changes active draw-call count
marker count slider updates reference/instanced draw-call comparison after rebuild
window resize keeps render target complete
```

## Automated capture procedure

For any PR that changes rendering output:

1. Capture the default startup scene:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm
```

2. Keep the generated metadata beside the image:

```text
build\sandbox-captures\sfm-baseline.ppm.txt
```

3. Confirm the metadata records:

```text
branch
commit or PR number
build type
framebuffer size
project manifest
point count
camera pose count
camera pose visibility
surface visibility and triangle count
image visibility and size
marker count
instancing on/off
point display settings
```

4. Compare against the previous milestone capture or screenshot for missing
major components.

If the local platform cannot create a window/context for capture, use a manual
screenshot and record the failure reason from the capture command. Automated
pixel-threshold comparison is still deferred.

## Failure policy

If a regression is found:

```text
preserve previous valid data on reload failure
show a visible diagnostic in the UI
avoid silent fallback that hides the problem
record the known limitation before merging
```
