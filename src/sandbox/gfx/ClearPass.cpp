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

	glViewport(0, 0, framebuffer_width, framebuffer_height);
	glClearColor(m_colour[0], m_colour[1], m_colour[2], m_colour[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace sfm::gfx
