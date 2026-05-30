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
