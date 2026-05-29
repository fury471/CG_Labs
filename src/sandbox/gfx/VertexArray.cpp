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

bool VertexArray::bind_vertex_buffer(GLuint binding_index,
                                     GLuint buffer,
                                     GLintptr offset,
                                     GLsizei stride) const noexcept
{
	if (m_id == 0u || buffer == 0u || stride <= 0)
		return false;

	glVertexArrayVertexBuffer(m_id, binding_index, buffer, offset, stride);
	return true;
}

bool VertexArray::configure_float_attribute(GLuint attribute_index,
                                            GLint component_count,
                                            GLenum component_type,
                                            GLboolean normalized,
                                            GLuint relative_offset,
                                            GLuint binding_index) const noexcept
{
	if (m_id == 0u || component_count <= 0)
		return false;

	glEnableVertexArrayAttrib(m_id, attribute_index);
	glVertexArrayAttribFormat(m_id,
	                          attribute_index,
	                          component_count,
	                          component_type,
	                          normalized,
	                          relative_offset);
	glVertexArrayAttribBinding(m_id, attribute_index, binding_index);
	return true;
}

void VertexArray::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteVertexArrays(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
