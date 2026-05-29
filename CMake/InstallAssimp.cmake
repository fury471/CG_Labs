# Build the pinned Assimp dependency as part of the main CMake graph.
# Assimp 6.0.5 itself requires C++17 and CMake 3.22 or newer.
set (BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies as static libraries" FORCE)
set (ASSIMP_NO_EXPORT ON CACHE BOOL "Disable model export functionality" FORCE)
set (ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable Assimp tools" FORCE)
set (ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Disable Assimp samples" FORCE)
set (ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests" FORCE)
set (ASSIMP_BUILD_DOCS OFF CACHE BOOL "Disable Assimp documentation" FORCE)
set (ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Do not promote dependency warnings to errors" FORCE)
set (ASSIMP_INSTALL OFF CACHE BOOL "Use Assimp as an embedded dependency" FORCE)
set (ASSIMP_IGNORE_GIT_HASH ON CACHE BOOL "Avoid probing dependency git state" FORCE)

FetchContent_Declare (
	assimp
	GIT_REPOSITORY [[https://github.com/assimp/assimp.git]]
	GIT_TAG "v${LUGGCGL_ASSIMP_VERSION}"
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable (assimp)

if (TARGET assimp AND NOT TARGET assimp::assimp)
	add_library (assimp::assimp ALIAS assimp)
endif ()
