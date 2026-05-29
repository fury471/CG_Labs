# Dear ImGui does not provide a root CMake project target; populate its tagged
# sources here and compile the GLFW/OpenGL3 backends in src/external.
FetchContent_Declare (
	imgui
	GIT_REPOSITORY [[https://github.com/ocornut/imgui.git]]
	GIT_TAG "${LUGGCGL_IMGUI_VERSION}"
	GIT_SHALLOW ON
)
FetchContent_GetProperties (imgui)
if (NOT imgui_POPULATED)
	FetchContent_Populate (imgui)
endif ()
