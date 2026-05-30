add_library (sfm_sandbox_scene STATIC)
target_sources (
	sfm_sandbox_scene
	PUBLIC
		[[PointCloud.hpp]]
	PRIVATE
		[[PointCloud.cpp]]
)
target_include_directories (
	sfm_sandbox_scene
	PUBLIC
		"${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries (
	sfm_sandbox_scene
	PUBLIC
		sfm_sandbox_options
		glm::glm
)
