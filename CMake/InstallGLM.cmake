# Build the pinned GLM dependency as a header-only target in the main graph.
set (GLM_BUILD_LIBRARY OFF CACHE BOOL "Use GLM as a header-only library" FORCE)
set (GLM_BUILD_TESTS OFF CACHE BOOL "Disable GLM tests" FORCE)
set (GLM_BUILD_INSTALL OFF CACHE BOOL "Use GLM as an embedded dependency" FORCE)
set (GLM_ENABLE_CXX_14 ON CACHE BOOL "Match the framework C++ language level" FORCE)

FetchContent_Declare (
	glm
	GIT_REPOSITORY [[https://github.com/g-truc/glm.git]]
	GIT_TAG "${LUGGCGL_GLM_VERSION}"
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable (glm)
