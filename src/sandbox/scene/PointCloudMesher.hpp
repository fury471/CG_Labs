#pragma once

#include "PointCloud.hpp"
#include "SurfaceImport.hpp"

#include <string>
#include <vector>

namespace sfm::scene
{

enum class PointMeshProjectionPlane
{
	Auto = 0,
	XY = 1,
	XZ = 2,
	YZ = 3,
};

struct PointToMeshBuildOptions final
{
	PointMeshProjectionPlane projection{ PointMeshProjectionPlane::Auto };
	float weld_epsilon{ 0.0001f };
	float minimum_triangle_area{ 0.000001f };
	float maximum_edge_length{ 0.0f };
	std::size_t maximum_projected_points{ 1000u };
};

struct PointToMeshBuildResult final
{
	SurfaceMesh mesh{};
	bool succeeded{ false };
	std::size_t input_point_count{ 0u };
	std::size_t unique_point_count{ 0u };
	std::size_t duplicate_point_count{ 0u };
	std::size_t invalid_point_count{ 0u };
	std::size_t rejected_triangle_count{ 0u };
	PointMeshProjectionPlane resolved_projection{ PointMeshProjectionPlane::Auto };
	std::string algorithm{};
	std::vector<std::string> messages{};
};

[[nodiscard]] char const* projection_plane_label(PointMeshProjectionPlane projection) noexcept;
[[nodiscard]] PointMeshProjectionPlane choose_projection_plane(PointCloudStatistics const& statistics) noexcept;
[[nodiscard]] PointToMeshBuildResult build_surface_mesh_from_point_cloud(PointCloud const& point_cloud,
                                                                         PointToMeshBuildOptions const& options);

} // namespace sfm::scene
