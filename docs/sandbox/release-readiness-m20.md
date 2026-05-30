# M20 Release Readiness Guide

Milestone 20 hardens the SfM Visualization Sandbox as a maintainable focused tool.

This document is not a commercial-release approval. It records the supported local workflow, regression checks, resource status and known release limitations.

## Supported local workflow

The supported development workflow is Windows + MSVC developer environment + Ninja.

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-visualization-sandbox
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Launch checks:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe
build\src\EDAF80\EDAF80_Assignment1.exe
build\src\EDAN35\EDAN35_Assignment2.exe
```

## CI workflow

M20 adds a GitHub Actions workflow:

```text
.github/workflows/sandbox-ci.yml
```

It configures, builds and runs CTest on Windows using Ninja. Pull requests into `feature/sfm-visualization-sandbox` should use this workflow as the automated gate.

AI-created intermediate commits must keep using `[skip ci]`; final PR validation should not skip CI.

## Regression strategy

Automated checks:

```text
cmake configure
cmake build
ctest
```

Manual launch checks:

```text
SfmSandbox
EDAF80_Assignment1
EDAN35_Assignment2
```

Manual `SfmSandbox` visual checks:

```text
point cloud visible
point-cloud bounds visible
surface mesh visible
camera frustums and trajectory visible
associated image plane visible
marker stress scene visible
M18 render-target status complete
M18/M19 performance counters visible
reload paths keep previous valid data on failure
```

Screenshot-baseline strategy for now:

```text
Capture one screenshot of the default SfmSandbox startup scene after every major rendering change.
Record branch/commit, framebuffer size and enabled debug layers/controls.
Compare visually for missing major scene components before merging.
```

A later milestone can automate screenshot comparison after the rendering path stabilizes.

## Resource and license status

Project-owned synthetic sandbox samples:

```text
res/sandbox/sample_point_cloud_ascii.ply
res/sandbox/sample_point_cloud.xyzrgb
res/sandbox/sample_camera_poses.txt
res/sandbox/sample_surface.obj
res/sandbox/sample_camera_image.ppm
```

These are small synthetic development samples created for the sandbox milestones. They may be redistributed with the repository.

Inherited course/framework resources remain subject to the original course repository and dependency licenses. Do not claim commercial redistribution readiness until those licenses are reviewed.

## Dependency status

Pinned dependency versions are configured in the root `CMakeLists.txt`.

Key dependencies include:

```text
Assimp
GLFW
GLM
ImGui
GLAD
tinyfiledialogs
stb
```

Before any public/commercial release, record the exact license for every dependency and confirm redistribution obligations.

## Packaging status

The root CMake install step currently installs:

```text
shaders
res
```

This is enough for a development install layout, but not a polished end-user package.

Known packaging limitations:

```text
no installer
no versioned release archive
no runtime dependency audit artifact
no end-user configuration file
no crash/error reporting UX
```

## Performance baseline status

Use `docs/sandbox/performance-baseline-m18.md` and `docs/sandbox/instancing-stress-m19.md` for measurement procedure.

Performance claims must include:

```text
build type
framebuffer size
scene settings
marker count if M19 is enabled
hardware/GPU context if known
before/after values
commit or PR reference
```

## Known release limitations

- The sandbox is a focused development tool, not a commercial product.
- Image support is currently PPM-based and debug-oriented.
- Surface support is a tiny OBJ subset, not a general mesh importer.
- Marker stress scenes are synthetic.
- GPU timer queries are not implemented yet.
- Screenshot regression is manual.
- Licensing of inherited course assets and all dependencies must be reviewed before distribution claims.

## M20 completion gate

M20 is complete when:

```text
CI config exists
local run instructions are documented
resource/license status is documented
regression strategy is documented
performance baseline procedure is documented
known release limitations are documented
SfmSandbox and legacy assignment launch checks still pass
```
