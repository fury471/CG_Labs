#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace sfm::gfx
{

/// Result of constructing the central renderer pipeline.
///
/// Renderer setup can fail because shader compilation, linkage, or GPU resource
/// configuration can fail. Returning structured messages keeps diagnostics
/// visible in the sandbox UI instead of hiding them behind console-only logs.
struct RendererBuildResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

/// Owns the GPU state needed by the sandbox renderer.
///
/// Milestone 5 turns the renderer into a small 3D visualization viewport: the
/// app provides a camera transform, while the renderer owns grid/axes geometry,
/// shader state and draw submission.
class Renderer final
{
public:
	Renderer() noexcept = default;
	~Renderer() noexcept = default;

	Renderer(Renderer const&) = delete;
	Renderer& operator=(Renderer const&) = delete;
	Renderer(Renderer&&) noexcept = default;
	Renderer& operator=(Renderer&&) noexcept = default;

	/// Builds the first world-space visualization pipeline. Call only after an
	/// OpenGL context is current and before the context is destroyed.
	[[nodiscard]] RendererBuildResult initialise();

	/// Draws the current renderer contents from the supplied camera transform.
	/// `world_to_clip` is usually camera.GetWorldToClipMatrix().
	void render(glm::mat4 const& world_to_clip) const noexcept;

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] GLsizei line_vertex_count() const noexcept { return m_line_vertex_count; }

private:
	ShaderProgram m_program{};
	UniformLocation m_world_to_clip_uniform{};
	VertexArray m_vertex_array{};
	Buffer m_vertex_buffer{};
	GLsizei m_line_vertex_count{ 0 };
	bool m_ready{ false };
};

} // namespace sfm::gfx
