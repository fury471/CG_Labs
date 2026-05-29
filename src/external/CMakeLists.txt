add_library (external_libs)

target_sources (
	external_libs
	PUBLIC
		"${imgui_SOURCE_DIR}/imconfig.h"
		"${imgui_SOURCE_DIR}/imgui.h"
		"${imgui_SOURCE_DIR}/imgui_internal.h"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
	PRIVATE
		"${imgui_SOURCE_DIR}/imgui.cpp"
		"${imgui_SOURCE_DIR}/imgui_demo.cpp"
		"${imgui_SOURCE_DIR}/imgui_draw.cpp"
		"${imgui_SOURCE_DIR}/imgui_tables.cpp"
		"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_compile_definitions (
	external_libs
	PRIVATE
		GLFW_INCLUDE_NONE
)

target_include_directories (
	external_libs
	SYSTEM
	PUBLIC
		"${imgui_SOURCE_DIR}"
		"${imgui_SOURCE_DIR}/backends"
)

target_link_libraries (
	external_libs
	PUBLIC
		glad_gl_core_46
	PRIVATE
		glfw
)

install (TARGETS external_libs DESTINATION lib)
