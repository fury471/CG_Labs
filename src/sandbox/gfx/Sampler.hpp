#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns one OpenGL sampler object.
///
/// Samplers separate filtering and wrapping policy from texture image storage.
/// The ownership type arrives now; concrete filtering policy is added only
/// when a rendered feature requires it.
class Sampler final
{
public:
	Sampler() noexcept = default;
	explicit Sampler(std::string_view debug_label) noexcept;
	~Sampler() noexcept;

	Sampler(Sampler const&) = delete;
	Sampler& operator=(Sampler const&) = delete;
	Sampler(Sampler&& other) noexcept;
	Sampler& operator=(Sampler&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	[[nodiscard]] bool set_parameter(GLenum parameter, GLint value) const noexcept;
	[[nodiscard]] bool configure_linear_clamp() const noexcept;

	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
