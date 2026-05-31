# Development Release Candidate Packaging

Milestone 26 defines the highest readiness level currently supported by
evidence: a development release candidate for the SfM Visualization Sandbox.
It is not a commercial-release approval.

## No-Window Validation

Use `--validate-install` to verify startup assets and shader lookup without
opening a window:

```bat
build\src\apps\SfmSandbox\SfmSandbox.exe --validate-install
```

The command checks:

```text
project manifest
point-cloud sample
camera-pose sample
surface sample
image sample
required sandbox shaders
```

CTest includes the same validation through:

```text
sfm_sandbox_startup_validation
```

## Development Install Smoke Check

Build, install and validate from the installed `bin` directory:

```bat
cmake --build build
cmake --install build --prefix build\install-sfm-sandbox
pushd build\install-sfm-sandbox\bin
SfmSandbox.exe --validate-install
popd
```

The install layout is intentionally simple:

```text
bin\SfmSandbox.exe
bin\shaders\...
bin\res\...
```

Shader loading first tries the runtime working directory, then falls back to the
source-tree path embedded in sandbox builds. The packaged smoke check should be
run from the installed `bin` directory so it validates the installed resources.

The root install target also installs legacy assignment executables and
inherited resources. Treat this as a development smoke layout, not a curated
redistribution package.

## Current Readiness Claim

Supported claim:

```text
Focused development viewer with manifest-driven startup resources, parser tests,
startup validation, visual capture, CPU/GPU timing diagnostics and a development
install smoke workflow.
```

Unsupported claim:

```text
Commercially cleared, redistributable end-user product.
```

That stronger claim still requires full inherited-course and dependency license
review, runtime dependency audit, release archive policy and user-facing error
handling beyond the current development UI.
