# Milestone 20 Record: Product Hardening, Packaging and Release Readiness

- **Milestone:** 20
- **Status:** Complete — locally validated by the developer
- **Date:** 2026-05-31
- **Development branch:** `feature/sfm-m20-hardening`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Move the SfM Visualization Sandbox from a milestone-era prototype line toward a maintainable focused tool.

M20 does not add another visualization feature. It adds the hardening layer needed to keep the project usable: CI configuration, correct run instructions, release-readiness documentation, resource/license inventory, rendering regression strategy and explicit release limitations.

## Implemented source boundaries

```text
.github/workflows/sandbox-ci.yml
README.md
docs/sandbox/release-readiness-m20.md
docs/sandbox/resource-license-inventory.md
docs/sandbox/rendering-regression-checklist.md
docs/sandbox/milestones/0020-product-hardening-packaging-release-readiness.md
```

## Implemented behavior and documentation

- Added GitHub Actions workflow for Windows + Ninja configure/build/test.
- Added root README with the supported VS 2026/Ninja workflow.
- Added launch instructions for `SfmSandbox`, `EDAF80_Assignment1` and `EDAN35_Assignment2`.
- Added release-readiness guide for the sandbox branch.
- Added resource/license inventory for sandbox-owned synthetic samples.
- Added explicit warning that inherited course assets and dependencies still need license review before commercial/public distribution claims.
- Added rendering regression checklist.
- Added manual screenshot-baseline strategy.
- Documented the current packaging status and limitations.
- Linked M18 and M19 performance-baseline procedures.
- Preserved the AI commit CI policy: intermediate AI commits skip CI, final PRs do not.

## CI workflow

M20 adds:

```text
.github/workflows/sandbox-ci.yml
```

The workflow runs on pull requests and pushes targeting:

```text
feature/sfm-visualization-sandbox
vs2026
```

The job uses:

```text
windows-latest
MSVC developer environment
Ninja
CMake configure
CMake build
CTest
```

## Roadmap alignment

| M20 roadmap item | Status |
|---|---|
| Automated tests included in CI | Implemented through GitHub Actions configure/build/CTest workflow |
| Rendering regression strategy or screenshot-baseline plan | Implemented in `docs/sandbox/rendering-regression-checklist.md` |
| Asset and dataset validation workflow | Implemented through resource/license inventory and reload failure policy documentation |
| Packaging/configuration handling | Documented in release-readiness guide |
| End-user documentation | Implemented through root README and release-readiness guide |
| Developer documentation | Implemented through regression checklist and readiness guide |
| Dependency, resource and license audit | Implemented as initial inventory with explicit unresolved dependency/course-asset review status |
| Performance target scenes and repeatable measurement procedure | Documented through M18/M19 links and M20 release-readiness guide |
| Cleanup of milestone-era debug wording where it should become product wording | Documentation now clearly distinguishes development-tool status from release/commercial readiness |

## Local validation status

The developer reported successful local validation on 2026-05-31 using the intended VS 2026/Ninja workflow.

Reference validation commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m20-hardening
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual launch checks:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe
build\src\EDAF80\EDAF80_Assignment1.exe
build\src\EDAN35\EDAN35_Assignment2.exe
```

Developer confirmation received:

```text
Confirmed
```

This confirmation covers the requested M20 validation checklist:

| Check | Result |
|---|---|
| README instructions are correct | Passed — developer confirmed |
| `SfmSandbox` still shows the M19 scene and previous workflows | Passed — developer confirmed |
| Legacy apps still launch | Passed — developer confirmed |
| CTest still passes | Passed — developer confirmed |
| Documentation accurately states non-commercial-release-ready status | Passed — developer confirmed |

## Known limitations

- M20 creates CI configuration, but CI must still run on the final pull request to prove the remote workflow.
- Screenshot regression is documented but still manual.
- No installer or versioned release archive is produced.
- No automated license scanner is added.
- Dependency licenses and inherited course assets still need review before any commercial/public distribution claim.
- GPU timer queries remain a later extension.
- The sandbox remains a focused development/learning tool rather than a finished product.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| CI covers build and available tests | Implemented; PR CI should verify remotely |
| Documented run instructions are correct on the supported local workflow | Passed |
| Sample resources have clear license/redistribution status | Passed for sandbox-owned synthetic samples |
| Performance baselines are reproducible | Passed through M18/M19 procedure docs and M20 links |
| Project does not claim commercial readiness beyond license review | Passed |
| Local validation completed | Passed |
| Known limitations documented | Passed |

Milestone 20 is complete and may be opened as a focused pull request into `feature/sfm-visualization-sandbox`.
