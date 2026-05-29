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

constexpr float kGridExtent = 10.0f;
constexpr int kGridHalfLineCount = 10;
constexpr GLsizei kAxisVertexCount = 6;

constexpr std::array<DebugVertex, kAxisVertexCount> kAxisVertices{
	// X axis: red
	DebugVertex{ { -kGridExtent, 0.0f, 0.0f }, { 0.95f, 0.20f, 0.20f } },
	DebugVertex{ {  kGridExtent, 0.0f, 0.0f }, { 0.95f, 0.20f, 0.20f } },
	// Y axis: green. Including it now makes the world-space convention visible.
	DebugVertex{ { 0.0f, -kGridExtent, 0.0f }, { 0.25f, 0.85f, 0.25f } },
	DebugVertex{ { 0.0f,  kGridExtent, 0.0f }, { 0.25f, 0.85f, 0.25f } },
	// Z axis: blue
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

void add_message(RendererBuildResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

std::vector<DebugVertex> build_grid_and_axes_vertices()
{
	std::vector<DebugVertex> vertices;
	vertices.reserve(static_cast<std::size_t>((kGridHalfLineCount * 2 + 1) * 4 + kAxisVertexCount));

	constexpr float grid_colour[3]{ 0.32f, 0.36f, 0.42f };
	for (int i = -kGridHalfLineCount; i <= kGridHalfLineCount; ++i) {
		float const coordinate = static_cast<float>(i);

		// Ground grid on the XZ plane. This is the conventional reference plane for
		// a 3D viewer and makes camera movement/orientation immediately visible.
		vertices.push_back(DebugVertex{ { -kGridExtent, 0.0f, coordinate }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ {  kGridExtent, 0.0f, coordinate }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ { coordinate, 0.0f, -kGridExtent }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
		vertices.push_back(DebugVertex{ { coordinate, 0.0f,  kGridExtent }, { grid_colour[0], grid_colour[1], grid_colour[2] } });
	}

	vertices.insert(vertices.end(), kAxisVertices.begin(), kAxisVertices.end());
	return vertices;
}

} // namespace

RendererBuildResult Renderer::initialise()
{
	RendererBuildResult result{};
	m_ready = false;
	m_line_vertex_count = 0;

	std::array<ShaderSource, 2> const sources{
		ShaderSource{ ShaderStage::vertex, kLineVertexShader, "Milestone5 grid/axes vertex shader" },
		ShaderSource{ ShaderStage::fragment, kLineFragmentShader, "Milestone5 grid/axes fragment shader" },
	};

	ShaderProgramBuildResult shader_build = ShaderProgram::build(sources, "Milestone5 grid/axes renderer program");
	if (!shader_build.succeeded || !shader_build.program) {
		add_message(result, "Renderer shader build failed");
		add_message(result, shader_build.log.empty() ? "<empty shader log>" : shader_build.log);
		return result;
	}
	m_program = std::move(shader_build.program);
	m_world_to_clip_uniform = m_program.uniform_location("u_world_to_clip");
	if (!m_world_to_clip_uniform) {
		add_message(result, "Renderer shader build failed: u_world_to_clip uniform is missing");
		return result;
	}
	add_message(result, "Grid/axes shader program compiled, linked and cached camera uniform");

	std::vector<DebugVertex> const vertices = build_grid_and_axes_vertices();
	m_line_vertex_count = static_cast<GLsizei>(vertices.size());

	m_vertex_buffer = Buffer{ "Milestone5 grid/axes vertex buffer" };
	if (!m_vertex_buffer.set_storage(std::span<DebugVertex const>{ vertices.data(), vertices.size() })) {
		add_message(result, "Grid/axes vertex-buffer upload failed");
		return result;
	}
	add_message(result, "Grid/axes vertex buffer uploaded with immutable storage");

	m_vertex_array = VertexArray{ "Milestone5 grid/axes vertex array" };
	constexpr GLuint binding_index = 0u;
	if (!m_vertex_array.bind_vertex_buffer(binding_index,
	                                       m_vertex_buffer.id(),
	                                       0,
	                                       static_cast<GLsizei>(sizeof(DebugVertex)))) {
		add_message(result, "Grid/axes vertex-buffer binding failed");
		return result;
	}

	if (!m_vertex_array.configure_float_attribute(0u,
	                                             3,
	                                             GL_FLOAT,
	                                             GL_FALSE,
	                                             static_cast<GLuint>(offsetof(DebugVertex, position)),
	                                             binding_index)) {
		add_message(result, "Grid/axes position-attribute configuration failed");
		return result;
	}

	if (!m_vertex_array.configure_float_attribute(1u,
	                                             3,
	                                             GL_FLOAT,
	                                             GL_FALSE,
	                                             static_cast<GLuint>(offsetof(DebugVertex, colour)),
	                                             binding_index)) {
		add_message(result, "Grid/axes colour-attribute configuration failed");
		return result;
	}
	add_message(result, "Grid/axes vertex-array layout configured through DSA");

	m_ready = true;
	result.succeeded = true;
	add_message(result, "Central renderer is ready and submitted a world-space grid/axes line pipeline");
	return result;
}

void Renderer::render(glm::mat4 const& world_to_clip) const noexcept
{
	if (!m_ready)
		return;

	m_program.set_uniform(m_world_to_clip_uniform, world_to_clip);
	m_program.bind();
	glBindVertexArray(m_vertex_array.id());
	glDrawArrays(GL_LINES, 0, m_line_vertex_count);
	glBindVertexArray(0);
}

} // namespace sfm::gfx
