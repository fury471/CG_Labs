#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "sandbox/scene/PointCloud.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace sfm::gfx
{

/// Result of constructing or updating renderer GPU state.
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
/// Milestone 8 adds runtime point-cloud reload. The renderer keeps the persistent
/// grid/axes pipeline alive and replaces the point-cloud GPU resources whenever
/// the application provides a newly loaded `PointCloud`.
class Renderer final
{
public:
	Renderer() noexcept = default;
	~Renderer() noexcept = default;

	Renderer(Renderer const&) = delete;
	Renderer& operator=(Renderer const&) = delete;
	Renderer(Renderer&&) noexcept = default;
	Renderer& operator=(Renderer&&) noexcept = default;

	/// Builds the world-space visualization pipelines. Call only after an OpenGL
	/// context is current and before the context is destroyed.
	[[nodiscard]] RendererBuildResult initialise(sfm::scene::PointCloud const& point_cloud);

	/// Replaces the GPU point-cloud resources while preserving the grid/axes
	/// resources. This method creates fresh immutable buffers instead of trying to
	/// reuse storage allocated with glNamedBufferStorage.
	[[nodiscard]] RendererBuildResult reload_point_cloud(sfm::scene::PointCloud const& point_cloud);

	/// Draws the current renderer contents from the supplied camera transform.
	/// `world_to_clip` is usually camera.GetWorldToClipMatrix().
	void render(glm::mat4 const& world_to_clip) const noexcept;

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] GLsizei line_vertex_count() const noexcept { return m_line_vertex_count; }
	[[nodiscard]] GLsizei point_count() const noexcept { return m_point_count; }

private:
	[[nodiscard]] RendererBuildResult initialise_grid_pipeline();
	[[nodiscard]] RendererBuildResult initialise_point_pipeline_if_needed();

	ShaderProgram m_grid_program{};
	UniformLocation m_grid_world_to_clip_uniform{};
	VertexArray m_grid_vertex_array{};
	Buffer m_grid_vertex_buffer{};
	GLsizei m_line_vertex_count{ 0 };

	ShaderProgram m_point_program{};
	UniformLocation m_point_world_to_clip_uniform{};
	UniformLocation m_point_size_uniform{};
	VertexArray m_point_vertex_array{};
	Buffer m_point_vertex_buffer{};
	GLsizei m_point_count{ 0 };

	bool m_grid_ready{ false };
	bool m_point_program_ready{ false };
	bool m_ready{ false };
};

} // namespace sfm::gfx
