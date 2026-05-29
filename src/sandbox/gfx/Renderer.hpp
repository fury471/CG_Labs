#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"

#include <string>
#include <vector>

namespace sfm::gfx
{

/// Result of constructing the first central renderer pipeline.
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
/// Milestone 4 deliberately keeps the renderer small: one triangle pipeline and
/// one draw call. The important architectural step is that application code now
/// asks a renderer to draw instead of directly issuing draw-call setup itself.
class Renderer final
{
public:
	Renderer() noexcept = default;
	~Renderer() noexcept = default;

	Renderer(Renderer const&) = delete;
	Renderer& operator=(Renderer const&) = delete;
	Renderer(Renderer&&) noexcept = default;
	Renderer& operator=(Renderer&&) noexcept = default;

	/// Builds the first visible GPU pipeline. Call only after an OpenGL context is
	/// current and before the context is destroyed.
	[[nodiscard]] RendererBuildResult initialise();

	/// Draws the current renderer contents. The app owns the frame lifecycle;
	/// Renderer owns the GPU pipeline and draw submission.
	void render() const noexcept;

	[[nodiscard]] bool ready() const noexcept { return m_ready; }

private:
	ShaderProgram m_program{};
	VertexArray m_vertex_array{};
	Buffer m_vertex_buffer{};
	bool m_ready{ false };
};

} // namespace sfm::gfx
