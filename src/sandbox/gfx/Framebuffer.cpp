#include "Framebuffer.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

Framebuffer::Framebuffer(std::string_view debug_label) noexcept
{
	// DSA framebuffer creation gives us a complete object name without binding
	// GL_FRAMEBUFFER. Attachment policy remains a later render-target concern.
	glCreateFramebuffers(1, &m_id);
	detail::label_object(GL_FRAMEBUFFER, m_id, debug_label);
}

Framebuffer::~Framebuffer() noexcept
{
	reset();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

void Framebuffer::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteFramebuffers(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
