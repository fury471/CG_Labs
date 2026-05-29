# CG_Labs — VS2026 + Ninja Setup

This branch configures the Lund University Computer Graphics lab framework for a Windows development environment using the **Visual Studio 2026 MSVC toolchain** with **CMake + Ninja**.

> This documentation applies to the `vs2026` branch. Visual Studio supplies the compiler and developer environment; **Ninja** is the intended CMake generator for this branch.

## Included assignment targets

| Course | Executable targets |
| --- | --- |
| EDAF80 | `EDAF80_Assignment1`, `EDAF80_Assignment2`, `EDAF80_Assignment3`, `EDAF80_Assignment4`, `EDAF80_Assignment5` |
| EDAN35 | `EDAN35_Assignment2` |

## Requirements

On Windows, install or enable:

- Visual Studio 2026 with the **Desktop development with C++** workload
- MSVC compiler toolchain and a Windows SDK
- CMake
- Ninja
- Git

The first CMake configuration also retrieves/builds project dependencies such as Assimp, GLFW, GLM, TinyFileDialogs and stb, so network access is required for a clean initial setup.

## 1. Get the `vs2026` branch

Clone directly on this branch:

```powershell
git clone --branch vs2026 https://github.com/fury471/CG_Labs.git
cd CG_Labs
```

Or switch an existing clone to this branch:

```powershell
git fetch origin
git switch vs2026
```

## 2. Use a Visual Studio 2026 developer terminal

Open **Developer PowerShell for VS 2026**, then verify the toolchain:

```powershell
where.exe cl
where.exe cmake
where.exe ninja
where.exe git

cl
cmake --version
ninja --version
git --version
```

Do not configure the project until `cl`, `cmake`, `ninja`, and `git` can all be located from the same terminal session.

## 3. Clean old or incompatible CMake state

When migrating from VS2022, a Visual Studio solution generator, or a failed dependency build, remove stale generated state first:

```powershell
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue

Get-ChildItem .\dependencies -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '(-build|-subbuild)$' } |
    Remove-Item -Recurse -Force
```

## 4. Configure with Ninja

Configure a debug build from **Developer PowerShell for VS 2026**:

```powershell
cmake -S . -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=Debug `
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

The important points are:

- Use `-G Ninja`, not a Visual Studio solution generator.
- Ninja is a single-configuration generator, so explicitly provide `-DCMAKE_BUILD_TYPE=Debug` or `Release`.
- `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` addresses CMake policy compatibility issues that can occur in fetched legacy dependency projects with newer CMake versions.

For a release build in a separate output directory:

```powershell
cmake -S . -B build-release -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

## 5. Build

Build all configured assignment executables:

```powershell
cmake --build build --parallel
```

Build only the assignment you are developing:

```powershell
cmake --build build --target EDAN35_Assignment2 --parallel
cmake --build build --target EDAF80_Assignment5 --parallel
```

## 6. Run

For a debug build configured in `build`, examples are:

```powershell
.\build\src\EDAN35\EDAN35_Assignment2.exe
.\build\src\EDAF80\EDAF80_Assignment5.exe
```

Use the equivalent `build-release` path when working with the release configuration.

## Troubleshooting

### Ninja cannot be found

If CMake reports:

```text
CMake was unable to find a build program corresponding to "Ninja"
```

verify Ninja in the same developer terminal:

```powershell
where.exe ninja
ninja --version
```

Install or expose Ninja in the Visual Studio 2026 developer environment, reopen the terminal, delete `build`, and configure again.

### C/C++ compiler cannot be found

If CMake reports that `CMAKE_CXX_COMPILER` or `CMAKE_C_COMPILER` is not set, the terminal is not exposing the MSVC toolchain. Check:

```powershell
where.exe cl
cl
```

Then configure again from **Developer PowerShell for VS 2026** after removing the failed `build` directory.

### Dependency configuration errors after changing toolchains

Remove cached build output and generated dependency build folders before rerunning CMake:

```powershell
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue

Get-ChildItem .\dependencies -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '(-build|-subbuild)$' } |
    Remove-Item -Recurse -Force

cmake -S . -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=Debug `
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

## Project directories

```text
CMake/          CMake dependency and resource retrieval scripts
src/core/       Shared Bonobo framework and application utilities
src/EDAF80/     EDAF80 assignment source files
src/EDAN35/     EDAN35 assignment source files
shaders/        GLSL shader files
res/            Runtime resource assets
```
