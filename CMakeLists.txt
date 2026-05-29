cmake_minimum_required (VERSION 3.24)

# Workaround for the lab computers
if (WIN32)
	set (LUGGCGL_TMP_DIR ${CMAKE_SOURCE_DIR}/../Temp)
	if (NOT EXISTS ${LUGGCGL_TMP_DIR})
		file (MAKE_DIRECTORY ${LUGGCGL_TMP_DIR})
	endif ()
	set (ENV{TMP} ${LUGGCGL_TMP_DIR})
endif ()

# Set project attributes.
project (
	CG_Labs
	VERSION 2026.1
	DESCRIPTION [[Repository for Computer Graphics courses EDAF80 and EDAN35 at Lund University, Sweden.]]
	HOMEPAGE_URL [[https://github.com/LUGGPublic/CG_Labs/]]
	LANGUAGES CXX C
)


# Set build type
set (CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
if (NOT CMAKE_BUILD_TYPE)
	set (
		CMAKE_BUILD_TYPE
		"Release"
		CACHE STRING
		"Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
		FORCE
	)
endif (NOT CMAKE_BUILD_TYPE)
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})


# Retrieve deterministic, tagged dependency versions.
find_package (Git REQUIRED)
include (FetchContent)
set (FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/dependencies" CACHE PATH "Base folder where dependencies are processed." FORCE)

if (NOT EXISTS "${FETCHCONTENT_BASE_DIR}")
	file (MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}")
endif ()

# assimp is used for loading object models.
set (LUGGCGL_ASSIMP_VERSION 6.0.5)
include (CMake/InstallAssimp.cmake)

# GLFW is used for inputs, windows and OpenGL context creation.
set (LUGGCGL_GLFW_VERSION 3.4)
include (CMake/InstallGLFW.cmake)

# GLM is used for matrices, vectors and camera handling.
set (LUGGCGL_GLM_VERSION 1.0.3)
include (CMake/InstallGLM.cmake)

# Dear ImGui is used for on-screen debugging and control widgets.
set (LUGGCGL_IMGUI_VERSION v1.92.8)
include (CMake/InstallImGui.cmake)

# glad 2 supplies the generated OpenGL 4.6 core-profile loader.
set (LUGGCGL_GLAD_VERSION v2.0.8)
include (CMake/InstallGLAD.cmake)

# TinyFileDialogs is used for displaying error popups.
set (LUGGCGL_TINYFILEDIALOGS_REVISION 4a8dbd)
include (CMake/InstallTinyFileDialogs.cmake)

# stb is used for loading image files. It has no tagged releases, so pin the
# latest upstream code revision available at the time of this modernization.
set (LUGGCGL_STB_REVISION 904aa67e1e2d1dec92959df63e700b166d5c1022)
include (CMake/InstallSTB.cmake)

# Resources are found in an external archive.
include (CMake/RetrieveResourceArchive.cmake)


# Configure C++ environment variables.
set (MSAA_RATE "1" CACHE STRING "Window MSAA rate")
set (WIDTH "1600" CACHE STRING "Window width")
set (HEIGHT "900" CACHE STRING "Window height")
set (ROOT_DIR "${PROJECT_SOURCE_DIR}")
configure_file ("${PROJECT_SOURCE_DIR}/src/core/config.hpp.in" "${PROJECT_BINARY_DIR}/config.hpp")


# Set up Doxygen documentation generation
option (LUGGCGL_BUILD_DOCUMENTATION "Build documentation for Lund University Computer Graphics Labs" OFF)
if (LUGGCGL_BUILD_DOCUMENTATION)
	find_package (Doxygen REQUIRED)
	set (LUGGCGL_VERSION_NUMBER "${CMAKE_PROJECT_VERSION}")
	set (LUGGCGL_DOC_INPUT "${CMAKE_SOURCE_DIR}/src")
	set (LUGGCGL_DOC_EXCLUDE ${CMAKE_SOURCE_DIR}/src/external)
	set (LUGGCGL_DOC_OUTPUT "${CMAKE_BINARY_DIR}/doc")
	if (NOT EXISTS "${LUGGCGL_DOC_OUTPUT}")
		file (MAKE_DIRECTORY "${LUGGCGL_DOC_OUTPUT}")
	endif ()
	configure_file ("${CMAKE_SOURCE_DIR}/Doxyfile.in" "${CMAKE_BINARY_DIR}/doc/Doxyfile" @ONLY)
	add_custom_target (
		doc
		COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/doc/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif ()


# Define a common C++ configuration target.
add_library (CG_Labs_options INTERFACE)
target_compile_definitions(
	CG_Labs_options
	INTERFACE
		$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:NOMINMAX>
)
target_compile_options (
	CG_Labs_options
	INTERFACE
		$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/utf-8;/Zc:__cplusplus>
)
target_compile_features (CG_Labs_options INTERFACE cxx_std_14)


# Define another interface library that provides a common setup for all
# assignments.
add_library (assignment_setup INTERFACE)
target_link_libraries (
	assignment_setup
	INTERFACE
		bonobo
		CG_Labs_options
		tinyfiledialogs::tinyfiledialogs
)


# assimp is fetched as a static dependency in this branch, so no runtime DLL
# needs to be copied next to each assignment executable.
function (copy_dlls target target_folder)
endfunction ()


add_subdirectory ("${CMAKE_SOURCE_DIR}/src/external")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/core")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAF80")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAN35")

install (DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/res DESTINATION bin)
