#pragma once

#include "Material.hpp"
#include "Mesh.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace sfm::gfx
{

struct RenderSubmission final
{
	MeshHandle mesh{};
	MaterialDescriptor material{};
	glm::vec3 point_solid_colour{ 0.85f, 0.90f, 1.0f };
	float point_size{ 1.0f };
	int point_colour_mode{ 0 };
};

/// Explicit submission list for a frame.
///
/// M15 keeps ownership and actual GL drawing inside `Renderer`, but this queue
/// makes the draw order explicit and prepares the codebase for later mesh/image
/// renderables without giving scene objects direct OpenGL calls.
class RenderQueue final
{
public:
	void clear() { m_submissions.clear(); }
	void submit(RenderSubmission submission) { m_submissions.push_back(submission); }

	[[nodiscard]] bool empty() const noexcept { return m_submissions.empty(); }
	[[nodiscard]] std::vector<RenderSubmission> const& submissions() const noexcept { return m_submissions; }

private:
	std::vector<RenderSubmission> m_submissions{};
};

} // namespace sfm::gfx
