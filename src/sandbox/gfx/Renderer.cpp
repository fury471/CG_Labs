#include "Renderer.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace sfm::gfx
{
namespace
{

struct DebugVertex final
{
	float position[3];
	float colour[3];
};

struct PointVertex final
{
	float position[3];
	float colour[3];
};

constexpr float kGridExtent = 10.0f;
constexpr int kGridHalfLineCount = 10;
constexpr std::size_t kAxisVertexCount = 6u;

constexpr std::array<DebugVertex, kAxisVertexCount> kAxisVertices{
	DebugVertex{ { -kGridExtent, 0.0f, 0.0f }, { 0.95f, 0.20f, 0.20f } },
	DebugVertex{ {  kGridExtent, 0.0f, 0.0f }, { 0.95f, 0.20f, 0.20f } },
	DebugVertex{ { 0.0f, -kGridExtent, 0.0f }, { 0.25f, 0.85f, 0.25f } },
	DebugVertex{ { 0.0f,  kGridExtent, 0.0f }, { 0.25f, 0.85f, 0.25f } },
	DebugVertex{ { 0.0f, 0.0f, -kGridExtent }, { 0.25f, 0.50f, 1.00f } },
	DebugVertex{ { 0.0f, 0.0f,  kGridExtent }, { 0.25f, 0.50f, 1.00f } },
};

constexpr std::string_view kLineVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
uniform mat4 u_world_to_clip;
out vec3 v_colour;
void main()
{
	v_colour = in_colour;
	gl_Position = u_world_to_clip * vec4(in_position, 1.0);
}
)glsl";

constexpr std::string_view kLineFragmentShader = R"glsl(
#version 460 core
in vec3 v_colour;
layout(location = 0) out vec4 out_colour;
void main()
{
	out_colour = vec4(v_colour, 1.0);
}
)glsl";

constexpr std::string_view kPointVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
uniform mat4 u_world_to_clip;
uniform float u_point_size;
uniform float u_colour_mode;
uniform vec3 u_solid_colour;
out vec3 v_colour;
void main()
{
	vec3 height_colour = mix(vec3(0.15, 0.45, 1.0), vec3(1.0, 0.9, 0.15), clamp(in_position.y * 0.5, 0.0, 1.0));
	if (u_colour_mode < 0.5)
		v_colour = in_colour;
	else if (u_colour_mode < 1.5)
		v_colour = height_colour;
	else
		v_colour = u_solid_colour;
	gl_Position = u_world_to_clip * vec4(in_position, 1.0);
	gl_PointSize = u_point_size;
}
)glsl";

constexpr std::string_view kPointFragmentShader = R"glsl(
#version 460 core
in vec3 v_colour;
layout(location = 0) out vec4 out_colour;
void main()
{
	out_colour = vec4(v_colour, 1.0);
}
)glsl";

