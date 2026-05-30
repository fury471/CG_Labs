#include "PointCloud.hpp"

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <utility>

namespace sfm::scene
{

PointCloud::PointCloud(std::vector<PointSample> points)
	: m_points(std::move(points))
{
}

PointCloud PointCloud::make_debug_cluster()
{
	constexpr int ring_count = 9;
	constexpr int points_per_ring = 32;
	constexpr float base_radius = 1.75f;
	constexpr float height_step = 0.22f;

	std::vector<PointSample> points;
	points.reserve(static_cast<std::size_t>(ring_count * points_per_ring));

	for (int ring = 0; ring < ring_count; ++ring) {
		float const y = 0.55f + static_cast<float>(ring) * height_step;
		float const ring_fraction = static_cast<float>(ring) / static_cast<float>(ring_count - 1);
		float const local_radius = base_radius * (0.35f + 0.65f * (1.0f - 0.5f * ring_fraction));

		for (int point = 0; point < points_per_ring; ++point) {
			float const t = static_cast<float>(point) / static_cast<float>(points_per_ring);
			float const angle = t * glm::two_pi<float>() + ring_fraction * 0.7f;
			float const wobble = 0.15f * std::sin(3.0f * angle + static_cast<float>(ring));

			points.push_back(PointSample{
				glm::vec3{
					(local_radius + wobble) * std::cos(angle),
					y,
					(local_radius - wobble) * std::sin(angle)
				},
				glm::vec3{
					0.25f + 0.75f * t,
					0.35f + 0.55f * ring_fraction,
					1.0f - 0.65f * t
				}
			});
		}
	}

	return PointCloud{ std::move(points) };
}

} // namespace sfm::scene
