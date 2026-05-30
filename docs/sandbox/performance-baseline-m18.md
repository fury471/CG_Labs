# M18 Performance Baseline Procedure

Milestone 18 establishes measurement infrastructure before optimization claims.

No performance improvement is claimed in M18. The purpose is to make later comparisons repeatable.

## Baseline scene

Use the default `SfmSandbox` startup scene:

```text
point cloud: res/sandbox/sample_point_cloud_ascii.ply
surface mesh: res/sandbox/sample_surface.obj
camera poses: res/sandbox/sample_camera_poses.txt
associated image: res/sandbox/sample_camera_image.ppm
```

The default viewport should show:

```text
point cloud
point-cloud bounds
surface mesh
camera frustums and trajectory
associated image plane
```

## Required local workflow

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m18-profiling-baseline
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe
```

## Measurements to record

Record these values from the M18 panel:

```text
framebuffer size
offscreen target status
offscreen target size
rebuild count after resize
last completed CPU frame total
named CPU pass timings
draw calls
line vertices drawn
point vertices drawn
surface triangles drawn
image-plane triangles drawn
```

## Resize validation

1. Launch `SfmSandbox`.
2. Observe that the offscreen target reports complete.
3. Resize the window.
4. Confirm the offscreen target size follows the framebuffer size.
5. Confirm the rebuild count increases only when the framebuffer extent changes.
6. Confirm rendering still shows point cloud, surface, camera frustums and image plane.

## Hardware/context note

When reporting a baseline, include:

```text
CPU model if known
GPU model if known
operating system
build configuration
window/framebuffer size
Git commit or PR number
```

If hardware information is not available, say so explicitly instead of guessing.

## Interpretation rule

Do not describe a change as faster or slower unless the before/after scene, camera, build type and measurement procedure are the same.

M18 provides baseline instrumentation. M19 is the first milestone where optimization claims may be compared against this baseline.
