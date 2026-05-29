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

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
