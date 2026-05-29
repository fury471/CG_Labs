Table of Content
================

1. `Setting up the software stack`_

   a. `On Windows`_
   b. `On macOS`_
   c. `On Linux`_

2. `Getting started`_

   a. `Visual Studio 2026 on Windows: using Ninja`_
   b. `Using CMake for other setups`_


Setting up the software stack
=============================

.. note::
   The Windows instructions in this ``vs2026`` branch use Visual Studio 2026
   as the MSVC toolchain provider and Ninja as the CMake generator. Older build
   directories generated using Visual Studio 2019/2022 or another generator
   should not be reused.


On Windows
----------

This branch is configured for Microsoft’s `Visual Studio`_ 2026. When
installing Visual Studio, make sure to check the *Desktop development with C++*
workload (in the *Workloads* tab, under the *Desktop & Mobile* group) and to
have the following individual components enabled:

* the current MSVC C++ x64/x86 build tools for Visual Studio 2026;
* a Windows SDK;
* C++ CMake tools for Windows;
* Ninja, either provided by the Visual Studio CMake tooling or otherwise
  available in the developer terminal.

You will also need to install Git_; you can leave all options at their default
values during the installation.

The instructions below use **Developer PowerShell for VS 2026**, which makes
the MSVC compiler available to CMake while Ninja supplies the actual build
backend. Before configuring the project, verify the required commands:

.. code-block:: powershell

   where.exe cl
   where.exe cmake
   where.exe ninja
   where.exe git

   cl
   cmake --version
   ninja --version
   git --version

All four tools must be available from the same developer terminal session.

.. figure:: images/VS2019_Components.jpg
   :alt: Select the Desktop development with C++ workload and ensure that an
         MSVC compiler toolset, Windows SDK, and CMake tools for Windows are
         enabled. The displayed screenshot is from an older Visual Studio
         version, but the corresponding component categories remain relevant.

   Historical component-selection example; choose the corresponding Visual
   Studio 2026 components in the current installer.


On macOS
--------

We recommend using Apple’s Xcode_, which can be installed from the Mac App
Store, but you are free to use other tools though the instructions provided
here might not be sufficient or applicable as a result. Once the installation
is done, you will need to run in a terminal ``xcode-select --install`` to get
the command line tools installed.

You will additionally need to install CMake_.

Once you open the project file in Xcode, switch Scheme to EDAF80_Assignment1, under the Product directory.

On Linux
--------

You are free to use whichever compiler you want (the code has been tested on
both Clang_ and GCC_) as long as it supports C++14. In addition to a
C++14-capable compiler, you will need to install CMake_ and Git_; prefer the
versions shipped by your distribution, as long as CMake 3.13 is available.


Getting started
===============

.. note::
   All the files and folders are specified relative to the root folder (the
   folder containing among others this file, “README.rst”, the “src/” and
   “shaders/” folder), unless specified otherwise.

We present two different approaches for setting up the assignment code. On
Windows, follow the Visual Studio 2026 + Ninja instructions below. The general
CMake section remains applicable for other platforms and toolchains.


Visual Studio 2026 on Windows: using Ninja
------------------------------------------

Here is the recommended workflow for this ``vs2026`` branch:

1. Clone this fork and check out the ``vs2026`` branch:

   .. code-block:: powershell

      git clone --branch vs2026 https://github.com/fury471/CG_Labs.git
      cd CG_Labs

   If you already cloned the repository, instead run:

   .. code-block:: powershell

      git fetch origin
      git switch vs2026

2. Open **Developer PowerShell for VS 2026** in the repository root. Do not use
   a terminal session in which ``cl.exe`` or ``ninja.exe`` cannot be found.

3. If you have already configured this repository using Visual Studio 2019,
   Visual Studio 2022, a Visual Studio solution generator, or an unsuccessful
   CMake run, remove old generated files before configuring again:

   .. code-block:: powershell

      Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue

      Get-ChildItem .\dependencies -Directory -ErrorAction SilentlyContinue |
          Where-Object { $_.Name -match '(-build|-subbuild)$' } |
          Remove-Item -Recurse -Force

