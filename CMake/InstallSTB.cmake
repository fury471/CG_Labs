# Keep stb at the known reproducible source revision used by this framework.
find_package (Stb QUIET)
if (NOT Stb_FOUND)
	FetchContent_Declare (
		stb
		GIT_REPOSITORY [[https://github.com/nothings/stb]]
		GIT_TAG "${LUGGCGL_STB_REVISION}"
	)

	FetchContent_GetProperties (stb)
	if (NOT stb_POPULATED)
		FetchContent_Populate (stb)
	endif ()

	add_library (stb::stb INTERFACE IMPORTED)
	set_target_properties (stb::stb PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${stb_SOURCE_DIR}"
		INTERFACE_SOURCES "${CMAKE_SOURCE_DIR}/src/core/stb_impl.c"
	)
else ()
	# vcpkg has its own FindStb.cmake which defines Stb_FOUND and
	# Stb_INCLUDE_DIR, but does not define a readily linkable library.
	add_library (stb::stb INTERFACE IMPORTED)
	set_target_properties (stb::stb PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${Stb_INCLUDE_DIR}"
		INTERFACE_SOURCES "${CMAKE_SOURCE_DIR}/src/core/stb_impl.c"
	)
endif ()
