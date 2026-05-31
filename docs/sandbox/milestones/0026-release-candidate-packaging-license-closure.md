# Milestone 26 Record: Release Candidate Packaging and License Closure

- **Milestone:** 26
- **Status:** Complete for development-release candidacy
- **Date:** 2026-05-31
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Define what the sandbox can honestly claim after productization and add a
repeatable development install validation path.

## Implemented Scope

- Added `--validate-install` no-window startup/resource/shader validation.
- Added CTest coverage through `sfm_sandbox_startup_validation`.
- Documented the development install smoke workflow.
- Updated the resource/license inventory with local dependency-license evidence.
- Kept commercial release claims explicitly out of scope.

## Validation

Validated with:

```bat
cmake --build build --parallel
ctest --test-dir build --output-on-failure
build\src\apps\SfmSandbox\SfmSandbox.exe --validate-install
cmake --install build --prefix build\install-sfm-sandbox
pushd build\install-sfm-sandbox\bin
SfmSandbox.exe --validate-install
popd
```

## Known Limitations

- No installer or versioned release archive is produced.
- Inherited course resources still require separate redistribution review.
- Dependency license evidence is recorded, but no automated license scanner is
  integrated.
- Development install validation should be run from the installed `bin`
  directory.
