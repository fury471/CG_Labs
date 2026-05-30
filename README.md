# CG Labs — VS 2026 / SfM Visualization Sandbox

This repository contains the Lund University Computer Graphics lab framework plus a focused SfM Visualization Sandbox developed on the `feature/sfm-visualization-sandbox` branch.

The sandbox is a development/learning tool for visualizing structure-from-motion style data:

```text
point clouds
camera poses and frustums
camera metadata
simple reconstructed surfaces
camera-image relationships
projection-debug image planes
render-target/profiling diagnostics
instanced marker stress scenes
```

It is not yet a commercial product or a general SfM reconstruction package.

## Supported local workflow

Use a Visual Studio 2026 x64 developer environment with Ninja available on `PATH`.

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-visualization-sandbox
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Launch checks:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe
build\src\EDAF80\EDAF80_Assignment1.exe
build\src\EDAN35\EDAN35_Assignment2.exe
```

## Main application

`SfmSandbox` is the main new application:

```text
src/apps/SfmSandbox
```

Core sandbox modules live under:

```text
src/sandbox/core
src/sandbox/scene
src/sandbox/gfx
```

Shader and sample resources live under:

```text
shaders/sandbox
res/sandbox
```

## GitHub Actions

M20 adds CI for pull requests into the integration branch:

```text
.github/workflows/sandbox-ci.yml
```

The workflow configures with CMake, builds with Ninja and runs CTest on Windows.

AI-assisted intermediate commits intentionally include `[skip ci]` to avoid unnecessary workflow runs. Final pull requests should not skip CI.

## Documentation

Important sandbox documents:

```text
docs/sandbox/ROADMAP.md
docs/sandbox/ENGINEERING_STANDARDS.md
docs/sandbox/AI_COMMIT_CI_POLICY.md
docs/sandbox/release-readiness-m20.md
docs/sandbox/performance-baseline-m18.md
docs/sandbox/instancing-stress-m19.md
docs/sandbox/image-projection-debugging.md
docs/sandbox/surface-mesh-format.md
```

Milestone completion records are stored in:

```text
docs/sandbox/milestones
```

## Resource status

The `res/sandbox` samples are synthetic development samples created for this sandbox line. They are safe to keep in the repository.

Inherited course resources and third-party dependencies need license review before any commercial or public distribution claim.

## Current limitations

- Surface loading supports only a tiny documented OBJ subset.
- Image loading supports only ASCII PPM P3 debug samples.
- Screenshot regression is manual.
- GPU timer queries are not implemented yet.
- Packaging is development-oriented, not an installer or polished release bundle.
- The project should not claim commercial readiness until dependency/resource licensing is reviewed.