4. Configure the framework using Ninja. For a debug build, run:

   .. code-block:: powershell

      cmake -S . -B build -G Ninja `
          -DCMAKE_BUILD_TYPE=Debug `
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

   This branch uses ``-G Ninja`` rather than a Visual Studio solution
   generator. Ninja is a single-configuration generator, so
   ``CMAKE_BUILD_TYPE`` must be set when configuring. The policy-version option
   accommodates older downloaded dependency CMake scripts when they are
   configured with newer CMake releases.

   For a release build, use a separate output directory:

   .. code-block:: powershell

      cmake -S . -B build-release -G Ninja `
          -DCMAKE_BUILD_TYPE=Release `
          "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

5. Build the source code. To build all targets in the debug build directory:

   .. code-block:: powershell

      cmake --build build --parallel

   To build a specific assignment only, for example:

   .. code-block:: powershell

      cmake --build build --target EDAF80_Assignment1 --parallel
      cmake --build build --target EDAN35_Assignment2 --parallel

6. Run an assignment. With the debug build folder used above, for example:

   .. code-block:: powershell

      .\build\src\EDAF80\EDAF80_Assignment1.exe
      .\build\src\EDAN35\EDAN35_Assignment2.exe

   In case you do not get a window or its content looks different, inspect the
   program output and the log file found below the corresponding build/course
   directory, for example ``build/src/EDAF80/logs.txt`` when running an EDAF80
   assignment from its generated output folder.

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
set, check that the terminal provides the Visual Studio compiler:

.. code-block:: powershell

   where.exe cl
   cl

After correcting the toolchain environment, remove the failed ``build``
directory and configure the project again.


Using CMake for other setups
----------------------------

Here, you will use CMake directly to generate the project files. The precise
generator depends on your platform and installed toolchain.

1. Download, or clone, the framework from the GitHub repository at
   https://github.com/fury471/CG_Labs; the resulting folder will be later
   referred to as “code”, but you are free to use whatever name you want. For
   the Visual Studio 2026 configuration described by this branch, check out
   ``vs2026``.

2. In the folder containing “code”, create a new folder named “build” that will
   contain all the binaries and object files.

3. Generate the project or build files specific to your setup. This can either
   be done via the command line or via a GUI.

   **If you are using the command line**, run ``cmake -G <GENERATOR> -S “code”
   -B <BUILD>`` where “<BUILD>” refers to the build folder created in step 2,
   and “<GENERATOR>” is one of the generators supported by CMake, such as
   “Xcode”, “Unix Makefiles”, or “Ninja”; the full list of supported generators
   can be found at `cmake-generators(7)`_.

   **If you prefer to use the GUI**, follow these steps instead:

   a. Launch the CMake GUI tool, and fill in the paths to the source and binary
      folders.

   b. Now press the “Configure” button (found towards the bottom of the GUI),
      and select the generator you want to use. For the Windows setup of this
      branch, select “Ninja” while working from a Visual Studio 2026 developer
      environment; on macOS, “Xcode” may be used, and on Linux, “Ninja” or
      “Unix Makefiles” may be used.

   c. Once the configuration is done, CMake variables and their values will
      appear above the previously mentioned “Configure” button, highlighted in
      red meaning their value changed since the last configuration. If you wish
      to change the window resolution, or some other variables, go ahead and do
      it. Either way, press the “Configure” button once more to remove the
      highlighting from all variables, and then press the “Generate” button to
      its right, to generate the project files; you will find them in the
      “build” folder you specified.

4. Build the source code using your IDE, or via the command line by running
   ``cmake --build <BUILD>`` where “<BUILD>” is the path to your build folder.

5. If you use an IDE such as Xcode, change which assignment is started when
   running the project as appropriate. With the Windows Ninja command-line
   setup above, run the intended executable from its generated course folder.

6. Run the first assignment: you should be greeted by a window looking like the
   figure below.

   In case you do not get a window or its content looks different, look at the
   logs which you can find in “<BUILD>/src/EDAF80/logs.txt” (replace
   *<BUILD>* by the path to your build folder); if you manually started
   the assignment from the command line, the file will be found in the current
   working directory.

   .. figure:: images/Assignment1_StartWindow.jpg
      :alt: The Earth rendered on a dark background.

      The first assignment running when launched for the first time.


.. _Visual Studio: https://visualstudio.microsoft.com/vs/features/cplusplus/
.. _Git: https://git-scm.com/
.. _CMake: https://cmake.org/
.. _Xcode: https://apps.apple.com/se/app/xcode/id497799835?mt=12
.. _Clang: https://clang.llvm.org/
.. _GCC: http://gcc.gnu.org/
.. _cmake-generators(7): https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
