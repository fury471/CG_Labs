#include "Renderer.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <string>
#include <utility>

namespace sfm::gfx
{
namespace
{

struct DebugVertex final
{
	float position[3];
	float colour[3];
};

constexpr std::array<DebugVertex, 3> kTriangleVertices{
	DebugVertex{ { -0.65f, -0.55f, 0.0f }, { 1.0f, 0.25f, 0.20f } },
	DebugVertex{ {  0.65f, -0.55f, 0.0f }, { 0.20f, 0.85f, 0.30f } },
	DebugVertex{ {  0.00f,  0.60f, 0.0f }, { 0.25f, 0.55f, 1.0f } },
};

constexpr std::string_view kTriangleVertexShader = R"glsl(
#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;

out vec3 v_colour;

void main()
{
	v_colour = in_colour;
	gl_Position = vec4(in_position, 1.0);
}
)glsl";

constexpr std::string_view kTriangleFragmentShader = R"glsl(
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

} // namespace

RendererBuildResult Renderer::initialise()
{
	RendererBuildResult result{};
	m_ready = false;

	std::array<ShaderSource, 2> const sources{
		ShaderSource{ ShaderStage::vertex, kTriangleVertexShader, "Milestone4 triangle vertex shader" },
		ShaderSource{ ShaderStage::fragment, kTriangleFragmentShader, "Milestone4 triangle fragment shader" },
	};

	ShaderProgramBuildResult shader_build = ShaderProgram::build(sources, "Milestone4 central renderer triangle program");
	if (!shader_build.succeeded || !shader_build.program) {
		add_message(result, "Renderer shader build failed");
		add_message(result, shader_build.log.empty() ? "<empty shader log>" : shader_build.log);
		return result;
	}
	m_program = std::move(shader_build.program);
	add_message(result, "Triangle shader program compiled and linked");

	m_vertex_buffer = Buffer{ "Milestone4 triangle vertex buffer" };
	if (!m_vertex_buffer.set_storage(std::span<DebugVertex const>{ kTriangleVertices })) {
		add_message(result, "Triangle vertex-buffer upload failed");
		return result;
	}
	add_message(result, "Triangle vertex buffer uploaded with immutable storage");

	m_vertex_array = VertexArray{ "Milestone4 triangle vertex array" };
	constexpr GLuint binding_index = 0u;
	if (!m_vertex_array.bind_vertex_buffer(binding_index,
	                                       m_vertex_buffer.id(),
	                                       0,
	                                       static_cast<GLsizei>(sizeof(DebugVertex)))) {
		add_message(result, "Triangle vertex-buffer binding failed");
		return result;
	}

	if (!m_vertex_array.configure_float_attribute(0u,
	                                             3,
	                                             GL_FLOAT,
	                                             GL_FALSE,
	                                             static_cast<GLuint>(offsetof(DebugVertex, position)),
	                                             binding_index)) {
		add_message(result, "Triangle position-attribute configuration failed");
		return result;
	}

	if (!m_vertex_array.configure_float_attribute(1u,
	                                             3,
	                                             GL_FLOAT,
	                                             GL_FALSE,
	                                             static_cast<GLuint>(offsetof(DebugVertex, colour)),
	                                             binding_index)) {
		add_message(result, "Triangle colour-attribute configuration failed");
		return result;
	}
	add_message(result, "Triangle vertex-array layout configured through DSA");

	m_ready = true;
	result.succeeded = true;
	add_message(result, "Central renderer is ready and submitted one triangle pipeline");
	return result;
}

void Renderer::render() const noexcept
{
	if (!m_ready)
		return;

	m_program.bind();
	glBindVertexArray(m_vertex_array.id());
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(kTriangleVertices.size()));
	glBindVertexArray(0);
}

} // namespace sfm::gfx
