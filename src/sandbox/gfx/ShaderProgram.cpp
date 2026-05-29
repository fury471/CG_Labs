#include "ShaderProgram.hpp"

#include "DebugLabel.hpp"

#include <utility>

namespace sfm::gfx
{

ShaderProgram::ShaderProgram(std::string_view debug_label) noexcept
	: m_id(glCreateProgram())
{
	// The constructor creates the ownership-bearing program container only.
	// Compilation and linkage arrive with the shader-interface milestone.
	detail::label_object(GL_PROGRAM, m_id, debug_label);
}

ShaderProgram::~ShaderProgram() noexcept
{
	reset();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
	}
	return *this;
}

void ShaderProgram::reset() noexcept
{
	if (m_id == 0u)
		return;

	glDeleteProgram(m_id);
	m_id = 0u;
}

} // namespace sfm::gfx
