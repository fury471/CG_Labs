#include "CameraPose.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <string>
#include <utility>

namespace sfm::scene
{

CameraPoseSet::CameraPoseSet(std::vector<CameraPose> poses)
	: m_poses(std::move(poses))
{
}

CameraPoseSet CameraPoseSet::make_debug_orbit()
{
	constexpr int pose_count = 8;
	constexpr float radius = 4.2f;
	constexpr float height = 1.8f;

	std::vector<CameraPose> poses;
	poses.reserve(static_cast<std::size_t>(pose_count));

	glm::vec3 const target{ 0.0f, 1.0f, 0.0f };
	for (int index = 0; index < pose_count; ++index) {
		float const t = static_cast<float>(index) / static_cast<float>(pose_count);
		float const angle = t * glm::two_pi<float>();
		glm::vec3 const eye{
			radius * std::cos(angle),
			height + 0.35f * std::sin(angle * 2.0f),
			radius * std::sin(angle)
		};

		// glm::lookAt returns world-to-camera. The pose model stores camera-to-world,
		// so invert the view matrix. This keeps the data model aligned with how SfM
		// camera poses are commonly visualized in world coordinates.
		glm::mat4 const world_to_camera = glm::lookAt(eye, target, glm::vec3{ 0.0f, 1.0f, 0.0f });
		CameraPose pose{};
		pose.camera_to_world = glm::inverse(world_to_camera);
		pose.colour = glm::vec3{ 1.0f, 0.75f - 0.25f * t, 0.15f + 0.75f * t };
		pose.metadata.index = static_cast<std::size_t>(index);
		pose.metadata.source_id = index;
		pose.metadata.source_format = "procedural debug orbit";
		pose.metadata.source_convention = "generated graphics camera-to-world, local -Z forward, +Y up";
		pose.metadata.image_name = "debug_orbit_" + std::to_string(index);
		poses.push_back(std::move(pose));
	}

	return CameraPoseSet{ std::move(poses) };
}

} // namespace sfm::scene
