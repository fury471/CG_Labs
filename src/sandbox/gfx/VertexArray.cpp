#include "VertexArray.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

VertexArray::VertexArray(std::string_view debug_label) noexcept
{
	// The vertex array is created eagerly so later configuration can use the
	// Direct State Access API without first binding global vertex-array state.
	glCreateVertexArrays(1, &m_id);
	detail::label_object(GL_VERTEX_ARRAY, m_id, debug_label);
}

VertexArray::~VertexArray() noexcept
{
	reset();
}

VertexArray::VertexArray(VertexArray&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

void VertexArray::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteVertexArrays(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
