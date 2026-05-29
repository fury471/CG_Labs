# Shared compile policy for new SfM Visualization Sandbox code only.
# Legacy assignment targets deliberately remain on their existing configuration.
add_library (sfm_sandbox_options INTERFACE)
target_link_libraries (sfm_sandbox_options INTERFACE CG_Labs_options)
target_compile_features (sfm_sandbox_options INTERFACE cxx_std_20)
target_compile_options (
	sfm_sandbox_options
	INTERFACE
		$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4>
		$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->
		$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall>
		$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wextra>
		$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wpedantic>
)

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/core")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/gfx")
