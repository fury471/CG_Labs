|UbuntuBadge| |MacOSBadge| |WindowsBadge|

.. |UbuntuBadge| image:: https://github.com/fury471/CG_Labs/actions/workflows/ubuntu.yml/badge.svg?branch=vs2026
   :target: https://github.com/fury471/CG_Labs/actions/workflows/ubuntu.yml?query=branch%3Avs2026
.. |MacOSBadge| image:: https://github.com/fury471/CG_Labs/actions/workflows/macos.yml/badge.svg?branch=vs2026
   :target: https://github.com/fury471/CG_Labs/actions/workflows/macos.yml?query=branch%3Avs2026
.. |WindowsBadge| image:: https://github.com/fury471/CG_Labs/actions/workflows/windows.yml/badge.svg?branch=vs2026
   :target: https://github.com/fury471/CG_Labs/actions/workflows/windows.yml?query=branch%3Avs2026

CG_Labs contains the source code for the various Computer Graphics courses
taught at `Lund University`_ (EDAF80_ and EDAN35_). All assignments of EDAF80
are available, as well as the second assignment of EDAN35.

The public upstream repository is found at https://github.com/LUGGPublic/CG_Labs,
and its documentation at https://luggpublic.github.io/CG_Labs.

This ``vs2026`` branch modernises the dependency and Windows development setup.
It uses the Visual Studio 2026 MSVC toolchain with CMake and the Ninja generator
on Windows, and requests modern OpenGL contexts where the native platform
supports them. See BUILD.rst_ for configuration and build instructions.

SfM Visualization Sandbox
=========================

The ``feature/sfm-visualization-sandbox`` branch contains a focused SfM
Visualization Sandbox application in addition to the original course lab
programs. The sandbox is a development and learning tool, not a full SfM
reconstruction package.

The sandbox currently supports:

* point clouds;
* camera poses, frustums, trajectories and metadata;
* simple surface inspection through a documented tiny OBJ subset;
* camera-image association through a small ASCII PPM debug path;
* projection-debug image planes;
* render-target, CPU profiling and GPU pass-timing diagnostics;
* an instanced repeated-marker stress scene;
* a small project manifest for startup dataset paths;
* startup validation and one-frame visual baseline capture commands;
* an initial constrained point-cloud-to-mesh builder with OBJ export.

Main sandbox paths:

.. code-block:: text

   src/apps/SfmSandbox
   src/apps/SfmSandbox/SfmSandboxMeshBuilderPanel.*
   src/apps/SfmSandbox/SfmSandboxMeshWorkflow.*
   src/apps/SfmSandbox/SfmSandboxStartup.*
   src/sandbox/core
   src/sandbox/scene
   src/sandbox/gfx
   shaders/sandbox
   res/sandbox
   docs/sandbox

Supported sandbox command-line workflow:

.. code-block:: bat

   git switch feature/sfm-visualization-sandbox
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ctest --test-dir build --output-on-failure

Launch checks:

.. code-block:: bat

   build\src\apps\SfmSandbox\SfmSandbox.exe
   build\src\EDAF80\EDAF80_Assignment1.exe
   build\src\EDAN35\EDAN35_Assignment2.exe

The sandbox starts from ``res/sandbox/default_project.sfmproj`` by default.
Use a different startup manifest with:

.. code-block:: bat

   build\src\apps\SfmSandbox\SfmSandbox.exe --project path\to\project.sfmproj

In a sandbox project manifest, ``point_cloud`` is the required dataset path.
``camera_poses``, ``surface`` and ``image`` are optional; leave them empty to
start those layers disabled.

Validate startup resources and shaders without opening a window:

.. code-block:: bat

   build\src\apps\SfmSandbox\SfmSandbox.exe --validate-install

Capture a default visual regression baseline:

.. code-block:: bat

   build\src\apps\SfmSandbox\SfmSandbox.exe --capture-baseline build\sandbox-captures\sfm-baseline.ppm

Build an OBJ mesh from the active project point cloud without opening a window:

.. code-block:: bat

   build\src\apps\SfmSandbox\SfmSandbox.exe --build-mesh build\sandbox-output\generated_surface.obj

The current builder is a constrained projected triangulation path for small,
surface-like clouds. It is guarded by a default point limit so large clouds
report diagnostics instead of freezing the interactive UI. Generated OBJ files
include provenance comments for the source point cloud, algorithm, projection
and build counters.

Startup parsing, install validation and no-window mesh export are isolated from
the live viewer class in ``SfmSandboxStartup`` so command workflows can be tested
and extended without turning the interactive app into a catch-all module.
Interactive mesh-building state is isolated in ``SfmSandboxMeshWorkflow`` and
the mesh-builder controls live in ``SfmSandboxMeshBuilderPanel``; the viewer app
keeps renderer upload and prior-surface preservation.

Sandbox pull requests use ``.github/workflows/sandbox-ci.yml`` for the Windows
Ninja configure/build/test gate. AI-assisted intermediate commits use
``[skip ci]``; final pull requests should not skip CI.

Important sandbox documents:

