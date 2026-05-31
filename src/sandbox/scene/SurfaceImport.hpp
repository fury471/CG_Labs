#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace sfm::scene
{

struct SurfaceTriangle final { std::size_t a{0u}; std::size_t b{0u}; std::size_t c{0u}; };

struct SurfaceStatistics final
{
	std::size_t vertex_count{0u};
	std::size_t triangle_count{0u};
	std::size_t approximate_cpu_bytes{0u};
	bool has_bounds{false};
	glm::vec3 bounds_min{};
	glm::vec3 bounds_max{};
	glm::vec3 bounds_extent{};
};

struct SurfaceMesh final
{
	std::vector<glm::vec3> positions{};
	std::vector<SurfaceTriangle> triangles{};
	[[nodiscard]] bool empty() const noexcept { return positions.empty() || triangles.empty(); }
	[[nodiscard]] std::size_t vertex_count() const noexcept { return positions.size(); }
	[[nodiscard]] std::size_t triangle_count() const noexcept { return triangles.size(); }
	[[nodiscard]] std::span<glm::vec3 const> vertices() const noexcept { return positions; }
	[[nodiscard]] std::span<SurfaceTriangle const> faces() const noexcept { return triangles; }
};

[[nodiscard]] SurfaceStatistics statistics_for(SurfaceMesh const& mesh) noexcept;

struct SurfaceImportResult final
{
	SurfaceMesh mesh{};
	bool succeeded{false};
	std::size_t skipped_lines{0u};
	std::vector<std::string> messages{};
	std::string source_format{};
};

[[nodiscard]] SurfaceImportResult import_surface_from_obj(std::filesystem::path const& path);
[[nodiscard]] SurfaceImportResult import_surface(std::filesystem::path const& path);

} // namespace sfm::scene
