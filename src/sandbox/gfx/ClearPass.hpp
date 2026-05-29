#pragma once

#include <array>

namespace sfm::gfx
{

class ClearPass final
{
public:
	using Colour = std::array<float, 4>;

	explicit ClearPass(Colour colour) noexcept;

	void initialise() const noexcept;
	void set_colour(Colour colour) noexcept;
	[[nodiscard]] Colour const& colour() const noexcept;
	void render(int framebuffer_width, int framebuffer_height) const noexcept;

private:
	Colour m_colour;
};

} // namespace sfm::gfx
