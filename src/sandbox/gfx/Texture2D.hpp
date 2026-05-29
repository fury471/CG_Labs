#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns one two-dimensional OpenGL texture object.
///
/// This class currently owns lifetime only. Storage formats, mip levels and
/// upload policy will be introduced with a real rendering feature so those
/// choices are driven by observable requirements instead of speculation.
class Texture2D final
{
public:
	Texture2D() noexcept = default;
	explicit Texture2D(std::string_view debug_label) noexcept;
	~Texture2D() noexcept;

	Texture2D(Texture2D const&) = delete;
	Texture2D& operator=(Texture2D const&) = delete;
	Texture2D(Texture2D&& other) noexcept;
	Texture2D& operator=(Texture2D&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