void add_message(RendererBuildResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

void append_messages(RendererBuildResult& destination, RendererBuildResult const& source)
{
	destination.messages.insert(destination.messages.end(), source.messages.begin(), source.messages.end());
}

DebugVertex make_debug_vertex(glm::vec3 const& position, glm::vec3 const& colour)
{
	return DebugVertex{ { position.x, position.y, position.z }, { colour.r, colour.g, colour.b } };
}

void add_line(std::vector<DebugVertex>& vertices, glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& colour)
{
	vertices.push_back(make_debug_vertex(a, colour));
	vertices.push_back(make_debug_vertex(b, colour));
}

std::vector<DebugVertex> build_grid_and_axes_vertices()
{
	std::vector<DebugVertex> vertices;
	vertices.reserve(static_cast<std::size_t>((kGridHalfLineCount * 2 + 1) * 4) + kAxisVertexCount);
	constexpr float grid_colour[3]{ 0.32f, 0.36f, 0.42f };
	for (int i = -kGridHalfLineCount; i <= kGridHalfLineCount; ++i) {
		float const coordinate = static_cast<float>(i);
		vertices.push_back(DebugVertex{ { -kGridExtent, 0.0f, coordinate }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ {  kGridExtent, 0.0f, coordinate }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ { coordinate, 0.0f, -kGridExtent }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ { coordinate, 0.0f,  kGridExtent }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
	}
	vertices.insert(vertices.end(), kAxisVertices.begin(), kAxisVertices.end());
	return vertices;
}

std::vector<PointVertex> build_point_vertices(sfm::scene::PointCloud const& point_cloud)
{
	std::vector<PointVertex> vertices;
	vertices.reserve(point_cloud.size());
	for (sfm::scene::PointSample const& point : point_cloud.points()) {
		vertices.push_back(PointVertex{
			{ point.position.x, point.position.y, point.position.z },
			{ point.colour.x, point.colour.y, point.colour.z }
		});
	}
	return vertices;
}

std::vector<DebugVertex> build_bounds_vertices(sfm::scene::PointCloudStatistics const& statistics)
{
	std::vector<DebugVertex> vertices;
	if (!statistics.has_bounds)
		return vertices;

	glm::vec3 const mn = statistics.bounds_min;
	glm::vec3 const mx = statistics.bounds_max;
	std::array<glm::vec3, 8> const corners{
		glm::vec3{ mn.x, mn.y, mn.z }, glm::vec3{ mx.x, mn.y, mn.z },
		glm::vec3{ mx.x, mn.y, mx.z }, glm::vec3{ mn.x, mn.y, mx.z },
		glm::vec3{ mn.x, mx.y, mn.z }, glm::vec3{ mx.x, mx.y, mn.z },
		glm::vec3{ mx.x, mx.y, mx.z }, glm::vec3{ mn.x, mx.y, mx.z },
	};
	glm::vec3 const colour{ 1.0f, 0.95f, 0.25f };
	vertices.reserve(24u);
	add_line(vertices, corners[0], corners[1], colour);
	add_line(vertices, corners[1], corners[2], colour);
	add_line(vertices, corners[2], corners[3], colour);
	add_line(vertices, corners[3], corners[0], colour);
	add_line(vertices, corners[4], corners[5], colour);
	add_line(vertices, corners[5], corners[6], colour);
	add_line(vertices, corners[6], corners[7], colour);
	add_line(vertices, corners[7], corners[4], colour);
	add_line(vertices, corners[0], corners[4], colour);
	add_line(vertices, corners[1], corners[5], colour);
	add_line(vertices, corners[2], corners[6], colour);
	add_line(vertices, corners[3], corners[7], colour);
	return vertices;
}

glm::vec3 transform_point(glm::mat4 const& matrix, glm::vec3 const& point)
{
	glm::vec4 const transformed = matrix * glm::vec4{ point, 1.0f };
	return glm::vec3{ transformed } / transformed.w;
}

std::vector<DebugVertex> build_camera_pose_vertices(sfm::scene::CameraPoseSet const& camera_poses)
{
	std::vector<DebugVertex> vertices;
	if (camera_poses.empty())
		return vertices;

	constexpr std::size_t line_vertices_per_camera = 18u;
	std::size_t const trajectory_vertices = camera_poses.size() > 1u ? (camera_poses.size() - 1u) * 2u : 0u;
	vertices.reserve(camera_poses.size() * line_vertices_per_camera + trajectory_vertices);

	constexpr float near_depth = -0.45f;
	constexpr float half_width = 0.36f;
	constexpr float half_height = 0.24f;
	glm::vec3 const local_origin{ 0.0f, 0.0f, 0.0f };
	std::array<glm::vec3, 4> const local_corners{
		glm::vec3{ -half_width, -half_height, near_depth },
		glm::vec3{  half_width, -half_height, near_depth },
		glm::vec3{  half_width,  half_height, near_depth },
		glm::vec3{ -half_width,  half_height, near_depth },
	};

	glm::vec3 previous_position{};
	bool has_previous = false;
	glm::vec3 const trajectory_colour{ 1.0f, 0.95f, 0.15f };

	for (sfm::scene::CameraPose const& pose : camera_poses.poses()) {
		glm::vec3 const origin = transform_point(pose.camera_to_world, local_origin);
		std::array<glm::vec3, 4> corners{};
		for (std::size_t i = 0; i < local_corners.size(); ++i)
			corners[i] = transform_point(pose.camera_to_world, local_corners[i]);

		for (glm::vec3 const& corner : corners)
			add_line(vertices, origin, corner, pose.colour);
		add_line(vertices, corners[0], corners[1], pose.colour);
		add_line(vertices, corners[1], corners[2], pose.colour);
		add_line(vertices, corners[2], corners[3], pose.colour);
		add_line(vertices, corners[3], corners[0], pose.colour);
		glm::vec3 const forward_tip = transform_point(pose.camera_to_world, glm::vec3{ 0.0f, 0.0f, near_depth * 1.55f });
		add_line(vertices, origin, forward_tip, glm::vec3{ 0.9f, 0.9f, 1.0f });

		if (has_previous)
			add_line(vertices, previous_position, origin, trajectory_colour);
		previous_position = origin;
		has_previous = true;
	}

	return vertices;
}

bool configure_vertex_layout(VertexArray const& vertex_array, GLuint buffer, GLsizei stride)
{
	constexpr GLuint binding_index = 0u;
	if (!vertex_array.bind_vertex_buffer(binding_index, buffer, 0, stride))
		return false;
	return vertex_array.configure_float_attribute(0u, 3, GL_FLOAT, GL_FALSE, 0u, binding_index) &&
	       vertex_array.configure_float_attribute(1u, 3, GL_FLOAT, GL_FALSE, 12u, binding_index);
}

} // namespace

RendererBuildResult Renderer::initialise(sfm::scene::PointCloud const& point_cloud,
                                         sfm::scene::CameraPoseSet const& camera_poses)
{
	RendererBuildResult result{};
	m_ready = false;

	RendererBuildResult grid_result = initialise_grid_pipeline();
	append_messages(result, grid_result);
	if (!grid_result.succeeded)
		return result;

	RendererBuildResult camera_result = initialise_camera_pose_pipeline(camera_poses);
	append_messages(result, camera_result);
	if (!camera_result.succeeded)
		return result;

	RendererBuildResult point_result = reload_point_cloud(point_cloud);
	append_messages(result, point_result);
	if (!point_result.succeeded)
		return result;

	result.succeeded = true;
	add_message(result, "Central renderer is ready with grid/axes, camera poses and point cloud pipelines");
	return result;
}

RendererBuildResult Renderer::initialise_grid_pipeline()
{
	RendererBuildResult result{};
	m_grid_ready = false;
	m_ready = false;
	m_line_vertex_count = 0;

	std::array<ShaderSource, 2> const grid_sources{
		ShaderSource{ ShaderStage::vertex, kLineVertexShader, "SfmSandbox grid vertex shader" },
		ShaderSource{ ShaderStage::fragment, kLineFragmentShader, "SfmSandbox grid fragment shader" },
	};
	ShaderProgramBuildResult grid_build = ShaderProgram::build(grid_sources, "SfmSandbox grid renderer program");
	if (!grid_build.succeeded || !grid_build.program) {
		add_message(result, "Grid shader build failed");
		add_message(result, grid_build.log.empty() ? "<empty shader log>" : grid_build.log);
		return result;
	}
	m_grid_program = std::move(grid_build.program);
	m_grid_world_to_clip_uniform = m_grid_program.uniform_location("u_world_to_clip");
	if (!m_grid_world_to_clip_uniform) {
		add_message(result, "Grid shader build failed: u_world_to_clip uniform is missing");
		return result;
	}
	add_message(result, "Grid/axes shader compiled and cached camera uniform");

	std::vector<DebugVertex> const grid_vertices = build_grid_and_axes_vertices();
	m_line_vertex_count = static_cast<GLsizei>(grid_vertices.size());
	m_grid_vertex_buffer = Buffer{ "SfmSandbox grid vertex buffer" };
	if (!m_grid_vertex_buffer.set_storage(std::span<DebugVertex const>{ grid_vertices.data(), grid_vertices.size() })) {
		add_message(result, "Grid vertex-buffer upload failed");
		return result;
	}
	m_grid_vertex_array = VertexArray{ "SfmSandbox grid vertex array" };
	if (!configure_vertex_layout(m_grid_vertex_array, m_grid_vertex_buffer.id(), static_cast<GLsizei>(sizeof(DebugVertex)))) {
		add_message(result, "Grid vertex-array layout failed");
		return result;
	}
	add_message(result, "Grid/axes GPU layout configured");

	m_grid_ready = true;
	result.succeeded = true;
	return result;
}

RendererBuildResult Renderer::initialise_camera_pose_pipeline(sfm::scene::CameraPoseSet const& camera_poses)
{
	RendererBuildResult result{};
	m_camera_pose_ready = false;
	m_camera_line_vertex_count = 0;
	m_camera_vertex_array.reset();
	m_camera_vertex_buffer.reset();

	RendererBuildResult reload_result = reload_camera_poses(camera_poses);
	append_messages(result, reload_result);
	if (!reload_result.succeeded)
		return result;

	result.succeeded = true;
	return result;
}

RendererBuildResult Renderer::initialise_point_pipeline_if_needed()
{
	RendererBuildResult result{};
	if (m_point_program_ready) {
		result.succeeded = true;
		return result;
	}

	std::array<ShaderSource, 2> const point_sources{
		ShaderSource{ ShaderStage::vertex, kPointVertexShader, "SfmSandbox point vertex shader" },
		ShaderSource{ ShaderStage::fragment, kPointFragmentShader, "SfmSandbox point fragment shader" },
	};
	ShaderProgramBuildResult point_build = ShaderProgram::build(point_sources, "SfmSandbox point renderer program");
	if (!point_build.succeeded || !point_build.program) {
		add_message(result, "Point shader build failed");
		add_message(result, point_build.log.empty() ? "<empty shader log>" : point_build.log);
		return result;
	}
	m_point_program = std::move(point_build.program);
	m_point_world_to_clip_uniform = m_point_program.uniform_location("u_world_to_clip");
	m_point_size_uniform = m_point_program.uniform_location("u_point_size");
	m_point_colour_mode_uniform = m_point_program.uniform_location("u_colour_mode");
	m_point_solid_colour_uniform = m_point_program.uniform_location("u_solid_colour");
	if (!m_point_world_to_clip_uniform || !m_point_size_uniform || !m_point_colour_mode_uniform || !m_point_solid_colour_uniform) {
		add_message(result, "Point shader build failed: required uniform is missing");
		return result;
	}
	m_point_program_ready = true;
	result.succeeded = true;
	add_message(result, "Point shader compiled and cached inspection uniforms");
	return result;
}

RendererBuildResult Renderer::reload_point_cloud(sfm::scene::PointCloud const& point_cloud)
{
	RendererBuildResult result{};
	if (!m_grid_ready) {
		add_message(result, "Point cloud reload failed: grid pipeline is not ready");
		return result;
	}
	if (point_cloud.empty()) {
		add_message(result, "Point cloud reload failed: CPU point cloud is empty");
		return result;
	}

	RendererBuildResult point_program_result = initialise_point_pipeline_if_needed();
	append_messages(result, point_program_result);
	if (!point_program_result.succeeded)
		return result;

	std::vector<PointVertex> const point_vertices = build_point_vertices(point_cloud);
	Buffer replacement_buffer{ "SfmSandbox point vertex buffer" };
	if (!replacement_buffer.set_storage(std::span<PointVertex const>{ point_vertices.data(), point_vertices.size() })) {
		add_message(result, "Point vertex-buffer upload failed; previous point cloud kept");
		return result;
	}
	VertexArray replacement_vertex_array{ "SfmSandbox point vertex array" };
	if (!configure_vertex_layout(replacement_vertex_array, replacement_buffer.id(), static_cast<GLsizei>(sizeof(PointVertex)))) {
		add_message(result, "Point vertex-array layout failed; previous point cloud kept");
		return result;
	}

	std::vector<DebugVertex> const bounds_vertices = build_bounds_vertices(point_cloud.statistics());
	Buffer replacement_bounds_buffer{ "SfmSandbox point bounds vertex buffer" };
	VertexArray replacement_bounds_vertex_array{ "SfmSandbox point bounds vertex array" };
	bool const replacement_bounds_ready = !bounds_vertices.empty();
	if (replacement_bounds_ready) {
		if (!replacement_bounds_buffer.set_storage(std::span<DebugVertex const>{ bounds_vertices.data(), bounds_vertices.size() })) {
			add_message(result, "Point bounds vertex-buffer upload failed; previous point cloud kept");
			return result;
		}
		if (!configure_vertex_layout(replacement_bounds_vertex_array, replacement_bounds_buffer.id(), static_cast<GLsizei>(sizeof(DebugVertex)))) {
			add_message(result, "Point bounds vertex-array layout failed; previous point cloud kept");
			return result;
		}
	}

	m_point_vertex_buffer = std::move(replacement_buffer);
	m_point_vertex_array = std::move(replacement_vertex_array);
	m_point_count = static_cast<GLsizei>(point_vertices.size());
	m_bounds_vertex_buffer = std::move(replacement_bounds_buffer);
	m_bounds_vertex_array = std::move(replacement_bounds_vertex_array);
	m_bounds_line_vertex_count = static_cast<GLsizei>(bounds_vertices.size());
	m_bounds_ready = replacement_bounds_ready;
	m_ready = m_grid_ready && m_camera_pose_ready;

	add_message(result, "Point cloud GPU resources rebuilt from loaded data");
	if (m_bounds_ready)
		add_message(result, "Point cloud bounds GPU layout configured");
	result.succeeded = true;
	return result;
}

RendererBuildResult Renderer::reload_camera_poses(sfm::scene::CameraPoseSet const& camera_poses)
{
	RendererBuildResult result{};
	if (!m_grid_ready) {
		add_message(result, "Camera pose reload failed: grid pipeline is not ready");
		return result;
	}
	if (camera_poses.empty()) {
		add_message(result, "Camera pose reload failed: pose set is empty; previous poses kept");
		return result;
	}

	std::vector<DebugVertex> const camera_vertices = build_camera_pose_vertices(camera_poses);
	if (camera_vertices.empty()) {
		add_message(result, "Camera pose reload failed: no line vertices were generated; previous poses kept");
		return result;
	}

	Buffer replacement_buffer{ "SfmSandbox camera pose vertex buffer" };
	if (!replacement_buffer.set_storage(std::span<DebugVertex const>{ camera_vertices.data(), camera_vertices.size() })) {
		add_message(result, "Camera pose vertex-buffer upload failed; previous poses kept");
		return result;
	}
	VertexArray replacement_vertex_array{ "SfmSandbox camera pose vertex array" };
	if (!configure_vertex_layout(replacement_vertex_array, replacement_buffer.id(), static_cast<GLsizei>(sizeof(DebugVertex)))) {
		add_message(result, "Camera pose vertex-array layout failed; previous poses kept");
		return result;
	}

	m_camera_vertex_buffer = std::move(replacement_buffer);
	m_camera_vertex_array = std::move(replacement_vertex_array);
	m_camera_line_vertex_count = static_cast<GLsizei>(camera_vertices.size());
	m_camera_pose_ready = true;
	m_ready = m_grid_ready && m_point_count > 0;

	add_message(result, "Camera pose GPU resources rebuilt from loaded data");
	add_message(result, "Camera frustum and trajectory GPU layout configured");
	result.succeeded = true;
	return result;
}

void Renderer::render(glm::mat4 const& world_to_clip, PointCloudRenderSettings const& point_settings) const noexcept
{
	if (!m_ready)
		return;

	m_grid_program.set_uniform(m_grid_world_to_clip_uniform, world_to_clip);
	m_grid_program.bind();
	glBindVertexArray(m_grid_vertex_array.id());
	glDrawArrays(GL_LINES, 0, m_line_vertex_count);

	if (point_settings.show_bounds && m_bounds_ready) {
		glBindVertexArray(m_bounds_vertex_array.id());
		glDrawArrays(GL_LINES, 0, m_bounds_line_vertex_count);
	}
	if (m_camera_pose_ready) {
		glBindVertexArray(m_camera_vertex_array.id());
		glDrawArrays(GL_LINES, 0, m_camera_line_vertex_count);
	}

	float const clamped_point_size = std::clamp(point_settings.point_size, 1.0f, 32.0f);
	m_point_program.set_uniform(m_point_world_to_clip_uniform, world_to_clip);
	m_point_program.set_uniform(m_point_size_uniform, clamped_point_size);
	m_point_program.set_uniform(m_point_colour_mode_uniform, static_cast<float>(point_settings.colour_mode));
	m_point_program.set_uniform(m_point_solid_colour_uniform, point_settings.solid_colour);
	m_point_program.bind();
	glBindVertexArray(m_point_vertex_array.id());
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, m_point_count);
	glBindVertexArray(0);
}

} // namespace sfm::gfx
