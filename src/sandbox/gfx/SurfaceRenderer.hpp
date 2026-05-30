#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "sandbox/scene/SurfaceImport.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <array>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace sfm::gfx
{

struct SurfaceRendererResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

struct SurfaceRendererStats final
{
	int draw_calls{ 0 };
	int vertices_drawn{ 0 };
	int triangles_drawn{ 0 };
};

class SurfaceRenderer final
{
public:
	SurfaceRenderer() = default;

	[[nodiscard]] SurfaceRendererResult initialise()
	{
		SurfaceRendererResult result{};
		if (m_program) {
			result.succeeded = true;
			return result;
		}

		std::array<ShaderFileSource, 2> const sources{
			ShaderFileSource{ ShaderStage::vertex, "shaders/sandbox/line.vert" },
			ShaderFileSource{ ShaderStage::fragment, "shaders/sandbox/line.frag" },
		};
		ShaderProgramBuildResult build = ShaderProgram::build_from_files(sources, "SfmSandbox surface renderer program");
		if (!build.succeeded || !build.program) {
			result.messages.push_back("Surface shader file build failed");
			result.messages.push_back(build.log.empty() ? "<empty shader log>" : build.log);
			return result;
		}

		m_program = std::move(build.program);
		m_world_to_clip_uniform = m_program.uniform_location("u_world_to_clip");
		if (!m_world_to_clip_uniform) {
			result.messages.push_back("Surface shader build failed: u_world_to_clip uniform is missing");
			return result;
		}
		result.messages.push_back("Surface renderer initialized with file-backed colour shader");
		result.succeeded = true;
		return result;
	}

	[[nodiscard]] SurfaceRendererResult reload(sfm::scene::SurfaceMesh const& mesh, glm::vec3 colour)
	{
		SurfaceRendererResult result = initialise();
		if (!result.succeeded)
			return result;
		if (mesh.empty()) {
			result.succeeded = false;
			result.messages.push_back("Surface reload failed: mesh is empty; previous surface kept");
			return result;
		}

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.triangle_count() * 3u);
		for (sfm::scene::SurfaceTriangle const& triangle : mesh.faces()) {
			if (triangle.a >= mesh.vertex_count() || triangle.b >= mesh.vertex_count() || triangle.c >= mesh.vertex_count()) {
				result.succeeded = false;
				result.messages.push_back("Surface reload failed: triangle references an out-of-range vertex; previous surface kept");
				return result;
			}
			append_vertex(vertices, mesh.positions[triangle.a], colour);
			append_vertex(vertices, mesh.positions[triangle.b], colour);
			append_vertex(vertices, mesh.positions[triangle.c], colour);
		}
		if (vertices.empty()) {
			result.succeeded = false;
			result.messages.push_back("Surface reload failed: no drawable triangle vertices; previous surface kept");
			return result;
		}

		Buffer replacement_buffer{ "SfmSandbox surface vertex buffer" };
		if (!replacement_buffer.set_storage(std::span<Vertex const>{ vertices.data(), vertices.size() })) {
			result.succeeded = false;
			result.messages.push_back("Surface vertex-buffer upload failed; previous surface kept");
			return result;
		}
		VertexArray replacement_vertex_array{ "SfmSandbox surface vertex array" };
		if (!replacement_vertex_array.bind_vertex_buffer(0u, replacement_buffer.id(), 0, static_cast<GLsizei>(sizeof(Vertex))) ||
		    !replacement_vertex_array.configure_float_attribute(0u, 3, GL_FLOAT, GL_FALSE, 0u, 0u) ||
		    !replacement_vertex_array.configure_float_attribute(1u, 3, GL_FLOAT, GL_FALSE, 12u, 0u)) {
			result.succeeded = false;
			result.messages.push_back("Surface vertex-array layout failed; previous surface kept");
			return result;
		}

		m_vertex_buffer = std::move(replacement_buffer);
		m_vertex_array = std::move(replacement_vertex_array);
		m_vertex_count = static_cast<GLsizei>(vertices.size());
		m_ready = true;
		result.messages.push_back("Surface GPU resources rebuilt from loaded mesh");
		result.succeeded = true;
		return result;
	}

	void render(glm::mat4 const& world_to_clip, bool visible) const noexcept
	{
		m_last_stats = {};
		if (!visible || !m_ready || !m_program || m_vertex_count <= 0)
			return;

		m_program.set_uniform(m_world_to_clip_uniform, world_to_clip);
		m_program.bind();
		glBindVertexArray(m_vertex_array.id());
		glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);
		glBindVertexArray(0);
		m_last_stats.draw_calls = 1;
		m_last_stats.vertices_drawn = m_vertex_count;
		m_last_stats.triangles_drawn = m_vertex_count / 3;
	}

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] GLsizei vertex_count() const noexcept { return m_vertex_count; }
	[[nodiscard]] GLsizei triangle_count() const noexcept { return m_vertex_count / 3; }
	[[nodiscard]] SurfaceRendererStats frame_statistics() const noexcept { return m_last_stats; }

private:
	struct Vertex final
	{
		float position[3];
		float colour[3];
	};

	static void append_vertex(std::vector<Vertex>& vertices, glm::vec3 position, glm::vec3 colour)
	{
		vertices.push_back(Vertex{ { position.x, position.y, position.z }, { colour.r, colour.g, colour.b } });
	}

	ShaderProgram m_program{};
	UniformLocation m_world_to_clip_uniform{};
	VertexArray m_vertex_array{};
	Buffer m_vertex_buffer{};
	GLsizei m_vertex_count{ 0 };
	bool m_ready{ false };
	mutable SurfaceRendererStats m_last_stats{};
};

} // namespace sfm::gfx
