#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace sfm::scene
{

/// One reconstructed or synthetic camera pose in world space.
///
/// The transform follows the usual graphics convention used by the renderer:
/// local camera space looks along negative Z, with positive Y as camera up. The
/// matrix maps local camera-space points into world space.
struct CameraPose final
{
	glm::mat4 camera_to_world{ 1.0f };
	glm::vec3 colour{ 1.0f, 0.9f, 0.2f };
};

/// CPU-side container for camera poses and trajectory visualization.
///
/// This type is renderer-independent. Later milestones can populate it from SfM
/// reconstruction output without changing the renderer-facing data contract.
class CameraPoseSet final
{
public:
	CameraPoseSet() = default;
	explicit CameraPoseSet(std::vector<CameraPose> poses);

	[[nodiscard]] bool empty() const noexcept { return m_poses.empty(); }
	[[nodiscard]] std::size_t size() const noexcept { return m_poses.size(); }
	[[nodiscard]] std::span<CameraPose const> poses() const noexcept { return m_poses; }

	/// Builds a deterministic camera path around the sample point-cloud region so
	/// frustum and trajectory rendering can be validated before a real SfM import.
	[[nodiscard]] static CameraPoseSet make_debug_orbit();

private:
	std::vector<CameraPose> m_poses{};
};

} // namespace sfm::scene
