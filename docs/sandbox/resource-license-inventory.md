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
| `res/sandbox/default_project.sfmproj` | Sandbox project manifest | Project startup configuration | May be redistributed with this repository |

## Project-owned sandbox code and shaders

| Path | Type | Origin | Redistribution status |
|---|---|---|---|
| `src/sandbox/**` | C++ sandbox libraries | Project code | Subject to repository license/inherited course context |
| `src/apps/SfmSandbox/**` | C++ sandbox app | Project code | Subject to repository license/inherited course context |
| `shaders/sandbox/**` | GLSL sandbox shaders | Project code | Subject to repository license/inherited course context |
| `tests/sandbox/**` | C++ tests | Project code | Subject to repository license/inherited course context |
| `docs/sandbox/**` | Documentation | Project documentation | Subject to repository license/inherited course context |

## Dependency review status

The root `CMakeLists.txt` pins or references the current dependency set. M26
records the local license evidence below, but this is still not a complete
commercial-release legal review because inherited course/framework obligations
and binary packaging notices still need final verification.

| Dependency | Use | Local license evidence | Release status |
|---|---|---|---|
| Assimp | Model/resource loading support inherited from lab framework | `dependencies/assimp-src/LICENSE`, BSD-style redistribution terms | Notice/attribution obligations must be included in any binary package |
| GLFW | Window/context handling | `dependencies/glfw-src/LICENSE.md`, zlib/libpng-style license text | Notice/attribution obligations must be included in any binary package |
| GLM | Math library | `dependencies/glm-src/copying.txt`, Happy Bunny or MIT license | Notice/attribution obligations must be included in any binary package |
| ImGui | Development UI | `dependencies/imgui-src/LICENSE.txt`, MIT license | Notice/attribution obligations must be included in any binary package |
| GLAD | OpenGL loader | `dependencies/glad-src/LICENSE`, MIT license for generated loader/source | Notice/attribution obligations must be included in any binary package |
| tinyfiledialogs | File dialog helper | `dependencies/tinyfiledialogs-src/README.txt`, zlib licence statement | Notice/attribution obligations must be included in any binary package |
| stb | Utility/image-related dependency inherited from lab framework | `dependencies/stb-src/LICENSE`, MIT or public-domain choice | Notice/attribution obligations must be included in any binary package if MIT is chosen |

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

## M26 development-release statement

The sandbox-owned samples, shaders, source and documentation are inventoried for
development use. Dependency license evidence is now recorded from local source
trees. A distributable release still needs:

```text
final third-party notices file
inherited course/framework redistribution decision
resource archive contents review
binary runtime dependency audit
release archive contents review
```
