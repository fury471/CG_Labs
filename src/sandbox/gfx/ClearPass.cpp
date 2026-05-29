#include "ClearPass.hpp"

#include <glad/gl.h>

namespace sfm::gfx
{

ClearPass::ClearPass(Colour colour) noexcept
	: m_colour(colour)
{
}

void ClearPass::initialise() const noexcept
{
	// Depth testing is enabled even before real scene geometry exists. That keeps
	// the app shell aligned with the future 3D viewer path and avoids hiding this
	// global render-state decision in unrelated application code.
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
}

void ClearPass::set_colour(Colour colour) noexcept
{
	m_colour = colour;
}

ClearPass::Colour const& ClearPass::colour() const noexcept
{
	return m_colour;
}

void ClearPass::render(int framebuffer_width, int framebuffer_height) const noexcept
{
	if (framebuffer_width <= 0 || framebuffer_height <= 0)
		return;

	// The framebuffer size can differ from the window size on HiDPI displays and
	// can change during resize/fullscreen transitions, so viewport state is
	// updated from the actual framebuffer extent every frame.
	glViewport(0, 0, framebuffer_width, framebuffer_height);
	glClearColor(m_colour[0], m_colour[1], m_colour[2], m_colour[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace sfm::gfx
