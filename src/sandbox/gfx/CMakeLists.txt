add_library (sfm_sandbox_gfx STATIC)
target_sources (
	sfm_sandbox_gfx
	PUBLIC
		[[ClearPass.hpp]]
	PRIVATE
		[[ClearPass.cpp]]
)
target_include_directories (
	sfm_sandbox_gfx
	PUBLIC
		"${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries (
	sfm_sandbox_gfx
	PUBLIC
		sfm_sandbox_options
		external_libs
)
