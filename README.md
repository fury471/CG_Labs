# CG_Labs — Visual Studio 2026 and Ninja setup

This branch modernizes the Lund University Computer Graphics lab framework for the current Visual Studio toolchain while keeping the EDAF80 and EDAN35 assignment structure intact.

## Requirements on Windows

Install the following before configuring the project:

- **Visual Studio 2026 Community** with:
  - Desktop development with C++
  - C++ CMake tools for Windows
  - MSVC x64 build tools
  - A Windows SDK
- **Git**
- **Python 3.12** (recommended for the GLAD 2 generator)

The project uses **CMake with Ninja**. Do not generate a traditional Visual Studio solution for this branch.

## Important: GLAD is generated during the first build

The project uses GLAD 2 to generate an OpenGL 4.6 core-profile loader during the build. Therefore, `glad/gl.h` does **not** exist in a fresh checkout until GLAD generation has completed successfully.

GLAD is retrieved as source code by CMake and is run with Python from its source directory. The Python interpreter does not need a separately installed `glad2` package, but it **does** need GLAD's Python dependency, `Jinja2`.

Visual Studio is configured through `CMakeSettings.json` to use the repository-local interpreter:

```text
.venv-glad\Scripts\python.exe
```

Create this environment once before building from Visual Studio.

## One-time setup for Visual Studio 2026

Open **x64 Native Tools Command Prompt for VS 2026** or **Developer Command Prompt for VS 2026**, then run:

```cmd
cd E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs

py -3.12 -m venv .venv-glad
".\.venv-glad\Scripts\python.exe" -m pip install --upgrade pip
".\.venv-glad\Scripts\python.exe" -m pip install "Jinja2>=2.7,<4.0"
```

If the repository is cloned in a different location, replace the `cd` path with your own project directory.

Verify that the required build tools are visible from the developer command prompt:

```cmd
cmake --version
ninja --version
where cl
".\.venv-glad\Scripts\python.exe" -c "import jinja2; print(jinja2.__version__)"
```

The last command must print a Jinja2 version number.

## Build inside Visual Studio 2026

1. Open Visual Studio and choose **Open a local folder**.
2. Select the root folder of `CG_Labs`.
3. Let Visual Studio finish its CMake configuration.
4. Select an executable target, for example `EDAF80_Assignment1.exe`.
5. Choose **Build > Build All** or press `Ctrl+Shift+B`.
6. Run using `F5` or `Ctrl+F5`.

The supplied `CMakeSettings.json` uses:

- Generator: `Ninja`
- Debug build directory: `out\build\x64-Debug`
- Release-with-debug-info build directory: `out\build\x64-Release`
- Python interpreter for GLAD: `.venv-glad\Scripts\python.exe`

The first build is slower because CMake retrieves/builds third-party libraries and generates GLAD sources. Later builds are incremental.

## Build from the command line with Ninja

From a Visual Studio developer command prompt:

```cmd
cd E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs

cmake -S . -B build -G Ninja ^
  "-DCMAKE_BUILD_TYPE=Debug" ^
  "-DPython_EXECUTABLE=%CD%\.venv-glad\Scripts\python.exe"

cmake --build build --verbose
```

A successful build produces executable targets below the build directory, for example:

```text
build\src\EDAF80\EDAF80_Assignment1.exe
```

Run it from the project root with:

```cmd
.\build\src\EDAF80\EDAF80_Assignment1.exe
```

## Troubleshooting

### Visual Studio reports `cannot open source file "glad/gl.h"`

This usually means GLAD generation did not complete in Visual Studio's current build directory. Inspect the first failed step in the **Build Output** window rather than the IntelliSense error list.

If the failed command contains:

```text
python.exe -m glad
```

and Python reports a missing `jinja2` module, rerun the one-time virtual-environment setup above, then delete the Visual Studio cache and reconfigure:

```cmd
rmdir /s /q .vs 2>nul
rmdir /s /q out\build\x64-Debug 2>nul
```

Open the project folder in Visual Studio again and build.

### CMake/Ninja cannot find the compiler

Run configuration and build commands from a **Visual Studio Developer Command Prompt**, not from an ordinary terminal. Check:

```cmd
where cl
ninja --version
cmake --version
```

### The program builds, but ImGui controls cannot be clicked or dragged

The framework must forward GLFW mouse-button and cursor-position callbacks to Dear ImGui. This branch includes that forwarding in `src/core/WindowManager.cpp`.

## Generated files and Git

Do not commit local build products or local Python environments. In particular, the following are intentionally ignored:

```text
build/
out/
dependencies/
.vs/
.venv-glad/
glad-test/
```

## Dependency versions on this branch

The root CMake configuration pins the principal modernized dependencies, including:

- Assimp `6.0.5`
- GLFW `3.4`
- GLM `1.0.3`
- Dear ImGui `v1.92.8`
- GLAD `v2.0.8`

These versions are retrieved by CMake during initial configuration/build.
