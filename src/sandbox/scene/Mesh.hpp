#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace sfm::scene
{

struct MeshTriangle final
{
	std::size_t a{ 0u };
	std::size_t b{ 0u };
	std::size_t c{ 0u };
};

struct MeshStatistics final
{
	std::size_t vertex_count{ 0u };
	std::size_t triangle_count{ 0u };
	std::size_t approximate_cpu_bytes{ 0u };
	bool has_bounds{ false };
	glm::vec3 bounds_min{};
	glm::vec3 bounds_max{};
	glm::vec3 bounds_extent{};
};

/// Minimal CPU-side triangle mesh used for reconstructed surface inspection.
///
/// M16 deliberately stores only vertex positions and triangle indices. Normals,
/// UVs, materials and topology metadata are important later, but this first mesh
/// milestone only needs a trustworthy surface dataset that can be uploaded
/// through the M15 renderer submission path.
class Mesh final
{
public:
	Mesh() = default;
	Mesh(std::vector<glm::vec3> positions, std::vector<MeshTriangle> triangles);

	[[nodiscard]] bool empty() const noexcept { return m_positions.empty() || m_triangles.empty(); }
	[[nodiscard]] std::size_t vertex_count() const noexcept { return m_positions.size(); }
	[[nodiscard]] std::size_t triangle_count() const noexcept { return m_triangles.size(); }
	[[nodiscard]] std::span<glm::vec3 const> positions() const noexcept { return m_positions; }
	[[nodiscard]] std::span<MeshTriangle const> triangles() const noexcept { return m_triangles; }
	[[nodiscard]] MeshStatistics statistics() const noexcept;

	[[nodiscard]] static Mesh make_debug_surface();

private:
	std::vector<glm::vec3> m_positions{};
	std::vector<MeshTriangle> m_triangles{};
};

} // namespace sfm::scene
