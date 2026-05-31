add_library (sfm_sandbox_gfx STATIC)
target_sources (
	sfm_sandbox_gfx
	PUBLIC
		[[Buffer.hpp]]
		[[ClearPass.hpp]]
		[[FramebufferCapture.hpp]]
		[[Framebuffer.hpp]]
		[[GpuProfiler.hpp]]
		[[OwnershipProbe.hpp]]
		[[Renderer.hpp]]
		[[Sampler.hpp]]
		[[ShaderProgram.hpp]]
		[[ShaderProgramProbe.hpp]]
		[[Texture2D.hpp]]
		[[VertexArray.hpp]]
	PRIVATE
		[[Buffer.cpp]]
		[[ClearPass.cpp]]
		[[DebugLabel.cpp]]
		[[DebugLabel.hpp]]
		[[FramebufferCapture.cpp]]
		[[Framebuffer.cpp]]
		[[GpuProfiler.cpp]]
		[[OwnershipProbe.cpp]]
		[[Renderer.cpp]]
		[[Sampler.cpp]]
		[[ShaderProgram.cpp]]
		[[ShaderProgramProbe.cpp]]
		[[Texture2D.cpp]]
		[[VertexArray.cpp]]
)
target_include_directories (
	sfm_sandbox_gfx
	PUBLIC
		"${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries (
	sfm_sandbox_gfx
	PUBLIC
		sfm_sandbox_options
		sfm_sandbox_scene
		glad_gl_core_46
		glm::glm
)
