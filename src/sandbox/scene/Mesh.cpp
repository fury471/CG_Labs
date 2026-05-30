#include "Mesh.hpp"

#include <algorithm>
#include <utility>

namespace sfm::scene
{

Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<MeshTriangle> triangles)
	: m_positions(std::move(positions))
	, m_triangles(std::move(triangles))
{
}

MeshStatistics Mesh::statistics() const noexcept
{
	MeshStatistics result{};
	result.vertex_count = m_positions.size();
	result.triangle_count = m_triangles.size();
	result.approximate_cpu_bytes = m_positions.size() * sizeof(glm::vec3) + m_triangles.size() * sizeof(MeshTriangle);
	if (m_positions.empty())
		return result;

	glm::vec3 bounds_min = m_positions.front();
	glm::vec3 bounds_max = m_positions.front();
	for (glm::vec3 const& position : m_positions) {
		bounds_min = glm::min(bounds_min, position);
		bounds_max = glm::max(bounds_max, position);
	}

	result.has_bounds = true;
	result.bounds_min = bounds_min;
	result.bounds_max = bounds_max;
	result.bounds_extent = bounds_max - bounds_min;
	return result;
}

} // namespace sfm::scene
