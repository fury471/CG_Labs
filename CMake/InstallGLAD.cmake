# Generate a current OpenGL 4.6 core-profile loader using glad 2.
# The loader is usable with the macOS OpenGL 4.1 runtime context as long as
# application code does not invoke unavailable post-4.1 functionality there.
FetchContent_Declare (
	glad
	GIT_REPOSITORY [[https://github.com/Dav1dde/glad.git]]
	GIT_TAG "${LUGGCGL_GLAD_VERSION}"
	GIT_SHALLOW ON
	SOURCE_SUBDIR [[cmake]]
)
FetchContent_MakeAvailable (glad)
glad_add_library (
	glad_gl_core_46
	STATIC
	REPRODUCIBLE
	API gl:core=4.6
	EXTENSIONS GL_KHR_debug
)
