#include "Buffer.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

Buffer::Buffer(std::string_view debug_label) noexcept
{
	// glCreateBuffers creates a real buffer object immediately, unlike legacy
	// name generation followed by first bind. This suits a DSA-only owner.
	glCreateBuffers(1, &m_id);
	detail::label_object(GL_BUFFER, m_id, debug_label);
}

Buffer::~Buffer() noexcept
{
	reset();
}

Buffer::Buffer(Buffer&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if (this != &other) {
		// Release our previous object before taking the source name. The source
		// becomes empty, so destruction of either object remains safe.
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

void Buffer::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteBuffers(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
