#pragma once

#include <glad/gl.h>

#include <cstddef>
#include <span>
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

	/// Allocates immutable storage and uploads the initial byte range.
	///
	/// Milestone 4 only needs static vertex data, so immutable storage is the
	/// narrowest useful API. Dynamic streaming and partial updates will be added
	/// only when a later renderer feature requires them.
	[[nodiscard]] bool set_storage(std::span<std::byte const> bytes, GLbitfield flags = 0u) const noexcept;

	template <typename T>
	[[nodiscard]] bool set_storage(std::span<T const> values, GLbitfield flags = 0u) const noexcept
	{
		return set_storage(std::as_bytes(values), flags);
	}

	/// Releases the owned GPU object, leaving this owner in its empty state.
	void reset() noexcept;

private:
	GLuint m_id{ 0u };
};

} // namespace sfm::gfx
