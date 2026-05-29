add_library (sfm_sandbox_core STATIC)
target_sources (
	sfm_sandbox_core
	PUBLIC
		[[FrameClock.hpp]]
	PRIVATE
		[[FrameClock.cpp]]
)
target_include_directories (
	sfm_sandbox_core
	PUBLIC
		"${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries (sfm_sandbox_core PUBLIC sfm_sandbox_options)
