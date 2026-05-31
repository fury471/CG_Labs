#pragma once

#include "Buffer.hpp"
#include "RenderSubmission.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "sandbox/scene/CameraPose.hpp"
#include "sandbox/scene/PointCloud.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace sfm::gfx
{

struct RendererBuildResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

enum class PointColourMode
{
	Source = 0,
	Height = 1,
	Solid = 2,
};

struct PointCloudRenderSettings final
{
	float point_size{ 5.0f };
	PointColourMode colour_mode{ PointColourMode::Source };
	glm::vec3 solid_colour{ 0.85f, 0.90f, 1.0f };
	bool show_bounds{ true };
};

struct RendererFrameStatistics final
{
	int submitted_items{ 0 };
	int draw_calls{ 0 };
	int program_binds{ 0 };
	int vertex_array_binds{ 0 };
	int line_vertices_drawn{ 0 };
	int point_vertices_drawn{ 0 };
};

class Renderer final
{
public:
	Renderer() noexcept = default;
	~Renderer() noexcept = default;

	Renderer(Renderer const&) = delete;
	Renderer& operator=(Renderer const&) = delete;
	Renderer(Renderer&&) noexcept = default;
	Renderer& operator=(Renderer&&) noexcept = default;

	[[nodiscard]] RendererBuildResult initialise(sfm::scene::PointCloud const& point_cloud,
	                                            sfm::scene::CameraPoseSet const& camera_poses);
	[[nodiscard]] RendererBuildResult reload_point_cloud(sfm::scene::PointCloud const& point_cloud);
	[[nodiscard]] RendererBuildResult reload_camera_poses(sfm::scene::CameraPoseSet const& camera_poses);

	void render(glm::mat4 const& world_to_clip,
	            PointCloudRenderSettings const& point_settings,
	            bool show_camera_poses) const noexcept;

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] GLsizei line_vertex_count() const noexcept { return m_line_vertex_count; }
	[[nodiscard]] GLsizei point_count() const noexcept { return m_point_count; }
	[[nodiscard]] GLsizei camera_line_vertex_count() const noexcept { return m_camera_line_vertex_count; }
	[[nodiscard]] GLsizei bounds_line_vertex_count() const noexcept { return m_bounds_line_vertex_count; }
	[[nodiscard]] bool has_point_cloud_bounds() const noexcept { return m_bounds_ready; }
	[[nodiscard]] RendererFrameStatistics frame_statistics() const noexcept { return m_last_frame_statistics; }

private:
	[[nodiscard]] RendererBuildResult initialise_grid_pipeline();
	[[nodiscard]] RendererBuildResult initialise_point_pipeline_if_needed();
	[[nodiscard]] RendererBuildResult initialise_camera_pose_pipeline(sfm::scene::CameraPoseSet const& camera_poses);
	void draw_submission(RenderSubmission const& submission,
	                     glm::mat4 const& world_to_clip,
	                     RendererFrameStatistics& statistics) const noexcept;

	ShaderProgram m_grid_program{};
	UniformLocation m_grid_world_to_clip_uniform{};
	VertexArray m_grid_vertex_array{};
	Buffer m_grid_vertex_buffer{};
	GLsizei m_line_vertex_count{ 0 };

	ShaderProgram m_point_program{};
	UniformLocation m_point_world_to_clip_uniform{};
	UniformLocation m_point_size_uniform{};
	UniformLocation m_point_colour_mode_uniform{};
	UniformLocation m_point_solid_colour_uniform{};
	VertexArray m_point_vertex_array{};
	Buffer m_point_vertex_buffer{};
	GLsizei m_point_count{ 0 };

	VertexArray m_bounds_vertex_array{};
	Buffer m_bounds_vertex_buffer{};
	GLsizei m_bounds_line_vertex_count{ 0 };
	bool m_bounds_ready{ false };

	VertexArray m_camera_vertex_array{};
	Buffer m_camera_vertex_buffer{};
	GLsizei m_camera_line_vertex_count{ 0 };

	bool m_grid_ready{ false };
	bool m_point_program_ready{ false };
	bool m_camera_pose_ready{ false };
	bool m_ready{ false };
	mutable RendererFrameStatistics m_last_frame_statistics{};
};

} // namespace sfm::gfx
