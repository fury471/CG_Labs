add_executable (SfmSandbox)
target_sources (
	SfmSandbox
	PRIVATE
		[[main.cpp]]
		[[SfmSandboxApp.cpp]]
		[[SfmSandboxApp.hpp]]
		[[SfmSandboxCaptureMetadata.cpp]]
		[[SfmSandboxCaptureMetadata.hpp]]
		[[SfmSandboxMeshBuilderPanel.cpp]]
		[[SfmSandboxMeshBuilderPanel.hpp]]
		[[SfmSandboxMeshWorkflow.cpp]]
		[[SfmSandboxMeshWorkflow.hpp]]
		[[SfmSandboxPanels.cpp]]
		[[SfmSandboxPanels.hpp]]
		[[SfmSandboxStartup.cpp]]
		[[SfmSandboxStartup.hpp]]
		[[SfmSandboxUi.cpp]]
		[[SfmSandboxUi.hpp]]
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
