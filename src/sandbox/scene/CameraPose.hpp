#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace sfm::scene
{

/// Source-side information preserved beside a converted camera pose.
///
/// Metadata is intentionally renderer-independent. It lets the sandbox explain
/// where a pose came from, which convention was converted and which source image
/// or camera record it represents without changing the graphics data contract.
struct CameraPoseMetadata final
{
	std::size_t index{ 0u };
	std::size_t source_line{ 0u };
	int source_id{ -1 };
	int camera_id{ -1 };
	std::string source_file{};
	std::string source_format{ "procedural" };
	std::string source_convention{ "graphics camera-to-world, local -Z forward, +Y up" };
	std::string image_name{};
};

/// One reconstructed or synthetic camera pose in world space.
///
/// The transform follows the usual graphics convention used by the renderer:
/// local camera space looks along negative Z, with positive Y as camera up. The
/// matrix maps local camera-space points into world space.
struct CameraPose final
{
	glm::mat4 camera_to_world{ 1.0f };
	glm::vec3 colour{ 1.0f, 0.9f, 0.2f };
	CameraPoseMetadata metadata{};
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
