find_package (assimp QUIET ${LUGGCGL_ASSIMP_MIN_VERSION})
if (NOT assimp_FOUND)
	FetchContent_Declare (
		assimp
		GIT_REPOSITORY [[https://github.com/assimp/assimp.git]]
		GIT_TAG "v${LUGGCGL_ASSIMP_DOWNLOAD_VERSION}"
		GIT_SHALLOW ON
	)

	FetchContent_GetProperties (assimp)
	if (NOT assimp_POPULATED)
		message (STATUS "Cloning assimp…")
		FetchContent_Populate (assimp)
	endif ()

	set (assimp_INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/assimp-install")
	if (NOT EXISTS "${assimp_INSTALL_DIR}")
		file (MAKE_DIRECTORY ${assimp_INSTALL_DIR})
	endif ()

	message (STATUS "Setting up CMake for assimp…")
	# If a previous configure created a CMake cache in a different path (for example on another machine),
	# CMake will refuse to reconfigure in-place. Remove any stale cache/build dir so we start clean.
	if (EXISTS "${assimp_BINARY_DIR}/CMakeCache.txt")
		message (STATUS "Found existing CMake cache in ${assimp_BINARY_DIR}; removing stale build directory to avoid path mismatch.")
		file (REMOVE_RECURSE "${assimp_BINARY_DIR}")
		file (MAKE_DIRECTORY "${assimp_BINARY_DIR}")
	endif ()

	execute_process (
		COMMAND ${CMAKE_COMMAND} -S ${assimp_SOURCE_DIR} -B ${assimp_BINARY_DIR}
				 -DASSIMP_NO_EXPORT=ON
				 -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
				 -DASSIMP_BUILD_ZLIB=ON
				 -DASSIMP_BUILD_TESTS=OFF
				 -DCMAKE_INSTALL_PREFIX=${assimp_INSTALL_DIR}
				 -DCMAKE_BUILD_TYPE=Release
		OUTPUT_VARIABLE stdout
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
	)
	if (result)
		message (FATAL_ERROR "CMake setup for assimp failed: ${result}\n"
							 "Standard output: ${stdout}\n"
							 "Error output: ${stderr}")
	endif ()

	message (STATUS "Building and installing assimp…")
	execute_process (
		COMMAND ${CMAKE_COMMAND} --build ${assimp_BINARY_DIR}
		                         --config Release
		                         --target install
		RESULT_VARIABLE result
	)
	if (result)
		message (FATAL_ERROR "Build step for assimp failed: ${result}\n"
		                     "Standard output: ${stdout}\n"
		                     "Error output: ${stderr}")
	endif ()

	list (APPEND CMAKE_PREFIX_PATH ${assimp_INSTALL_DIR}/lib/cmake)

	set (assimp_INSTALL_DIR)
endif ()
