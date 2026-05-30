# Parser Validation Guarantees

## Purpose

The sandbox is intended to become a powerful and extensible SfM visualization tool, not a throwaway prototype. File parsing is therefore treated as a product-quality boundary: unsupported or malformed input must fail visibly and reproducibly instead of producing silent, misleading visualization data.

This document records the behavior currently covered by Milestone 11 tests.

## Point-cloud text parser

Function:

```cpp
sfm::scene::load_point_cloud_from_text_file(path)
```

Supported rows:

```text
x y z
x y z r g b
x,y,z
x,y,z,r,g,b
```

Current guarantees:

- text after `#` is treated as a comment;
- empty/comment-only lines are ignored;
- comma and whitespace separators are accepted;
- rows must contain exactly 3 or 6 numeric fields;
- position values must be finite;
- 3-field rows receive the documented default colour;
- 6-field rows may use normalized `[0, 1]` RGB or byte-style `[0, 255]` RGB;
- malformed rows are skipped and counted;
- loading succeeds if at least one valid point exists;
- loading fails if the file is missing or no valid point rows exist.

Known exclusions:

- this is not a PLY/LAS/OBJ parser;
- no binary point-cloud format is accepted;
- no normals, confidence values, track IDs or source observation metadata are parsed yet.

## Camera-pose text parser

Function:

```cpp
sfm::scene::load_camera_poses_from_text_file(path)
```

Supported rows:

```text
eye_x eye_y eye_z target_x target_y target_z r g b
```

Current guarantees:

- text after `#` is treated as a comment;
- empty/comment-only lines are ignored;
- comma and whitespace separators are accepted;
- rows must contain exactly 9 numeric fields;
- eye and target values must be finite;
- degenerate eye-to-target directions are rejected;
- near-singular directions parallel to the fixed up vector are rejected;
- colour values may use normalized `[0, 1]` RGB or byte-style `[0, 255]` RGB;
- malformed rows are skipped and counted;
- loading succeeds if at least one valid pose exists;
- loading fails if the file is missing or no valid pose rows exist.

Known exclusions:

- no COLMAP, Bundler, OpenMVG or full-matrix camera import exists yet;
- no quaternion pose format exists yet;
- no intrinsic calibration values are parsed yet;
- coordinate-convention conversion tests are scheduled for a later milestone.

## Test policy

Milestone 11 introduces CPU-side tests for deterministic parser behavior. These tests do not require an OpenGL context and are expected to run locally through CTest and in CI where practical.

A parser behavior should not be changed without either updating these tests or adding new tests that document the intended behavior.
