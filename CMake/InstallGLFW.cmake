# Build the pinned GLFW dependency as part of the main CMake graph.
set (GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW documentation" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples" FORCE)
set (GLFW_INSTALL OFF CACHE BOOL "Use GLFW as an embedded dependency" FORCE)
set (GLFW_LIBRARY_TYPE STATIC CACHE STRING "Link GLFW statically" FORCE)

# The assignments only require one Linux window-system backend; disabling
# Wayland avoids pulling additional protocol-generation packages into CI.
if (UNIX AND NOT APPLE)
	set (GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 backend" FORCE)
	set (GLFW_BUILD_WAYLAND OFF CACHE BOOL "Do not build GLFW Wayland backend" FORCE)
endif ()

FetchContent_Declare (
	glfw
	GIT_REPOSITORY [[https://github.com/glfw/glfw.git]]
	GIT_TAG "${LUGGCGL_GLFW_VERSION}"
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable (glfw)