* ``docs/sandbox/ROADMAP.md``;
* ``docs/sandbox/ENGINEERING_STANDARDS.md``;
* ``docs/sandbox/project-manifest.md``;
* ``docs/sandbox/point-to-mesh-builder.md``;
* ``docs/sandbox/visual-regression-capture.md``;
* ``docs/sandbox/gpu-timing-telemetry.md``;
* ``docs/sandbox/release-candidate-packaging.md``;
* ``docs/sandbox/release-readiness-m20.md``;
* ``docs/sandbox/resource-license-inventory.md``;
* ``docs/sandbox/rendering-regression-checklist.md``;
* ``docs/sandbox/adr/0003-reconstruction-backend-strategy.md``.

The ``res/sandbox`` samples are synthetic development samples created for the
sandbox. Inherited course resources and third-party dependencies still need
license review before any external release claim.

Dependencies
============

CG_Labs uses pinned dependency versions to keep builds reproducible:

* assimp_ (6.0.5): to load OBJ models;
* CMake_ (>= 3.24): to configure the project and handle dependencies;
* `Dear ImGui`_ (1.92.8): to display information and controls on screen;
* Git_: to retrieve missing dependencies;
* glad_ (2.0.8): to generate and load the OpenGL 4.6 core API;
* GLFW_ (3.4): to create a window and an OpenGL context, as well as handle inputs;
* GLM_ (1.0.3): for linear algebra operations;
* Ninja_: to build with the Visual Studio 2026 toolchain on Windows and in the
  documented Linux workflow;
* Python_ with Jinja2_: required while CMake generates the glad 2 loader;
* stb_: to load image files, retained at the framework's known reproducible revision;
* tinyfiledialogs_: to display dialogue windows, retained at the framework's
  known reproducible revision.

CMake retrieves and builds the pinned dependencies during configuration. For a
clean first configuration, network access is required, and Python with Jinja2
must be available for glad code generation. BUILD.rst_ shows an isolated Python
virtual-environment setup for this prerequisite.

OpenGL support
==============

This branch generates a glad OpenGL 4.6 core-profile loader and uses the
following runtime context policy:

* **Windows and Linux:** request an OpenGL 4.6 core context;
* **macOS:** request an OpenGL 4.1 forward-compatible core context, as this is
  the latest native OpenGL version available on macOS.

Existing assignment shaders remain compatible with the macOS 4.1 path; the
framework does not require GLSL 4.60 shaders merely because a 4.6 context is
available on Windows or Linux. Code using functionality introduced after
OpenGL 4.1 must not be expected to run on macOS.

If hardware or drivers on Windows or Linux do not support OpenGL 4.6, lower the
requested context version in ``src/core/WindowManager.cpp`` or use an older
compatible branch such as the upstream `OpenGL 3.3`_ branch.

The framework and its current dependencies require a C++17-capable compiler.
The Windows instructions use the MSVC compiler provided by Visual Studio 2026.

The different resources used in the assignments, textures and object files, can
be found in `this ZIP file`_, which should be automatically retrieved by CMake.

Configuring and building
========================

All information regarding software configuration, prerequisites, setup and
building the framework can be found in BUILD.rst_. For Windows, create the
repository-local ``.venv-glad`` environment described there **before the first
Visual Studio CMake configure**. The committed ``CMakeSettings.json`` then uses
Ninja, generates IDE builds below ``out\build``, and points glad generation to
that local Python environment. BUILD.rst_ also documents command-line Ninja
builds and troubleshooting for generated ``glad/gl.h`` and Visual Studio cache
failures.

Miscellaneous
=============

If you are on a laptop and would like to force the assignments to run on the
discrete GPU, set the option ``GLFW_USE_HYBRID_HPG`` to ``ON`` using CMake
— either from the CMake GUI or using CMake on the command line.

Licence
=======

* Dear ImGui is released under an MIT licence, see `Dear ImGui's licence`_.
* This project is unlicenced, see `the UNLICENCE file`_.

.. _Lund University: http://www.lu.se/
.. _EDAF80: http://cs.lth.se/edaf80
.. _EDAN35: http://cs.lth.se/edan35
.. _CMake: https://cmake.org/
.. _Git: https://git-scm.com/
.. _GLFW: https://www.glfw.org/
.. _glad: https://github.com/Dav1dde/glad
.. _GLM: https://github.com/g-truc/glm
.. _Dear ImGui: https://github.com/ocornut/imgui
.. _assimp: https://github.com/assimp/assimp
.. _Ninja: https://ninja-build.org/
.. _Python: https://www.python.org/
.. _Jinja2: https://palletsprojects.com/projects/jinja/
.. _stb: https://github.com/nothings/stb
.. _tinyfiledialogs: https://sourceforge.net/projects/tinyfiledialogs/
.. _Dear ImGui's licence: https://github.com/ocornut/imgui/blob/master/LICENSE.txt
.. _OpenGL 3.3: https://github.com/LUGGPublic/CG_Labs/tree/OpenGL_3.3
.. _this ZIP file: http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDAF80_resources.zip
.. _BUILD.rst: BUILD.rst
.. _the UNLICENCE file: UNLICENCE
