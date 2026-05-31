#pragma once

#include "sandbox/scene/PointCloudMesher.hpp"
#include "sandbox/scene/SurfaceImport.hpp"

#include <filesystem>
#include <string>

namespace sfm::app
{

struct BaselineCaptureMetadata final
{
	int framebuffer_width{ 0 };
	int framebuffer_height{ 0 };
	std::filesystem::path project_manifest{};
	bool project_loaded{ false };

	std::string point_cloud_source{};
	std::size_t point_count{ 0u };

	std::string camera_pose_source{};
	std::size_t camera_pose_count{ 0u };
	bool camera_poses_visible{ false };

	std::string surface_source{};
	std::string surface_kind{};
	std::string surface_source_format{};
	bool surface_visible{ false };
	sfm::scene::SurfaceStatistics surface_statistics{};
	sfm::scene::PointToMeshBuildResult const* generated_mesh_build{ nullptr };

	std::string image_source{};
	bool image_visible{ false };
	int image_width{ 0 };
	int image_height{ 0 };

	int marker_count{ 0 };
	bool marker_visible{ false };
	bool marker_instancing{ false };

	int point_colour_mode{ 0 };
	float point_size{ 0.0f };
};

[[nodiscard]] bool write_baseline_capture_metadata(std::filesystem::path const& metadata_path,
                                                   BaselineCaptureMetadata const& metadata,
                                                   std::string& error_message);

} // namespace sfm::app
