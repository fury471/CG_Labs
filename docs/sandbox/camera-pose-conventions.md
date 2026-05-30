# Camera Pose Conventions

This document records the camera-pose contract used by the SfM Visualization Sandbox from Milestone 14 onward.

The goal is to remove ambiguity between graphics camera transforms and common computer-vision reconstruction outputs before later milestones connect cameras, points and images.

## Internal sandbox convention

The sandbox stores every camera pose as a **camera-to-world** transform:

```text
CameraPose::camera_to_world
```

The transform maps local camera-space points into sandbox world space.

Internal camera axes are the graphics convention already used by the frustum renderer:

```text
+X = camera right
+Y = camera up
-Z = camera forward
```

Therefore, for a camera pose matrix `M = camera_to_world`:

```text
camera position  =  M[3].xyz
right direction  =  M[0].xyz
up direction     =  M[1].xyz
forward direction = -M[2].xyz
```

The renderer builds frustums in local camera space using negative Z depth, then transforms those vertices by `camera_to_world`.

## Existing milestone text pose format

The original milestone camera-pose text format is still supported:

```text
eye_x eye_y eye_z target_x target_y target_z r g b
```

The loader builds a graphics view matrix from `eye`, `target` and world up `(0, 1, 0)`, then stores the inverse as `camera_to_world`.

This format is intentionally simple and remains useful for synthetic checks, demos and regression tests.

## COLMAP / OpenCV image pose convention

Milestone 14 adds support for COLMAP text `images.txt` pose records.

The supported COLMAP pose line is:

```text
IMAGE_ID QW QX QY QZ TX TY TZ CAMERA_ID NAME
```

The following 2D observation line is skipped because M14 imports pose metadata only.

COLMAP image poses are treated as OpenCV-style **world-to-camera** extrinsics:

```text
x_camera = R_world_to_camera * x_world + t_world_to_camera
```

COLMAP/OpenCV camera axes are interpreted as:

```text
+X = camera right
+Y = camera down
+Z = camera forward
```

The world-space camera center is therefore:

```text
C_world = -transpose(R_world_to_camera) * t_world_to_camera
```

## COLMAP to sandbox conversion

The sandbox converts COLMAP/OpenCV camera space into the internal graphics convention by flipping the Y and Z camera axes:

```text
COLMAP/OpenCV: +X right, +Y down, +Z forward
Sandbox:       +X right, +Y up,   -Z forward
```

The basis-change matrix is:

```text
S = diag(1, -1, -1)
```

Given a COLMAP pose:

```text
R_cw = world-to-camera rotation
T_cw = world-to-camera translation
```

M14 computes:

```text
R_wc_cv      = transpose(R_cw)
C_world      = -R_wc_cv * T_cw
R_wc_sandbox = R_wc_cv * S
```

Then stores:

```text
camera_to_world = [ R_wc_sandbox  C_world ]
                  [ 0 0 0         1       ]
```

## Metadata contract

Each `CameraPose` now carries `CameraPoseMetadata` so the UI can explain where a pose came from.

Stored metadata includes:

```text
index
source_line
source_id
camera_id
source_file
source_format
source_convention
image_name
```

This metadata is deliberately separate from rendering. The renderer only needs `camera_to_world` and colour, while the UI and future image/projection milestones can inspect source identity and convention.

## Runtime reload policy

Camera-pose reload follows the same safety rule as point-cloud reload:

```text
A failed reload must not destroy the previous visible data.
```

The file importer must succeed first. Then the renderer builds replacement camera-frustum GPU resources. Only after both steps succeed does the app replace the active pose set.

## Current M14 supported import paths

Supported:

```text
sample_camera_poses.txt      milestone eye/target text format
images.txt                   COLMAP text image pose format
*.poses / *.cam              milestone eye/target text format
```

Not supported yet:

```text
COLMAP binary models
COLMAP cameras.txt intrinsics
COLMAP points3D observation metadata
OpenMVG JSON
Nerfstudio transforms.json
image loading or projection overlays
```

Those belong to later milestones once the pose convention foundation is stable.
