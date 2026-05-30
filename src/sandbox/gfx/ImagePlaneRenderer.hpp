#pragma once

#include "Buffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "sandbox/scene/ImageImport.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <array>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace sfm::gfx
{

struct ImagePlaneRendererResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

struct ImagePlaneRendererStats final
{
	int draw_calls{ 0 };
	int vertices_drawn{ 0 };
	int triangles_drawn{ 0 };
};

class ImagePlaneRenderer final
{
public:
	ImagePlaneRenderer() = default;

	[[nodiscard]] ImagePlaneRendererResult initialise()
	{
		ImagePlaneRendererResult result{};
		if (m_program) {
			result.succeeded = true;
			return result;
		}

		std::array<ShaderFileSource, 2> const sources{
			ShaderFileSource{ ShaderStage::vertex, "shaders/sandbox/line.vert" },
			ShaderFileSource{ ShaderStage::fragment, "shaders/sandbox/line.frag" },
		};
		ShaderProgramBuildResult build = ShaderProgram::build_from_files(sources, "SfmSandbox image plane renderer program");
		if (!build.succeeded || !build.program) {
			result.messages.push_back("Image-plane shader file build failed");
			result.messages.push_back(build.log.empty() ? "<empty shader log>" : build.log);
			return result;
		}

		m_program = std::move(build.program);
		m_world_to_clip_uniform = m_program.uniform_location("u_world_to_clip");
		if (!m_world_to_clip_uniform) {
			result.messages.push_back("Image-plane shader build failed: u_world_to_clip uniform is missing");
			return result;
		}
		result.messages.push_back("Image-plane renderer initialized with file-backed colour shader");
		result.succeeded = true;
		return result;
	}

	[[nodiscard]] ImagePlaneRendererResult reload(sfm::scene::ImageResource const& image,
	                                            glm::mat4 const& camera_to_world,
	                                            float distance,
	                                            float height)
	{
		ImagePlaneRendererResult result = initialise();
		if (!result.succeeded)
			return result;
		if (image.empty()) {
			result.succeeded = false;
			result.messages.push_back("Image-plane reload failed: image is empty; previous image plane kept");
			return result;
		}
		distance = std::max(distance, 0.05f);
		height = std::max(height, 0.05f);
		float const aspect = static_cast<float>(image.width) / static_cast<float>(image.height);
		float const half_h = height * 0.5f;
		float const half_w = half_h * aspect;

		glm::vec3 const tl = transform(camera_to_world, glm::vec3{ -half_w,  half_h, -distance });
		glm::vec3 const tr = transform(camera_to_world, glm::vec3{  half_w,  half_h, -distance });
		glm::vec3 const br = transform(camera_to_world, glm::vec3{  half_w, -half_h, -distance });
		glm::vec3 const bl = transform(camera_to_world, glm::vec3{ -half_w, -half_h, -distance });

		glm::vec3 const c_tl = sfm::scene::sample_image_nearest(image, 0, 0);
		glm::vec3 const c_tr = sfm::scene::sample_image_nearest(image, image.width - 1, 0);
		glm::vec3 const c_bl = sfm::scene::sample_image_nearest(image, 0, image.height - 1);
		glm::vec3 const c_br = sfm::scene::sample_image_nearest(image, image.width - 1, image.height - 1);

		std::array<Vertex, 6> const vertices{
			make_vertex(tl, c_tl), make_vertex(bl, c_bl), make_vertex(br, c_br),
			make_vertex(tl, c_tl), make_vertex(br, c_br), make_vertex(tr, c_tr),
		};

		Buffer replacement_buffer{ "SfmSandbox image plane vertex buffer" };
		if (!replacement_buffer.set_storage(std::span<Vertex const>{ vertices.data(), vertices.size() })) {
			result.succeeded = false;
			result.messages.push_back("Image-plane vertex-buffer upload failed; previous image plane kept");
			return result;
		}
		VertexArray replacement_vertex_array{ "SfmSandbox image plane vertex array" };
		if (!replacement_vertex_array.bind_vertex_buffer(0u, replacement_buffer.id(), 0, static_cast<GLsizei>(sizeof(Vertex))) ||
		    !replacement_vertex_array.configure_float_attribute(0u, 3, GL_FLOAT, GL_FALSE, 0u, 0u) ||
		    !replacement_vertex_array.configure_float_attribute(1u, 3, GL_FLOAT, GL_FALSE, 12u, 0u)) {
			result.succeeded = false;
			result.messages.push_back("Image-plane vertex-array layout failed; previous image plane kept");
			return result;
		}

		m_vertex_buffer = std::move(replacement_buffer);
		m_vertex_array = std::move(replacement_vertex_array);
		m_ready = true;
		result.messages.push_back("Image plane rebuilt for associated camera pose");
		result.succeeded = true;
		return result;
	}

	void render(glm::mat4 const& world_to_clip, bool visible) const noexcept
	{
		m_last_stats = {};
		if (!visible || !m_ready || !m_program)
			return;
		m_program.set_uniform(m_world_to_clip_uniform, world_to_clip);
		m_program.bind();
		glBindVertexArray(m_vertex_array.id());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		m_last_stats.draw_calls = 1;
		m_last_stats.vertices_drawn = 6;
		m_last_stats.triangles_drawn = 2;
	}

	[[nodiscard]] bool ready() const noexcept { return m_ready; }
	[[nodiscard]] ImagePlaneRendererStats frame_statistics() const noexcept { return m_last_stats; }

private:
	struct Vertex final { float position[3]; float colour[3]; };

	static Vertex make_vertex(glm::vec3 p, glm::vec3 c)
	{
		return Vertex{ { p.x, p.y, p.z }, { c.r, c.g, c.b } };
	}

	static glm::vec3 transform(glm::mat4 const& matrix, glm::vec3 p)
	{
		glm::vec4 const q = matrix * glm::vec4{ p, 1.0f };
		return glm::vec3{ q } / q.w;
	}

	ShaderProgram m_program{};
	UniformLocation m_world_to_clip_uniform{};
	VertexArray m_vertex_array{};
	Buffer m_vertex_buffer{};
	bool m_ready{ false };
	mutable ImagePlaneRendererStats m_last_stats{};
};

} // namespace sfm::gfx
