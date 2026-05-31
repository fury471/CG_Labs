#include "PointCloudMesher.hpp"

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace sfm::scene
{
namespace
{

struct ProjectedPoint final
{
	glm::vec2 projected{};
	glm::vec3 position_sum{};
	std::size_t sample_count{ 0u };
};

struct Triangle2D final
{
	std::size_t a{ 0u };
	std::size_t b{ 0u };
	std::size_t c{ 0u };
};

struct Edge final
{
	std::size_t a{ 0u };
	std::size_t b{ 0u };
};

void add_message(PointToMeshBuildResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

bool finite(glm::vec3 value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

glm::vec2 project(glm::vec3 value, PointMeshProjectionPlane projection) noexcept
{
	switch (projection) {
	case PointMeshProjectionPlane::XY: return { value.x, value.y };
	case PointMeshProjectionPlane::XZ: return { value.x, value.z };
	case PointMeshProjectionPlane::YZ: return { value.y, value.z };
	case PointMeshProjectionPlane::Auto: break;
	}
	return { value.x, value.z };
}

float signed_area_twice(glm::vec2 a, glm::vec2 b, glm::vec2 c) noexcept
{
	glm::vec2 const ab = b - a;
	glm::vec2 const ac = c - a;
	return ab.x * ac.y - ab.y * ac.x;
}

Triangle2D make_oriented_triangle(std::size_t a,
                                  std::size_t b,
                                  std::size_t c,
                                  std::vector<glm::vec2> const& points) noexcept
{
	if (signed_area_twice(points[a], points[b], points[c]) < 0.0f)
		std::swap(b, c);
	return Triangle2D{ a, b, c };
}

bool point_is_inside_circumcircle(glm::vec2 p,
                                  Triangle2D triangle,
                                  std::vector<glm::vec2> const& points,
                                  float epsilon) noexcept
{
	glm::vec2 const a = points[triangle.a];
	glm::vec2 const b = points[triangle.b];
	glm::vec2 const c = points[triangle.c];
	float const orientation = signed_area_twice(a, b, c);
	if (std::abs(orientation) <= epsilon)
		return false;

	float const ax = a.x - p.x;
	float const ay = a.y - p.y;
	float const bx = b.x - p.x;
	float const by = b.y - p.y;
	float const cx = c.x - p.x;
	float const cy = c.y - p.y;

	float const det = (ax * ax + ay * ay) * (bx * cy - cx * by) -
	                  (bx * bx + by * by) * (ax * cy - cx * ay) +
	                  (cx * cx + cy * cy) * (ax * by - bx * ay);
	return orientation > 0.0f ? det > epsilon : det < -epsilon;
}

bool same_undirected_edge(Edge lhs, Edge rhs) noexcept
{
	return (lhs.a == rhs.a && lhs.b == rhs.b) || (lhs.a == rhs.b && lhs.b == rhs.a);
}

void add_boundary_edge(std::vector<Edge>& boundary, Edge edge)
{
	auto const found = std::find_if(boundary.begin(), boundary.end(), [edge](Edge candidate) {
		return same_undirected_edge(candidate, edge);
	});
	if (found != boundary.end()) {
		boundary.erase(found);
		return;
	}
	boundary.push_back(edge);
}

float projected_edge_length(glm::vec2 a, glm::vec2 b) noexcept
{
	return glm::length(b - a);
}

bool triangle_exceeds_edge_limit(Triangle2D triangle,
                                 std::vector<glm::vec2> const& points,
                                 float maximum_edge_length) noexcept
{
	if (maximum_edge_length <= 0.0f)
		return false;
	return projected_edge_length(points[triangle.a], points[triangle.b]) > maximum_edge_length ||
	       projected_edge_length(points[triangle.b], points[triangle.c]) > maximum_edge_length ||
	       projected_edge_length(points[triangle.c], points[triangle.a]) > maximum_edge_length;
}

std::vector<Triangle2D> triangulate_projected_points(std::vector<glm::vec2> points,
                                                     float minimum_triangle_area,
                                                     float maximum_edge_length,
                                                     std::size_t& rejected_triangle_count)
{
	rejected_triangle_count = 0u;
	if (points.size() < 3u)
		return {};

	glm::vec2 bounds_min = points.front();
	glm::vec2 bounds_max = points.front();
	for (glm::vec2 point : points) {
		bounds_min = glm::min(bounds_min, point);
		bounds_max = glm::max(bounds_max, point);
	}

	glm::vec2 const extent = bounds_max - bounds_min;
	float const span = std::max(extent.x, extent.y);
	if (span <= std::numeric_limits<float>::epsilon())
		return {};

	glm::vec2 const centre = 0.5f * (bounds_min + bounds_max);
	float const super_extent = span * 32.0f;
	std::size_t const input_count = points.size();
	points.push_back(centre + glm::vec2{ -super_extent, -super_extent });
	points.push_back(centre + glm::vec2{ 0.0f, super_extent });
	points.push_back(centre + glm::vec2{ super_extent, -super_extent });

	std::vector<Triangle2D> triangles;
	triangles.push_back(make_oriented_triangle(input_count, input_count + 1u, input_count + 2u, points));

	float const circumcircle_epsilon = std::max(minimum_triangle_area * 0.01f, 0.00000001f);
	for (std::size_t point_index = 0u; point_index < input_count; ++point_index) {
		std::vector<Edge> boundary;
		std::vector<Triangle2D> kept_triangles;
		kept_triangles.reserve(triangles.size() + 2u);

		for (Triangle2D triangle : triangles) {
			if (!point_is_inside_circumcircle(points[point_index], triangle, points, circumcircle_epsilon)) {
				kept_triangles.push_back(triangle);
				continue;
			}

			add_boundary_edge(boundary, Edge{ triangle.a, triangle.b });
			add_boundary_edge(boundary, Edge{ triangle.b, triangle.c });
			add_boundary_edge(boundary, Edge{ triangle.c, triangle.a });
		}

		for (Edge edge : boundary) {
			if (edge.a == point_index || edge.b == point_index)
				continue;
			float const area = 0.5f * std::abs(signed_area_twice(points[edge.a], points[edge.b], points[point_index]));
			if (area <= minimum_triangle_area) {
				++rejected_triangle_count;
				continue;
			}
			kept_triangles.push_back(make_oriented_triangle(edge.a, edge.b, point_index, points));
		}
		triangles = std::move(kept_triangles);
	}

	std::vector<Triangle2D> output;
	output.reserve(triangles.size());
	for (Triangle2D triangle : triangles) {
		if (triangle.a >= input_count || triangle.b >= input_count || triangle.c >= input_count)
			continue;
		float const area = 0.5f * std::abs(signed_area_twice(points[triangle.a], points[triangle.b], points[triangle.c]));
		if (area <= minimum_triangle_area || triangle_exceeds_edge_limit(triangle, points, maximum_edge_length)) {
			++rejected_triangle_count;
			continue;
		}
		output.push_back(triangle);
	}
	return output;
}

} // namespace

char const* projection_plane_label(PointMeshProjectionPlane projection) noexcept
{
	switch (projection) {
	case PointMeshProjectionPlane::Auto: return "auto";
	case PointMeshProjectionPlane::XY: return "XY";
	case PointMeshProjectionPlane::XZ: return "XZ";
	case PointMeshProjectionPlane::YZ: return "YZ";
	}
	return "unknown";
}

PointMeshProjectionPlane choose_projection_plane(PointCloudStatistics const& statistics) noexcept
{
	if (!statistics.has_bounds)
		return PointMeshProjectionPlane::XZ;

	glm::vec3 const extent = statistics.bounds_extent;
	if (extent.y <= extent.x && extent.y <= extent.z)
		return PointMeshProjectionPlane::XZ;
	if (extent.z <= extent.x && extent.z <= extent.y)
		return PointMeshProjectionPlane::XY;
	return PointMeshProjectionPlane::YZ;
}

PointToMeshBuildResult build_surface_mesh_from_point_cloud(PointCloud const& point_cloud,
                                                           PointToMeshBuildOptions const& options)
{
	PointToMeshBuildResult result{};
	result.input_point_count = point_cloud.size();
	result.algorithm = "projected Delaunay-style triangulation";
	if (point_cloud.size() < 3u) {
		add_message(result, "Point-to-mesh build failed: at least three points are required");
		return result;
	}

	PointCloudStatistics const statistics = point_cloud.statistics();
	PointMeshProjectionPlane const projection = options.projection == PointMeshProjectionPlane::Auto
		? choose_projection_plane(statistics)
		: options.projection;
	result.resolved_projection = projection;

	float const weld_epsilon = std::max(options.weld_epsilon, 0.0000001f);
	float const minimum_triangle_area = std::max(options.minimum_triangle_area, 0.000000001f);
	std::map<std::pair<long long, long long>, std::size_t> projected_lookup;
	std::vector<ProjectedPoint> projected_points;
	projected_points.reserve(point_cloud.size());

	for (PointSample const& sample : point_cloud.points()) {
		if (!finite(sample.position)) {
			++result.invalid_point_count;
			continue;
		}
		glm::vec2 const projected = project(sample.position, projection);
		auto const key = std::make_pair(static_cast<long long>(std::llround(projected.x / weld_epsilon)),
		                                static_cast<long long>(std::llround(projected.y / weld_epsilon)));
		auto const found = projected_lookup.find(key);
		if (found == projected_lookup.end()) {
			projected_lookup.emplace(key, projected_points.size());
			projected_points.push_back(ProjectedPoint{ projected, sample.position, 1u });
			continue;
		}
		ProjectedPoint& existing = projected_points[found->second];
		existing.projected += projected;
		existing.position_sum += sample.position;
		++existing.sample_count;
		++result.duplicate_point_count;
	}

	result.unique_point_count = projected_points.size();
	if (projected_points.size() < 3u) {
		add_message(result, "Point-to-mesh build failed: fewer than three unique projected samples remain after welding");
		add_message(result, "Projection: " + std::string{ projection_plane_label(projection) });
		return result;
	}
	std::size_t const maximum_projected_points = std::max<std::size_t>(options.maximum_projected_points, 3u);
	if (projected_points.size() > maximum_projected_points) {
		add_message(result, "Point-to-mesh build failed: " + std::to_string(projected_points.size()) +
		                    " unique projected samples exceed the current safety limit of " +
		                    std::to_string(maximum_projected_points));
		add_message(result, "Increase the max point limit only for small/controlled data, or downsample/filter the point cloud first");
		add_message(result, "Projection: " + std::string{ projection_plane_label(projection) });
		return result;
	}

	std::vector<glm::vec2> projected_vertices;
	projected_vertices.reserve(projected_points.size());
	result.mesh.positions.reserve(projected_points.size());
	for (ProjectedPoint const& point : projected_points) {
		float const count = static_cast<float>(point.sample_count);
		projected_vertices.push_back(point.projected / count);
		result.mesh.positions.push_back(point.position_sum / count);
	}

	std::vector<Triangle2D> const triangles = triangulate_projected_points(projected_vertices,
	                                                                       minimum_triangle_area,
	                                                                       options.maximum_edge_length,
	                                                                       result.rejected_triangle_count);
	result.mesh.triangles.reserve(triangles.size());
	for (Triangle2D triangle : triangles)
		result.mesh.triangles.push_back(SurfaceTriangle{ triangle.a, triangle.b, triangle.c });

	add_message(result, "Point-to-mesh builder: " + result.algorithm);
	add_message(result, "Projection: " + std::string{ projection_plane_label(projection) });
	add_message(result, "Input points: " + std::to_string(result.input_point_count));
	add_message(result, "Unique projected points: " + std::to_string(result.unique_point_count));
	add_message(result, "Duplicate projected points welded: " + std::to_string(result.duplicate_point_count));
	add_message(result, "Invalid input points skipped: " + std::to_string(result.invalid_point_count));
	add_message(result, "Rejected triangles: " + std::to_string(result.rejected_triangle_count));
	if (result.unique_point_count < 4u)
		add_message(result, "Quality warning: fewer than four unique projected points leaves little surface evidence");
	if (result.duplicate_point_count > 0u && result.unique_point_count > 0u &&
	    result.duplicate_point_count >= result.unique_point_count / 2u)
		add_message(result, "Quality warning: many samples were welded; check projection plane and weld tolerance");
	if (result.rejected_triangle_count > 0u)
		add_message(result, "Quality warning: triangle filters removed candidate faces; inspect minimum area and edge-length settings");

	if (result.mesh.empty()) {
		add_message(result, "Point-to-mesh build failed: triangulation produced no valid triangles");
		return result;
	}

	result.succeeded = true;
	add_message(result, "Generated surface mesh: " + std::to_string(result.mesh.vertex_count()) + " vertices, " +
	                    std::to_string(result.mesh.triangle_count()) + " triangles");
	return result;
}

} // namespace sfm::scene
