# M16 Surface Mesh Format

Milestone 16 adds the first reconstructed-surface inspection capability.

The milestone intentionally supports one small, documented mesh format instead of a general asset importer.

## Supported format

M16 supports a tiny ASCII OBJ subset:

```text
v x y z
f i j k
```

Example:

```text
v 0 1 0
v -1 0 -1
v 1 0 -1
f 1 2 3
```

OBJ face indices are one-based. The importer stores them internally as zero-based triangle indices.

## Deliberately out of scope

The following OBJ features are ignored or rejected in M16:

```text
materials
texture coordinates
normals
quads
negative indices
smoothing groups
object/group semantics
```

This is deliberate. M16 is about validating the surface-inspection workflow: load a simple reconstructed surface, upload it to the renderer, display it beside the point cloud and camera frustums, and expose basic statistics.

## Runtime behavior

The sandbox loads:

```text
res/sandbox/sample_surface.obj
```

The surface panel exposes:

```text
visibility toggle
surface colour
surface path reload
vertex and triangle counts
surface bounds
surface draw statistics
malformed-input messages
```

Failed surface reloads must keep the previous visible surface. This mirrors the point-cloud and camera-pose reload policy from earlier milestones.

## Implementation note

The M16 importer and surface data model are currently header-only in `src/sandbox/scene/SurfaceImport.hpp`. This keeps the change isolated while the renderer and inspection workflow are being validated. A later cleanup milestone may split the implementation into `.cpp` files and register them in CMake once the surface data model stabilizes.
