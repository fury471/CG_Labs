# Keep tinyfiledialogs at the known reproducible source revision used by this
# framework until an officially versioned upgrade is validated.
FetchContent_Declare (
	tinyfiledialogs
	GIT_REPOSITORY [[https://git.code.sf.net/p/tinyfiledialogs/code]]
	GIT_TAG "${LUGGCGL_TINYFILEDIALOGS_REVISION}"
	SOURCE_SUBDIR [[cmake-fetchcontent-noop]]
)
FetchContent_MakeAvailable (tinyfiledialogs)

add_library (tinyfiledialogs::tinyfiledialogs INTERFACE IMPORTED)
set_target_properties (tinyfiledialogs::tinyfiledialogs PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${tinyfiledialogs_SOURCE_DIR}"
	INTERFACE_SOURCES "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)
