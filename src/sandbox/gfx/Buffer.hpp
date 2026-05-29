#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Owns exactly one OpenGL buffer object.
///
/// A Buffer is intentionally move-only: two C++ values must never believe they
/// own the same GPU name, because both destructors would otherwise delete it.
/// The default-constructed and moved-from state is empty (`id() == 0`).
class Buffer final
{
public:
	Buffer() noexcept = default;
	explicit Buffer(std::string_view debug_label) noexcept;
	~Buffer() noexcept;

	Buffer(Buffer const&) = delete;
	Buffer& operator=(Buffer const&) = delete;
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	/// Releases the owned GPU object, leaving this owner in its empty state.
	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
