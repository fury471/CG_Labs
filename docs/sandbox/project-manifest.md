# Sandbox Project Manifest

Milestone 21 introduces a small project manifest so `SfmSandbox` can start from
dataset paths instead of relying only on hard-coded sample constants.

The manifest is not a full scene format. It is a stable startup configuration
boundary for the focused SfM visualization tool.

## Default manifest

The default checked-in manifest is:

```text
res/sandbox/default_project.sfmproj
```

`SfmSandbox` loads this file at startup unless a different project file is
provided with:

```text
SfmSandbox.exe --project path\to\project.sfmproj
```

The no-window mesh builder also uses the selected project manifest:

```text
SfmSandbox.exe --project path\to\project.sfmproj --build-mesh output.obj
```

## Format

The manifest is line-oriented UTF-8 compatible text:

```text
key = value
```

Blank lines are ignored. Text after `#` is a comment. Keys are case-sensitive.
Paths may be absolute or relative. Relative paths resolve against the directory
containing the manifest file.

Supported keys:

```text
point_cloud   required
camera_poses  optional
surface       optional
image         optional
```

Example:

```text
point_cloud = sample_point_cloud_ascii.ply
camera_poses = sample_camera_poses.txt
surface = sample_surface.obj
image = sample_camera_image.ppm
```

Only `point_cloud` is required. Optional layers may be omitted or set to an
empty value:

```text
point_cloud = sample_point_cloud_ascii.ply
camera_poses =
surface =
image =
```

Empty optional values mean the layer starts disabled. The sandbox should not
fall back to demo camera poses, sample surfaces or sample images when a selected
project intentionally leaves those fields empty.

## Failure policy

The parser reports unsupported keys, malformed lines and a missing
`point_cloud` path as diagnostics. Startup still falls back to the documented
sample resources when the selected manifest cannot be loaded, so the sandbox
remains runnable during development. Empty optional paths are not errors.

Individual asset reloads keep their existing transactional behavior: a failed
point-cloud, pose, surface or image load does not destroy the previous visible
data.

## Current limitations

- The manifest stores startup paths only.
- `--build-mesh` currently consumes only the `point_cloud` path.
- It does not persist UI settings yet.
- It does not describe multiple images, camera intrinsics, dense tracks or
  reconstruction metadata.
- It is intentionally simple until real dataset adapters require richer project
  structure.
