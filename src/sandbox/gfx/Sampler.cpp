#include "Sampler.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

Sampler::Sampler(std::string_view debug_label) noexcept
{
	// OpenGL 4.5+ provides DSA creation for sampler objects too. Keeping this
	// constructor in the glCreate* family makes ownership construction consistent
	// across the Milestone 2 resource layer.
	glCreateSamplers(1, &m_id);
	detail::label_object(GL_SAMPLER, m_id, debug_label);
}

Sampler::~Sampler() noexcept
{
	reset();
}

Sampler::Sampler(Sampler&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

Sampler& Sampler::operator=(Sampler&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

bool Sampler::set_parameter(GLenum parameter, GLint value) const noexcept
{
	if (m_id == 0u)
		return false;

	glSamplerParameteri(m_id, parameter, value);
	return true;
}

bool Sampler::configure_linear_clamp() const noexcept
{
	return set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR) &&
	       set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR) &&
	       set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) &&
	       set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Sampler::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteSamplers(1, &m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
