#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx::detail
{

/// Assigns a human-readable name to a GPU object when OpenGL debug labels are
/// available. Labels make captures and driver diagnostics refer to the role of
/// an object instead of only its numeric handle.
void label_object(GLenum identifier, GLuint object, std::string_view label) noexcept;

} // namespace sfm::gfx::detail
