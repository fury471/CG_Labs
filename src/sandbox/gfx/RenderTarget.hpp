#pragma once

#include "Framebuffer.hpp"
#include "Texture2D.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <string>
#include <utility>

namespace sfm::gfx
{

struct RenderTargetStatus final
{
	bool ready{ false };
	int width{ 0 };
	int height{ 0 };
	GLuint framebuffer_id{ 0u };
	GLuint colour_texture_id{ 0u };
	GLenum framebuffer_status{ GL_FRAMEBUFFER_UNDEFINED };
	int rebuild_count{ 0 };
	std::string message{};
};

class RenderTarget final
{
public:
	RenderTarget() = default;

	[[nodiscard]] bool ensure_size(int requested_width, int requested_height)
	{
		requested_width = std::max(requested_width, 1);
		requested_height = std::max(requested_height, 1);
		if (m_status.ready && m_status.width == requested_width && m_status.height == requested_height)
			return true;

		Framebuffer replacement_framebuffer{ "SfmSandbox M18 offscreen framebuffer" };
		Texture2D replacement_colour{ "SfmSandbox M18 offscreen colour target" };
		if (!replacement_framebuffer || !replacement_colour) {
			m_status.ready = false;
			m_status.message = "Render target allocation failed: OpenGL returned an empty object name";
			return false;
		}

		glTextureStorage2D(replacement_colour.id(), 1, GL_RGBA16F, requested_width, requested_height);
		glTextureParameteri(replacement_colour.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(replacement_colour.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(replacement_colour.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(replacement_colour.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(replacement_framebuffer.id(), GL_COLOR_ATTACHMENT0, replacement_colour.id(), 0);
		GLenum const draw_buffer = GL_COLOR_ATTACHMENT0;
		glNamedFramebufferDrawBuffers(replacement_framebuffer.id(), 1, &draw_buffer);

		GLenum const status = glCheckNamedFramebufferStatus(replacement_framebuffer.id(), GL_FRAMEBUFFER);
		m_status.framebuffer_status = status;
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			m_status.ready = false;
			m_status.width = requested_width;
			m_status.height = requested_height;
			m_status.framebuffer_id = replacement_framebuffer.id();
			m_status.colour_texture_id = replacement_colour.id();
			m_status.message = "Render target incomplete: " + framebuffer_status_name(status);
			return false;
		}

		m_framebuffer = std::move(replacement_framebuffer);
		m_colour = std::move(replacement_colour);
		m_status.ready = true;
		m_status.width = requested_width;
		m_status.height = requested_height;
		m_status.framebuffer_id = m_framebuffer.id();
		m_status.colour_texture_id = m_colour.id();
		m_status.framebuffer_status = status;
		++m_status.rebuild_count;
		m_status.message = "Render target complete: " + framebuffer_status_name(status);
		return true;
	}

	[[nodiscard]] RenderTargetStatus const& status() const noexcept { return m_status; }

private:
	[[nodiscard]] static std::string framebuffer_status_name(GLenum status)
	{
		switch (status) {
		case GL_FRAMEBUFFER_COMPLETE: return "GL_FRAMEBUFFER_COMPLETE";
		case GL_FRAMEBUFFER_UNDEFINED: return "GL_FRAMEBUFFER_UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
		default: return "unknown framebuffer status";
		}
	}

	Framebuffer m_framebuffer{};
	Texture2D m_colour{};
	RenderTargetStatus m_status{};
};

} // namespace sfm::gfx
