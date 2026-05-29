# Milestone 0 Record: Baseline and Architecture Documentation

- **Milestone:** 0
- **Status:** Complete for architecture start; performance capture pending
- **Date:** 2026-05-29
- **Integration branch:** `feature/sfm-visualization-sandbox`
- **Base commit from `vs2026`:** `e6f51ab0dd3e8019104b1c7d9a76170d6ec44c6a`

## Purpose

Milestone 0 establishes a stable starting point before new product-oriented C++ source is introduced. It records the architectural intent, development rules and baseline validation approach for the SfM Visualization Sandbox.

## Confirmed baseline facts

### Repository and codebase

- The inherited project is `CG_Labs`, a Lund University computer-graphics repository for EDAF80 and EDAN35.
- The current modernization line is `vs2026`.
- The sandbox development line is `feature/sfm-visualization-sandbox`, created from the base commit shown above.
- Existing course/application targets include `EDAF80_Assignment1` through `EDAF80_Assignment5` and `EDAN35_Assignment2`.

### Development environment workflow

The local development workflow uses a Visual Studio x64 developer terminal so that MSVC and its SDK environment variables are active, with Ninja as the CMake generator.

Reference configuration/build commands:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Reference executable-discovery command when using `cmd.exe`:

```bat
dir /s /b build\*.exe
```

The command differs from PowerShell equivalents; development records must state which terminal environment is being used when giving local commands.

### Manual regression validation

The developer confirmed completion of the initial build-and-run validation before implementation begins. These two legacy applications are selected as minimum regression references:

| Application | Regression role |
|---|---|
| `EDAF80_Assignment1` | Simple forward-rendered scene hierarchy/texturing reference |
| `EDAN35_Assignment2` | Advanced multi-pass deferred/shadow renderer reference |

From Milestone 1 onward, `SfmSandbox` will be added as a third required manual launch check.

## Documents created in Milestone 0

| Document | Purpose |
|---|---|
| `docs/sandbox/README.md` | Charter, scope, goals, non-goals and documentation index |
| `docs/sandbox/ARCHITECTURE.md` | Layer boundaries, core model, rendering flow and extension policy |
| `docs/sandbox/ROADMAP.md` | Milestones, acceptance criteria and permanent quality gates |
| `docs/sandbox/ENGINEERING_STANDARDS.md` | C++/OpenGL/CMake/performance/testing/Git/legal working rules |
| `docs/sandbox/adr/0001-build-alongside-bonobo.md` | Decision to build new code beside retained lab code |
| `docs/sandbox/adr/0002-cpp20-opengl46-resource-and-renderer-policy.md` | Initial language, backend, ownership and submission policy |
| `docs/sandbox/milestones/0000-baseline.md` | This baseline record |

## Deliberately not yet implemented

No new sandbox executable, graphics wrapper, scene component or visualization feature is introduced in Milestone 0. This is intentional: the design contract must exist before code commits begin.

## Pending baseline evidence

Before making a performance comparison later, record the following in a future measurement note:

| Item | Status |
|---|---|
| GPU model | Pending |
| Graphics driver version | Pending |
| CPU model | Pending |
| Release build confirmation for measured run | Pending |
| Window/framebuffer resolution | Pending |
| `EDAN35_Assignment2` CPU frame time | Pending |
| G-buffer GPU time | Pending |
| Shadow-map GPU time per active light | Pending |
| Light-accumulation GPU time per active light | Pending |
| Resolve GPU time | Pending |
| Scene/camera state screenshot | Pending |

This missing evidence does not block Milestone 1 application-shell development. It does block honest before/after performance claims concerning later renderer optimizations.

## Milestone 0 exit assessment

| Gate | Result |
|---|---|
| Sandbox branch created from modernized baseline | Passed |
| Local build/run baseline established by developer | Passed |
| Architecture written before new product code | Passed |
| Engineering standards recorded | Passed |
| Initial ADRs recorded | Passed |
| Detailed numerical timing archive | Deferred, required before optimization comparison |

## Next action

Proceed to **Milestone 1 — Application shell and target boundaries**. The first implementation must add the new `SfmSandbox` executable and target-scoped C++20 library structure while preserving all legacy assignment behaviour.
