# Keep tinyfiledialogs at the known reproducible source revision used by this
# framework until an officially versioned upgrade is validated.
FetchContent_Declare (
	tinyfiledialogs
	GIT_REPOSITORY [[https://git.code.sf.net/p/tinyfiledialogs/code]]
	GIT_TAG "${LUGGCGL_TINYFILEDIALOGS_REVISION}"
)
FetchContent_GetProperties (tinyfiledialogs)
if (NOT tinyfiledialogs_POPULATED)
	FetchContent_Populate (tinyfiledialogs)
endif ()

add_library (tinyfiledialogs::tinyfiledialogs INTERFACE IMPORTED)
set_target_properties (tinyfiledialogs::tinyfiledialogs PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${tinyfiledialogs_SOURCE_DIR}"
	INTERFACE_SOURCES "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)
