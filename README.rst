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
