Table of Content
================

1. `Setting up the software stack`_

   a. `Common requirements`_
   b. `On Windows`_
   c. `On macOS`_
   d. `On Linux`_

2. `Getting started`_

   a. `Visual Studio 2026 on Windows: using Ninja`_
   b. `Using CMake on macOS and Linux`_


Setting up the software stack
=============================

.. note::
   This ``vs2026`` branch modernises the dependencies and OpenGL setup. It
   requests an OpenGL 4.6 core context on Windows and Linux, while retaining an
   OpenGL 4.1 forward-compatible core context on macOS because that is the
   latest OpenGL context exposed natively by macOS. Do not reuse build folders
   generated from older branches or from the pre-modernisation configuration.


Common requirements
-------------------

The project now requires:

* a C++17-capable compiler;
* CMake_ version 3.24 or newer;
* Git_ for retrieving pinned dependencies;
* Python_ 3 with Jinja2_, used while CMake generates the glad 2 OpenGL loader;
* a driver supporting the requested OpenGL context on the platform where the
  executable will run.

The first clean configuration downloads pinned versions of Assimp, GLFW, GLM,
Dear ImGui, glad, stb and tinyfiledialogs. Network access is therefore required
for that configuration.

To isolate the glad generator requirement, the examples below create a Python
virtual environment and install Jinja2 there. If an appropriate Jinja2 package
is already available to the Python interpreter selected by CMake, this step can
be omitted.


On Windows
----------

This branch is configured for Microsoft’s `Visual Studio`_ 2026. When
installing Visual Studio, select the *Desktop development with C++* workload
and ensure that these components are enabled:

* the current MSVC C++ x64/x86 build tools for Visual Studio 2026;
* a Windows SDK;
* C++ CMake tools for Windows;
* Ninja, either provided by Visual Studio's CMake tooling or otherwise
  available in the developer terminal;
* Python 3, either installed independently or available from your development
  environment.

Install Git_ separately if it is not already available.

The instructions below use **Developer PowerShell for VS 2026**, which exposes
MSVC to CMake while Ninja supplies the build backend. Verify the tools from that
same terminal session:

.. code-block:: powershell

   where.exe cl
   where.exe cmake
   where.exe ninja
   where.exe git
   where.exe python

   cl
   cmake --version
   ninja --version
   git --version
   python --version

.. figure:: images/VS2019_Components.jpg
   :alt: Select the Desktop development with C++ workload and ensure that an
         MSVC compiler toolset, Windows SDK, and CMake tools for Windows are
         enabled. The displayed screenshot is from an older Visual Studio
         version, but the corresponding component categories remain relevant.

   Historical component-selection example; choose the corresponding Visual
   Studio 2026 components in the current installer.


On macOS
--------

Install Xcode_ from the Mac App Store, then install its command-line tools:

.. code-block:: console

   xcode-select --install

You additionally need CMake_, Git_, and Python_ 3. The project creates an Xcode
build tree on macOS and requests an OpenGL 4.1 forward-compatible core context;
OpenGL 4.6 functionality must not be used for a code path intended to run
natively on macOS.


On Linux
--------

The code is tested with both Clang_ and GCC_. Use a C++17-capable version of
either compiler, together with CMake_ 3.24 or newer, Git_, Ninja_, Python_ 3,
and the X11 development packages needed to compile GLFW 3.4.

For Ubuntu/Debian-based systems, install the required build packages with:

.. code-block:: console

   sudo apt-get update
   sudo apt-get install -y cmake git ninja-build python3-venv \
       libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

This branch requests an OpenGL 4.6 core context at runtime on Linux, so the
system graphics driver must expose OpenGL 4.6 to run the assignments.


Getting started
===============

.. note::
   All files and folders are specified relative to the repository root (the
   folder containing among others this file, ``README.rst``, ``src/`` and
   ``shaders/``), unless specified otherwise.


Visual Studio 2026 on Windows: using Ninja
------------------------------------------

Here is the recommended Windows workflow for this ``vs2026`` branch.

1. Clone this fork and check out ``vs2026``:

   .. code-block:: powershell

      git clone --branch vs2026 https://github.com/fury471/CG_Labs.git
      cd CG_Labs

   For an existing clone, instead run:

   .. code-block:: powershell

      git fetch origin
      git switch vs2026
      git pull origin vs2026

2. Open **Developer PowerShell for VS 2026** in the repository root. Do not
   configure the project from a terminal where ``cl.exe`` or ``ninja.exe`` is
   unavailable.

3. Create a Python environment for glad 2 code generation:

   .. code-block:: powershell

      python -m venv .\.venv-glad
      .\.venv-glad\Scripts\python.exe -m pip install --upgrade pip
      .\.venv-glad\Scripts\python.exe -m pip install "Jinja2>=2.7,<4.0"

