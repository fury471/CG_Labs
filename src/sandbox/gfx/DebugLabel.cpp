#include "DebugLabel.hpp"

#include <limits>

namespace sfm::gfx::detail
{

void label_object(GLenum identifier, GLuint object, std::string_view label) noexcept
{
	// A zero name represents the safe empty state of an RAII wrapper. It is not
	// an OpenGL object and must never be passed to a labelling operation.
	if (object == 0u || label.empty() || glObjectLabel == nullptr)
		return;

	// Sandbox labels are short developer-authored diagnostics. Clamp the length
	// defensively before converting from size_t to the OpenGL signed size type.
	constexpr auto glsizei_max = static_cast<std::size_t>(std::numeric_limits<GLsizei>::max());
	GLsizei const length = static_cast<GLsizei>(label.size() < glsizei_max ? label.size() : glsizei_max);
	glObjectLabel(identifier, object, length, label.data());
}

} // namespace sfm::gfx::detail
