# Sandbox Resource and License Inventory

This inventory covers the SfM Visualization Sandbox resources introduced under `res/sandbox`, `shaders/sandbox`, `src/sandbox` and `src/apps/SfmSandbox`.

It does not certify the full inherited course repository for commercial redistribution.

## Project-owned sandbox samples

| Path | Type | Origin | Redistribution status |
|---|---|---|---|
| `res/sandbox/sample_point_cloud_ascii.ply` | ASCII PLY point cloud | Synthetic sandbox sample | May be redistributed with this repository |
| `res/sandbox/sample_point_cloud.xyzrgb` | Text XYZRGB point cloud | Synthetic sandbox sample | May be redistributed with this repository |
| `res/sandbox/sample_camera_poses.txt` | Text camera poses | Synthetic sandbox sample | May be redistributed with this repository |
| `res/sandbox/sample_surface.obj` | Tiny OBJ surface | Synthetic sandbox sample | May be redistributed with this repository |
| `res/sandbox/sample_camera_image.ppm` | ASCII PPM image | Synthetic sandbox sample | May be redistributed with this repository |

## Project-owned sandbox code and shaders

| Path | Type | Origin | Redistribution status |
|---|---|---|---|
| `src/sandbox/**` | C++ sandbox libraries | Project code | Subject to repository license/inherited course context |
| `src/apps/SfmSandbox/**` | C++ sandbox app | Project code | Subject to repository license/inherited course context |
| `shaders/sandbox/**` | GLSL sandbox shaders | Project code | Subject to repository license/inherited course context |
| `tests/sandbox/**` | C++ tests | Project code | Subject to repository license/inherited course context |
| `docs/sandbox/**` | Documentation | Project documentation | Subject to repository license/inherited course context |

## Dependency review status

The root `CMakeLists.txt` pins or references the current dependency set. These dependencies must be reviewed before any commercial or public binary release claim:

| Dependency | Use | Review status |
|---|---|---|
| Assimp | Model/resource loading support inherited from lab framework | Needs license review before distribution claim |
| GLFW | Window/context handling | Needs license review before distribution claim |
| GLM | Math library | Needs license review before distribution claim |
| ImGui | Development UI | Needs license review before distribution claim |
| GLAD | OpenGL loader | Needs license review before distribution claim |
| tinyfiledialogs | File dialog helper | Needs license review before distribution claim |
| stb | Utility/image-related dependency inherited from lab framework | Needs license review before distribution claim |

## Inherited course assets and framework code

This repository originated from a course/lab framework. Course framework code, legacy assignment resources and any inherited assets are not covered by the synthetic sandbox-sample statement above.

Before any external distribution beyond normal academic repository use, verify:

```text
repository license
course framework redistribution permission
third-party dependency licenses
resource archive contents
shader/resource ownership
attribution requirements
binary distribution obligations
```

## M20 policy

The sandbox may be described as a focused development tool. It must not be described as commercially ready until the dependency and inherited-resource review is complete.