4. If you configured this checkout with an older dependency stack, Visual
   Studio 2019/2022, a Visual Studio solution generator, or an unsuccessful
   CMake run, remove the generated state first:

   .. code-block:: powershell

      Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue
      Remove-Item -Recurse -Force .\build-release -ErrorAction SilentlyContinue
      Remove-Item -Recurse -Force .\dependencies -ErrorAction SilentlyContinue

   The ``dependencies`` directory is regenerated by CMake and contains fetched
   third-party source/build state, not your assignment source code.

5. Configure a debug build with Ninja:

   .. code-block:: powershell

      cmake -S . -B build -G Ninja `
          -DCMAKE_BUILD_TYPE=Debug `
          -DPython_EXECUTABLE="$PWD\.venv-glad\Scripts\python.exe" `
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

   Use ``-G Ninja`` rather than a Visual Studio solution generator for the
   documented Windows workflow. Ninja is a single-configuration generator, so
   ``CMAKE_BUILD_TYPE`` must be specified during configuration. The
   policy-version option matches the configuration exercised by the repository
   CI and protects configuration against policy requirements in fetched CMake
   projects.

   For a release build in a separate output directory:

   .. code-block:: powershell

      cmake -S . -B build-release -G Ninja `
          -DCMAKE_BUILD_TYPE=Release `
          -DPython_EXECUTABLE="$PWD\.venv-glad\Scripts\python.exe" `
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

6. Build all assignments, or build one intended target:

   .. code-block:: powershell

      cmake --build build --parallel
      cmake --build build --target EDAF80_Assignment1 --parallel
      cmake --build build --target EDAN35_Assignment2 --parallel

7. Run an assignment, for example:

   .. code-block:: powershell

      .\build\src\EDAF80\EDAF80_Assignment1.exe
      .\build\src\EDAN35\EDAN35_Assignment2.exe

   The Windows/Linux runtime path requests an OpenGL 4.6 core context. If
   context creation fails, update the graphics driver or change the requested
   version in ``src/core/WindowManager.cpp`` for older hardware.

   .. figure:: images/Assignment1_StartWindow.jpg
      :alt: The Earth rendered on a dark background.

      The first assignment running when launched for the first time.

Troubleshooting on Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~

If CMake reports ``CMake was unable to find a build program corresponding to
"Ninja"``, check that Ninja is available in the developer terminal:

.. code-block:: powershell

   where.exe ninja
   ninja --version

If CMake reports that ``CMAKE_CXX_COMPILER`` or ``CMAKE_C_COMPILER`` is not
set, check that the terminal exposes the Visual Studio compiler:

.. code-block:: powershell

   where.exe cl
   cl

If glad generation fails with a Python/Jinja2 error, confirm that the virtual
environment is functional and reconfigure with its Python executable:

.. code-block:: powershell

   .\.venv-glad\Scripts\python.exe -c "import jinja2; print(jinja2.__version__)"

After correcting the environment, remove the failed ``build`` folder and
configure again.


Using CMake on macOS and Linux
------------------------------

1. Clone or update the ``vs2026`` branch:

   .. code-block:: console

      git clone --branch vs2026 https://github.com/fury471/CG_Labs.git
      cd CG_Labs

2. Create an isolated Python environment for glad generation:

   .. code-block:: console

      python3 -m venv .venv-glad
      .venv-glad/bin/python -m pip install --upgrade pip
      .venv-glad/bin/python -m pip install "Jinja2>=2.7,<4.0"

3. Configure the platform-specific build tree.

   On macOS, generate an Xcode build tree; this runtime path keeps OpenGL 4.1:

   .. code-block:: console

      cmake -S . -B build -G Xcode \
          -DCMAKE_BUILD_TYPE=Debug \
          -DPython_EXECUTABLE="$PWD/.venv-glad/bin/python" \
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

   On Linux, generate a Ninja build tree; this runtime path requests OpenGL 4.6:

   .. code-block:: console

      cmake -S . -B build -G Ninja \
          -DCMAKE_BUILD_TYPE=Debug \
          -DPython_EXECUTABLE="$PWD/.venv-glad/bin/python" \
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

4. Build the source code:

   .. code-block:: console

      cmake --build build --config Debug --parallel

5. Run the intended assignment executable from its generated output location.
   If the application does not open or renders unexpectedly, inspect its
   terminal output and generated ``logs.txt`` file.


.. _Visual Studio: https://visualstudio.microsoft.com/vs/features/cplusplus/
.. _Git: https://git-scm.com/
.. _CMake: https://cmake.org/
.. _Ninja: https://ninja-build.org/
.. _Python: https://www.python.org/
.. _Jinja2: https://palletsprojects.com/projects/jinja/
.. _Xcode: https://apps.apple.com/se/app/xcode/id497799835?mt=12
.. _Clang: https://clang.llvm.org/
.. _GCC: https://gcc.gnu.org/
