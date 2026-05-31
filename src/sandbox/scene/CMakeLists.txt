add_library (sfm_sandbox_scene STATIC)
target_sources (
	sfm_sandbox_scene
	PUBLIC
		"CameraPose.hpp"
		"CameraPoseLoader.hpp"
		"ImageImport.hpp"
		"Mesh.hpp"
		"MeshLoader.hpp"
		"PointCloudMesher.hpp"
		"PointCloud.hpp"
		"PointCloudLoader.hpp"
		"SandboxProject.hpp"
		"SurfaceExport.hpp"
		"SurfaceImport.hpp"
	PRIVATE
		"CameraPose.cpp"
		"CameraPoseLoader.cpp"
		"ImageImport.cpp"
		"Mesh.cpp"
		"PointCloudMesher.cpp"
		"PointCloud.cpp"
		"PointCloudLoader.cpp"
		"SandboxProject.cpp"
		"SurfaceExport.cpp"
		"SurfaceImport.cpp"
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
