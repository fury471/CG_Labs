#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <array>
#include <cmath>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace sfm::gfx
{

struct MarkerStressSettings final
{
	int marker_count{ 400 };
	float radius{ 4.0f };
	float height{ 1.2f };
	float marker_scale{ 0.06f };
	bool use_instancing{ true };
	bool visible{ true };
};

struct MarkerStressStats final
{
	int marker_count{ 0 };
	int draw_calls{ 0 };
	int reference_draw_calls{ 0 };
	int instanced_draw_calls{ 0 };
	int vertices_drawn{ 0 };
	bool using_instancing{ true };
};

struct MarkerRendererResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

class InstancedMarkerRenderer final
{
public:
	InstancedMarkerRenderer() = default;

	[[nodiscard]] MarkerRendererResult initialise()
	{
		MarkerRendererResult result{};
		if (m_program) {
			result.succeeded = true;
			return result;
		}

		std::array<ShaderFileSource, 2> const sources{
			ShaderFileSource{ ShaderStage::vertex, "shaders/sandbox/instanced_marker.vert" },
			ShaderFileSource{ ShaderStage::fragment, "shaders/sandbox/line.frag" },
		};
		ShaderProgramBuildResult build = ShaderProgram::build_from_files(sources, "SfmSandbox instanced marker program");
		if (!build.succeeded || !build.program) {
			result.messages.push_back("Instanced marker shader build failed");
			result.messages.push_back(build.log.empty() ? "<empty shader log>" : build.log);
			return result;
		}

		m_program = std::move(build.program);
		m_world_to_clip_uniform = m_program.uniform_location("u_world_to_clip");
		m_marker_scale_uniform = m_program.uniform_location("u_marker_scale");
		if (!m_world_to_clip_uniform || !m_marker_scale_uniform) {
			result.messages.push_back("Instanced marker shader build failed: required uniform is missing");
			return result;
		}

		std::array<Vertex, 6> const marker_vertices{
			make_vertex({ -1.0f, 0.0f, 0.0f }), make_vertex({ 1.0f, 0.0f, 0.0f }),
			make_vertex({ 0.0f, -1.0f, 0.0f }), make_vertex({ 0.0f, 1.0f, 0.0f }),
			make_vertex({ 0.0f, 0.0f, -1.0f }), make_vertex({ 0.0f, 0.0f, 1.0f }),
		};
		m_marker_vertex_buffer = Buffer{ "SfmSandbox marker unit glyph vertex buffer" };
		if (!m_marker_vertex_buffer.set_storage(std::span<Vertex const>{ marker_vertices.data(), marker_vertices.size() })) {
			result.messages.push_back("Marker unit glyph vertex upload failed");
			return result;
		}
		m_vertex_count = static_cast<GLsizei>(marker_vertices.size());
		result.messages.push_back("Instanced marker renderer initialized");
		result.succeeded = true;
		return result;
	}

	[[nodiscard]] MarkerRendererResult rebuild(MarkerStressSettings const& settings)
	{
		MarkerRendererResult result = initialise();
		if (!result.succeeded)
			return result;
		m_instance_count = std::max(settings.marker_count, 1);

		std::vector<Instance> instances;
		instances.reserve(static_cast<std::size_t>(m_instance_count));
		for (int i = 0; i < m_instance_count; ++i) {
			float const t = static_cast<float>(i) / static_cast<float>(m_instance_count);
			float const angle = t * 6.28318530718f * 6.0f;
			float const ring = settings.radius * (0.35f + 0.65f * t);
			glm::vec3 const offset{ std::cos(angle) * ring, settings.height + 0.5f * std::sin(t * 6.28318530718f * 3.0f), std::sin(angle) * ring };
			glm::vec3 const colour{ 0.25f + 0.75f * t, 0.85f - 0.45f * t, 1.0f - 0.65f * t };
			instances.push_back(make_instance(offset, colour));
		}

		m_instance_buffer = Buffer{ "SfmSandbox marker instance buffer" };
		if (!m_instance_buffer.set_storage(std::span<Instance const>{ instances.data(), instances.size() })) {
			result.succeeded = false;
			result.messages.push_back("Marker instance upload failed; previous stress scene kept");
			return result;
		}

		m_vertex_array = VertexArray{ "SfmSandbox instanced marker vertex array" };
		if (!m_vertex_array.bind_vertex_buffer(0u, m_marker_vertex_buffer.id(), 0, static_cast<GLsizei>(sizeof(Vertex))) ||
		    !m_vertex_array.bind_vertex_buffer(1u, m_instance_buffer.id(), 0, static_cast<GLsizei>(sizeof(Instance))) ||
		    !m_vertex_array.configure_float_attribute(0u, 3, GL_FLOAT, GL_FALSE, 0u, 0u) ||
		    !m_vertex_array.configure_float_attribute(1u, 3, GL_FLOAT, GL_FALSE, 0u, 1u) ||
		    !m_vertex_array.configure_float_attribute(2u, 3, GL_FLOAT, GL_FALSE, 12u, 1u)) {
			result.succeeded = false;
			result.messages.push_back("Marker vertex-array layout failed; previous stress scene kept");
			return result;
		}
		glVertexArrayBindingDivisor(m_vertex_array.id(), 1u, 1u);
		m_ready = true;
		result.messages.push_back("Marker stress scene rebuilt: " + std::to_string(m_instance_count) + " repeated markers");
		result.succeeded = true;
		return result;
	}

	void render(glm::mat4 const& world_to_clip, MarkerStressSettings const& settings) const noexcept
	{
		m_last_stats = {};
		m_last_stats.marker_count = m_instance_count;
		m_last_stats.reference_draw_calls = m_instance_count;
		m_last_stats.instanced_draw_calls = m_instance_count > 0 ? 1 : 0;
		m_last_stats.using_instancing = settings.use_instancing;
		if (!settings.visible || !m_ready || !m_program || m_instance_count <= 0)
			return;

		m_program.set_uniform(m_world_to_clip_uniform, world_to_clip);
		m_program.set_uniform(m_marker_scale_uniform, settings.marker_scale);
		m_program.bind();
		glBindVertexArray(m_vertex_array.id());
		if (settings.use_instancing) {
			glDrawArraysInstanced(GL_LINES, 0, m_vertex_count, m_instance_count);
			m_last_stats.draw_calls = 1;
		} else {
			for (int i = 0; i < m_instance_count; ++i)
				glDrawArraysInstancedBaseInstance(GL_LINES, 0, m_vertex_count, 1, static_cast<GLuint>(i));
			m_last_stats.draw_calls = m_instance_count;
		}
		glBindVertexArray(0);
		m_last_stats.vertices_drawn = m_vertex_count * m_instance_count;
	}

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] MarkerStressStats frame_statistics() const noexcept { return m_last_stats; }

private:
	struct Vertex final { float position[3]; };
	struct Instance final { float offset[3]; float colour[3]; };

	static Vertex make_vertex(glm::vec3 position)
	{
		return Vertex{ { position.x, position.y, position.z } };
	}

	static Instance make_instance(glm::vec3 offset, glm::vec3 colour)
	{
		return Instance{ { offset.x, offset.y, offset.z }, { colour.r, colour.g, colour.b } };
	}

	ShaderProgram m_program{};
	UniformLocation m_world_to_clip_uniform{};
	UniformLocation m_marker_scale_uniform{};
	VertexArray m_vertex_array{};
	Buffer m_marker_vertex_buffer{};
	Buffer m_instance_buffer{};
	GLsizei m_vertex_count{ 0 };
	int m_instance_count{ 0 };
	bool m_ready{ false };
	mutable MarkerStressStats m_last_stats{};
};

} // namespace sfm::gfx
