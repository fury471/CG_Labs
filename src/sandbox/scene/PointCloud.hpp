#pragma once

#include <glm/glm.hpp>

#include <span>
#include <vector>

namespace sfm::scene
{

/// One CPU-side point sample used by the sandbox point-cloud renderer.
///
/// The model deliberately stores position and colour only. Normals, confidence,
/// reprojection error, track id and image observations are useful SfM metadata,
/// but adding them before a real importer exists would make the data contract
/// speculative.
struct PointSample final
{
	glm::vec3 position{};
	glm::vec3 colour{};
};

/// Minimal CPU-side point cloud container.
///
/// `PointCloud` is renderer-independent: it owns semantic visualization data,
/// not OpenGL buffers. The renderer can upload a span of samples, while later
/// milestones can populate the same model from SfM/reconstruction sources.
class PointCloud final
{
public:
	PointCloud() = default;
	explicit PointCloud(std::vector<PointSample> points);

	[[nodiscard]] bool empty() const noexcept { return m_points.empty(); }
	[[nodiscard]] std::size_t size() const noexcept { return m_points.size(); }
	[[nodiscard]] std::span<PointSample const> points() const noexcept { return m_points; }

	/// Builds a deterministic sample cloud for validating the renderer before any
	/// file importer or SfM pipeline exists. The pattern forms a small coloured
	/// spatial cluster above the grid so perspective camera movement is visible.
	[[nodiscard]] static PointCloud make_debug_cluster();

private:
	std::vector<PointSample> m_points{};
};

} // namespace sfm::scene
