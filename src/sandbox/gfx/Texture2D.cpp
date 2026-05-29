#include "Texture2D.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

Texture2D::Texture2D(std::string_view debug_label) noexcept
{
	// The target is part of DSA texture creation: this owner represents a 2D
	// texture specifically, so later storage and upload APIs cannot misuse it as
	// another texture type.
	glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	detail::label_object(GL_TEXTURE, m_id, debug_label);
}

Texture2D::~Texture2D() noexcept
{
	reset();
}

Texture2D::Texture2D(Texture2D&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

void Texture2D::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteTextures(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
