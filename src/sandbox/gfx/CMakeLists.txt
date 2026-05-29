add_library (sfm_sandbox_gfx STATIC)
target_sources (
	sfm_sandbox_gfx
	PUBLIC
		[[Buffer.hpp]]
		[[ClearPass.hpp]]
		[[Framebuffer.hpp]]
		[[OwnershipProbe.hpp]]
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
		[[Framebuffer.cpp]]
		[[OwnershipProbe.cpp]]
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
		glad_gl_core_46
		glm::glm
)
