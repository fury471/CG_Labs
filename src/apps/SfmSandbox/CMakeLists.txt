add_executable (SfmSandbox)
target_sources (
	SfmSandbox
	PRIVATE
		[[main.cpp]]
		[[SfmSandboxApp.cpp]]
		[[SfmSandboxApp.hpp]]
)
target_link_libraries (
	SfmSandbox
	PRIVATE
		sfm_sandbox_core
		sfm_sandbox_gfx
		bonobo
		sfm_sandbox_options
)

install (TARGETS SfmSandbox DESTINATION [[bin]])
