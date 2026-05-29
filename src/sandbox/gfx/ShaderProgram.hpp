#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns one OpenGL program object.
///
/// This milestone establishes lifetime ownership only. Shader compilation,
/// attachment, linkage and reflected/cached bindings are introduced with the
/// shader-interface milestone so their API is shaped by real renderer needs.
class ShaderProgram final
{
public:
	ShaderProgram() noexcept = default;
	explicit ShaderProgram(std::string_view debug_label) noexcept;
	~ShaderProgram() noexcept;

	ShaderProgram(ShaderProgram const&) = delete;
	ShaderProgram& operator=(ShaderProgram const&) = delete;
	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
