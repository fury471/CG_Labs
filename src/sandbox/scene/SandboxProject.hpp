#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

struct SandboxProject final
{
	std::filesystem::path manifest_path{};
	std::filesystem::path point_cloud_path{};
	std::filesystem::path camera_pose_path{};
	std::filesystem::path surface_path{};
	std::filesystem::path image_path{};
};

struct SandboxProjectLoadResult final
{
	SandboxProject project{};
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] SandboxProject make_sample_project(std::filesystem::path const& resources_root);
[[nodiscard]] SandboxProjectLoadResult load_sandbox_project(std::filesystem::path const& manifest_path);

} // namespace sfm::scene
