#include "Renderer.hpp"

#include <glad/gl.h>

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
out vec3 v_colour;
void main()
{
	v_colour = in_colour;
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

bool configure_vertex_layout(VertexArray const& vertex_array, GLuint buffer, GLsizei stride)
{
	constexpr GLuint binding_index = 0u;
	if (!vertex_array.bind_vertex_buffer(binding_index, buffer, 0, stride))
		return false;
	return vertex_array.configure_float_attribute(0u, 3, GL_FLOAT, GL_FALSE, 0u, binding_index) &&
	       vertex_array.configure_float_attribute(1u, 3, GL_FLOAT, GL_FALSE, 12u, binding_index);
}

} // namespace

RendererBuildResult Renderer::initialise(sfm::scene::PointCloud const& point_cloud)
{
	RendererBuildResult result{};
	m_ready = false;
	m_line_vertex_count = 0;
	m_point_count = 0;

	std::array<ShaderSource, 2> const grid_sources{
		ShaderSource{ ShaderStage::vertex, kLineVertexShader, "Milestone6 grid vertex shader" },
		ShaderSource{ ShaderStage::fragment, kLineFragmentShader, "Milestone6 grid fragment shader" },
	};
	ShaderProgramBuildResult grid_build = ShaderProgram::build(grid_sources, "Milestone6 grid renderer program");
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
	m_grid_vertex_buffer = Buffer{ "Milestone6 grid vertex buffer" };
	if (!m_grid_vertex_buffer.set_storage(std::span<DebugVertex const>{ grid_vertices.data(), grid_vertices.size() })) {
		add_message(result, "Grid vertex-buffer upload failed");
		return result;
	}
	m_grid_vertex_array = VertexArray{ "Milestone6 grid vertex array" };
	if (!configure_vertex_layout(m_grid_vertex_array, m_grid_vertex_buffer.id(), static_cast<GLsizei>(sizeof(DebugVertex)))) {
		add_message(result, "Grid vertex-array layout failed");
		return result;
	}
	add_message(result, "Grid/axes GPU layout configured");

	if (point_cloud.empty()) {
		add_message(result, "Point cloud is empty; renderer remains unavailable");
		return result;
	}

	std::array<ShaderSource, 2> const point_sources{
		ShaderSource{ ShaderStage::vertex, kPointVertexShader, "Milestone6 point vertex shader" },
		ShaderSource{ ShaderStage::fragment, kPointFragmentShader, "Milestone6 point fragment shader" },
	};
	ShaderProgramBuildResult point_build = ShaderProgram::build(point_sources, "Milestone6 point renderer program");
	if (!point_build.succeeded || !point_build.program) {
		add_message(result, "Point shader build failed");
		add_message(result, point_build.log.empty() ? "<empty shader log>" : point_build.log);
		return result;
	}
	m_point_program = std::move(point_build.program);
	m_point_world_to_clip_uniform = m_point_program.uniform_location("u_world_to_clip");
	UniformLocation const point_size = m_point_program.uniform_location("u_point_size");
	if (!m_point_world_to_clip_uniform || !point_size) {
		add_message(result, "Point shader build failed: required uniform is missing");
		return result;
	}
	m_point_program.set_uniform(point_size, 5.0f);
	add_message(result, "Point shader compiled and cached uniforms");

	std::vector<PointVertex> const point_vertices = build_point_vertices(point_cloud);
	m_point_count = static_cast<GLsizei>(point_vertices.size());
	m_point_vertex_buffer = Buffer{ "Milestone6 point vertex buffer" };
	if (!m_point_vertex_buffer.set_storage(std::span<PointVertex const>{ point_vertices.data(), point_vertices.size() })) {
		add_message(result, "Point vertex-buffer upload failed");
		return result;
	}
	m_point_vertex_array = VertexArray{ "Milestone6 point vertex array" };
	if (!configure_vertex_layout(m_point_vertex_array, m_point_vertex_buffer.id(), static_cast<GLsizei>(sizeof(PointVertex)))) {
		add_message(result, "Point vertex-array layout failed");
		return result;
	}
	add_message(result, "Point cloud GPU layout configured");

	m_ready = true;
	result.succeeded = true;
	add_message(result, "Central renderer is ready with grid/axes and point cloud pipelines");
	return result;
}

void Renderer::render(glm::mat4 const& world_to_clip) const noexcept
{
	if (!m_ready)
		return;

	m_grid_program.set_uniform(m_grid_world_to_clip_uniform, world_to_clip);
	m_grid_program.bind();
	glBindVertexArray(m_grid_vertex_array.id());
	glDrawArrays(GL_LINES, 0, m_line_vertex_count);

	m_point_program.set_uniform(m_point_world_to_clip_uniform, world_to_clip);
	m_point_program.bind();
	glBindVertexArray(m_point_vertex_array.id());
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, m_point_count);
	glBindVertexArray(0);
}

} // namespace sfm::gfx
