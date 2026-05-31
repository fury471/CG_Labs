# Milestone 21 Record: Project Manifest and Importer Module Cleanup

- **Milestone:** 21
- **Status:** Complete - locally validated
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Turn the post-M20 sandbox from a sample-path-driven development prototype into
a more configurable and maintainable tool foundation.

M21 introduces a startup project manifest and moves the M16/M17 header-only
import implementations into registered scene-library source files.

## Implemented source boundaries

```text
docs/sandbox/project-manifest.md
res/sandbox/default_project.sfmproj
src/sandbox/scene/SandboxProject.hpp/.cpp
src/sandbox/scene/SurfaceImport.hpp/.cpp
src/sandbox/scene/ImageImport.hpp/.cpp
src/sandbox/scene/CMakeLists.txt
src/apps/SfmSandbox/main.cpp
tests/sandbox/SceneParserTests.cpp
tests/sandbox/SurfaceImportSmokeTest.cpp
tests/sandbox/ImageImportSmokeTest.cpp
```

## Implemented behavior

- Added a line-oriented `key = value` sandbox project manifest.
- Added default startup manifest at `res/sandbox/default_project.sfmproj`.
- Added `--project <path>` command-line override.
- Added `--help` command-line output.
- Resolved relative manifest paths against the manifest file.
- Kept sample fallback startup paths when the selected manifest cannot load.
- Moved surface import implementation into `SurfaceImport.cpp`.
- Moved image import implementation into `ImageImport.cpp`.
- Registered surface, image, mesh compatibility and project implementation files
  in `sfm_sandbox_scene`.
- Updated root README and sandbox docs for the project manifest workflow.

## Test coverage

M21 adds or expands CPU-side tests for:

- supported project manifest parsing;
- relative path resolution;
- missing manifest files;
- missing required manifest keys;
- unsupported manifest keys;
- surface statistics;
- malformed surface faces;
- unsupported surface extensions;
- image nearest sampling;
- invalid PPM headers;
- incomplete PPM samples;
- unsupported image extensions.

## Local validation

Validated locally on 2026-05-31 through the Visual Studio 2026 developer
environment wrapper:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --help
```

CTest result:

```text
3/3 tests passed
```

The plain PowerShell build environment exposed `cl.exe` without the standard
library include paths. Building through `VsDevCmd.bat -arch=x64` fixed the
environment and produced a clean build.

## Known limitations

- The manifest stores startup asset paths only.
- UI settings are not persisted yet.
- There is no in-app project save/update flow.
- Multiple images, intrinsics, tracks and richer reconstruction metadata remain
  future work.
- Image display is still the M17 debug colour-card path; textured image planes
  are scheduled for M23.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| `SfmSandbox` starts from manifest-defined startup paths | Passed |
| `--project <path>` startup override exists | Passed |
| Invalid manifests fall back to documented sample paths | Passed |
| Scene importers compile as normal library sources | Passed |
| CTest covers manifest and importer failure cases | Passed |
| Root README and sandbox docs describe manifest workflow | Passed |

Milestone 21 is complete. Next scheduled work is Milestone 22: application state
and UI composition cleanup.
