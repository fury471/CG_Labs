#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns the vertex input state used by a draw submission.
///
/// A vertex array records how vertex buffers are interpreted. It is a distinct
/// resource from those buffers, so it receives its own move-only owner.
class VertexArray final
{
public:
	VertexArray() noexcept = default;
	explicit VertexArray(std::string_view debug_label) noexcept;
	~VertexArray() noexcept;

	VertexArray(VertexArray const&) = delete;
	VertexArray& operator=(VertexArray const&) = delete;
	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	/// Binds a vertex buffer to a VAO binding slot without making the VAO current.
	[[nodiscard]] bool bind_vertex_buffer(GLuint binding_index,
	                                      GLuint buffer,
	                                      GLintptr offset,
	                                      GLsizei stride) const noexcept;

	/// Describes one floating-point vertex attribute and connects it to a binding.
	[[nodiscard]] bool configure_float_attribute(GLuint attribute_index,
	                                            GLint component_count,
	                                            GLenum component_type,
	                                            GLboolean normalized,
	                                            GLuint relative_offset,
	                                            GLuint binding_index) const noexcept;

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
