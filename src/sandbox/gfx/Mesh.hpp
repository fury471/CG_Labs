#pragma once

#include <glad/gl.h>

#include <string_view>

namespace sfm::gfx
{

/// Minimal draw-ready mesh descriptor owned by the renderer.
///
/// The descriptor does not own GPU resources. It is a compact submission record
/// pointing at renderer-owned VAOs/buffers and naming the primitive/count needed
/// for a draw call. Later milestones can replace this with richer mesh assets
/// without changing the principle that scene data does not issue raw GL draws.
struct MeshHandle final
{
	GLuint vertex_array{ 0u };
	GLenum primitive{ GL_LINES };
	GLsizei vertex_count{ 0 };
	std::string_view debug_name{};
};

} // namespace sfm::gfx
