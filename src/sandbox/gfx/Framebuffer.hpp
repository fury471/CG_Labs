#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns one OpenGL framebuffer object.
///
/// Attachments and completeness validation are intentionally deferred until
/// render targets are introduced. This class first establishes unambiguous
/// ownership for the framebuffer name itself.
class Framebuffer final
{
public:
	Framebuffer() noexcept = default;
	explicit Framebuffer(std::string_view debug_label) noexcept;
	~Framebuffer() noexcept;

	Framebuffer(Framebuffer const&) = delete;
	Framebuffer& operator=(Framebuffer const&) = delete;
	Framebuffer(Framebuffer&& other) noexcept;
	Framebuffer& operator=(Framebuffer&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
