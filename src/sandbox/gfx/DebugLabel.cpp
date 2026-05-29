#include "DebugLabel.hpp"

#include <algorithm>
#include <cstddef>

namespace sfm::gfx::detail
{

void label_object(GLenum identifier, GLuint object, std::string_view label) noexcept
{
	// A zero name represents the safe empty state of an RAII wrapper. It is not
	// an OpenGL object and must never be passed to a labelling operation.
	if (object == 0u || label.empty() || glObjectLabel == nullptr)
		return;

	GLint max_label_length = 0;
	glGetIntegerv(GL_MAX_LABEL_LENGTH, &max_label_length);
	if (max_label_length <= 1)
		return;

	// The OpenGL limit includes space for the terminating null character. Pass
	// an explicit character count which is strictly smaller than that limit.
	auto const max_character_count = static_cast<std::size_t>(max_label_length - 1);
	auto const character_count = std::min(label.size(), max_character_count);
	glObjectLabel(identifier, object, static_cast<GLsizei>(character_count), label.data());
}

} // namespace sfm::gfx::detail
