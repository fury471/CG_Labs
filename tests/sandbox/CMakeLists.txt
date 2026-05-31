add_executable (sfm_sandbox_scene_tests)
target_sources (
	sfm_sandbox_scene_tests
	PRIVATE
		"SceneParserTests.cpp"
)
target_link_libraries (
	sfm_sandbox_scene_tests
	PRIVATE
		sfm_sandbox_options
		sfm_sandbox_scene
)
add_test (NAME sfm_sandbox_scene_tests COMMAND sfm_sandbox_scene_tests)
set_tests_properties (
	sfm_sandbox_scene_tests
	PROPERTIES
		LABELS "sandbox;scene;parsers"
)

add_executable (sfm_surface_import_smoke_test)
target_sources (
	sfm_surface_import_smoke_test
	PRIVATE
		"SurfaceImportSmokeTest.cpp"
)
target_link_libraries (
	sfm_surface_import_smoke_test
	PRIVATE
		sfm_sandbox_options
		sfm_sandbox_scene
)
add_test (NAME sfm_surface_import_smoke_test COMMAND sfm_surface_import_smoke_test)
set_tests_properties (
	sfm_surface_import_smoke_test
	PROPERTIES
		LABELS "sandbox;scene;surface;parsers"
)

add_executable (sfm_image_import_smoke_test)
target_sources (
	sfm_image_import_smoke_test
	PRIVATE
		"ImageImportSmokeTest.cpp"
)
target_link_libraries (
	sfm_image_import_smoke_test
	PRIVATE
		sfm_sandbox_options
		sfm_sandbox_scene
)
add_test (NAME sfm_image_import_smoke_test COMMAND sfm_image_import_smoke_test)
set_tests_properties (
	sfm_image_import_smoke_test
	PROPERTIES
		LABELS "sandbox;scene;image;parsers"
)

add_executable (sfm_point_cloud_mesher_tests)
target_sources (
	sfm_point_cloud_mesher_tests
	PRIVATE
		"PointCloudMesherTests.cpp"
)
target_link_libraries (
	sfm_point_cloud_mesher_tests
	PRIVATE
		sfm_sandbox_options
		sfm_sandbox_scene
)
add_test (NAME sfm_point_cloud_mesher_tests COMMAND sfm_point_cloud_mesher_tests)
set_tests_properties (
	sfm_point_cloud_mesher_tests
	PROPERTIES
	LABELS "sandbox;scene;meshing;export"
)

add_executable (sfm_sandbox_mesh_workflow_tests)
target_sources (
	sfm_sandbox_mesh_workflow_tests
	PRIVATE
		"../../src/apps/SfmSandbox/SfmSandboxMeshWorkflow.cpp"
		"SfmSandboxMeshWorkflowTests.cpp"
)
target_include_directories (
	sfm_sandbox_mesh_workflow_tests
	PRIVATE
		"${PROJECT_SOURCE_DIR}/src/apps/SfmSandbox"
)
target_link_libraries (
	sfm_sandbox_mesh_workflow_tests
	PRIVATE
		sfm_sandbox_options
		sfm_sandbox_scene
)
add_test (NAME sfm_sandbox_mesh_workflow_tests COMMAND sfm_sandbox_mesh_workflow_tests)
set_tests_properties (
	sfm_sandbox_mesh_workflow_tests
	PROPERTIES
		LABELS "sandbox;app;meshing;workflow"
)

add_test (NAME sfm_sandbox_startup_validation COMMAND SfmSandbox --validate-install)
set_tests_properties (
	sfm_sandbox_startup_validation
	PROPERTIES
		LABELS "sandbox;startup;packaging"
)

add_test (
	NAME sfm_sandbox_mesh_build_cli
	COMMAND SfmSandbox --build-mesh "${CMAKE_BINARY_DIR}/sandbox-output/ctest_generated_surface.obj"
)
set_tests_properties (
	sfm_sandbox_mesh_build_cli
	PROPERTIES
		LABELS "sandbox;meshing;cli;export"
)
