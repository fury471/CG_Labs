#pragma once

#include <array>

namespace sfm::gfx
{

/// Minimal frame clear operation owned by the graphics layer.
///
/// `SfmSandbox` still has no scene renderer in Milestone 2. Keeping viewport
/// and clear-state changes here prevents the app shell from accumulating raw
/// OpenGL calls while later renderer abstractions are still being designed.
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
