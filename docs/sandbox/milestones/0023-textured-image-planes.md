# Milestone 23 Record: Textured Image Planes and Calibrated Image Groundwork

- **Milestone:** 23
- **Status:** Complete - locally validated
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Replace the M17 debug image card with a real texture-backed image-plane path
while preserving the existing camera-image association workflow.

## Implemented source boundaries

```text
shaders/sandbox/image_plane.vert
shaders/sandbox/image_plane.frag
src/sandbox/gfx/Texture2D.hpp/.cpp
src/sandbox/gfx/Sampler.hpp/.cpp
src/sandbox/gfx/ImagePlaneRenderer.hpp
docs/sandbox/image-projection-debugging.md
docs/sandbox/milestones/0023-textured-image-planes.md
```

## Implemented behavior

- Added `Texture2D::allocate_storage(...)`.
- Added `Texture2D::upload_level(...)`.
- Added sampler parameter helpers and a linear-clamp sampler policy.
- Added dedicated image-plane vertex and fragment shaders.
- Changed image-plane vertices from position/colour to position/UV.
- Uploaded imported PPM pixels into an OpenGL `GL_RGB32F` texture.
- Bound texture/sampler unit 0 during image-plane rendering.
- Preserved image reload transaction behavior: failed file or GPU upload keeps
  the previous visible image plane.
- Preserved image-plane draw statistics.

## Local validation

Validated locally on 2026-05-31 through the Visual Studio 2026 developer
environment wrapper:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --help
```

## Known limitations

- The only supported image file format remains ASCII PPM P3.
- The image plane is textured but not yet calibrated with imported intrinsics.
- No reprojection error overlay is computed.
- No GPU screenshot regression is automated yet.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Image plane displays through an OpenGL texture path | Passed |
| Texture storage/upload and sampler policy exist in `sandbox/gfx` | Passed |
| Invalid image reloads preserve the previous image plane | Preserved by transaction path |
| Renderer statistics continue to account for image-plane draws | Passed |
| Documentation states remaining calibration limits | Passed |

Milestone 23 is complete. Next scheduled work is Milestone 24: automated visual
regression capture path.
