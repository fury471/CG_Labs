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

bool Texture2D::allocate_storage(GLsizei width, GLsizei height, GLenum internal_format, GLsizei levels) const noexcept
{
	if (m_id == 0u || width <= 0 || height <= 0 || levels <= 0)
		return false;

	glTextureStorage2D(m_id, levels, internal_format, width, height);
	return true;
}

bool Texture2D::upload_level(GLint level,
                             GLsizei width,
                             GLsizei height,
                             GLenum format,
                             GLenum type,
                             std::span<std::byte const> data) const noexcept
{
	if (m_id == 0u || level < 0 || width <= 0 || height <= 0 || data.empty())
		return false;

	glTextureSubImage2D(m_id, level, 0, 0, width, height, format, type, data.data());
	return true;
}

void Texture2D::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteTextures(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
