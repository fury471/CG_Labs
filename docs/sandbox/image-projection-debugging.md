# M17 Image Projection Debugging

Milestone 17 connects camera poses with source imagery in a deliberately small and inspectable way.

The goal is not a full image-processing pipeline. The goal is to make the relationship between camera pose, frustum direction, point cloud and source image visible enough to debug reconstruction alignment.

## Supported sample image format

M17 uses a tiny license-safe synthetic image resource:

```text
res/sandbox/sample_camera_image.ppm
```

The importer supports ASCII PPM P3 files:

```text
P3
width height
max_value
r g b ...
```

This format was chosen because it is human-readable, dependency-free and safe to include as a project-owned sample asset.

## Image association model

For M17, the sandbox associates one loaded image with one selected camera pose.

The UI exposes:

```text
associated camera index
image path reload
image-plane visibility
image-plane distance
image-plane height
image import messages
image-plane draw statistics
```

Later milestones can replace this with many-to-many reconstruction image metadata, but M17 keeps the first relationship explicit and testable.

## Projection sanity check

The image plane is built in the selected camera's local space and transformed by that camera's `camera_to_world` matrix.

Internal sandbox camera convention from M14:

```text
+X = camera right
+Y = camera up
-Z = camera forward
```

Therefore, the synthetic image card is placed at local depth:

```text
z = -image_plane_distance
```

Its centre is:

```text
local centre = (0, 0, -image_plane_distance)
world centre = camera_to_world * local centre
```

The visual expectation is:

```text
The image plane centre lies on the selected camera frustum's forward ray.
```

This is the M17 projection sanity check. If the selected camera changes, or if the distance/height controls change, the image plane is rebuilt from the selected pose.

## Current limitations

- The image card is a coloured debug plane, not a textured OpenGL sampler path.
- The displayed colours come from the loaded image's corner samples and are interpolated across two triangles.
- Only ASCII PPM P3 is supported.
- No camera intrinsics are imported yet.
- No point reprojection error overlay is computed yet.
- The implementation validates pose/image association visually before introducing a richer texture and calibration pipeline.
