#include "SandboxProject.hpp"

#include <array>
#include <fstream>
#include <sstream>
#include <string_view>
#include <utility>

namespace sfm::scene
{
namespace
{

constexpr char const* kPointCloudKey = "point_cloud";
constexpr char const* kCameraPosesKey = "camera_poses";
constexpr char const* kSurfaceKey = "surface";
constexpr char const* kImageKey = "image";

void add_message(SandboxProjectLoadResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

std::string trim(std::string_view text)
{
	auto const first = text.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos)
		return {};
	auto const last = text.find_last_not_of(" \t\r\n");
	return std::string{ text.substr(first, last - first + 1u) };
}

std::filesystem::path resolve_manifest_path(std::filesystem::path const& manifest_path, std::string_view raw_value)
{
	std::filesystem::path value{ trim(raw_value) };
	if (value.is_absolute())
		return value.lexically_normal();
	return (manifest_path.parent_path() / value).lexically_normal();
}

bool assign_key(SandboxProject& project,
                std::filesystem::path const& manifest_path,
                std::string_view key,
                std::string_view value,
                SandboxProjectLoadResult& result,
                std::size_t line_number)
{
	if (value.empty()) {
		add_message(result, "Project manifest line " + std::to_string(line_number) + " has an empty value for key '" + std::string{ key } + "'");
		return false;
	}

	std::filesystem::path const resolved = resolve_manifest_path(manifest_path, value);
	if (key == kPointCloudKey) {
		project.point_cloud_path = resolved;
		return true;
	}
	if (key == kCameraPosesKey) {
		project.camera_pose_path = resolved;
		return true;
	}
	if (key == kSurfaceKey) {
		project.surface_path = resolved;
		return true;
	}
	if (key == kImageKey) {
		project.image_path = resolved;
		return true;
	}

	add_message(result, "Project manifest line " + std::to_string(line_number) + " uses unsupported key '" + std::string{ key } + "'");
	return false;
}

void require_path(SandboxProjectLoadResult& result, std::filesystem::path const& path, std::string_view key)
{
	if (path.empty())
		add_message(result, "Project manifest is missing required key '" + std::string{ key } + "'");
}

} // namespace

SandboxProject make_sample_project(std::filesystem::path const& resources_root)
{
	SandboxProject project{};
	project.manifest_path = resources_root / "sandbox/default_project.sfmproj";
	project.point_cloud_path = resources_root / "sandbox/sample_point_cloud_ascii.ply";
	project.camera_pose_path = resources_root / "sandbox/sample_camera_poses.txt";
	project.surface_path = resources_root / "sandbox/sample_surface.obj";
	project.image_path = resources_root / "sandbox/sample_camera_image.ppm";
	return project;
}

SandboxProjectLoadResult load_sandbox_project(std::filesystem::path const& manifest_path)
{
	SandboxProjectLoadResult result{};
	result.project.manifest_path = manifest_path;

	std::ifstream file{ manifest_path };
	if (!file) {
		add_message(result, "Project manifest load failed: could not open '" + manifest_path.string() + "'");
		return result;
	}

	std::string line;
	std::size_t line_number = 0u;
	while (std::getline(file, line)) {
		++line_number;

		auto const comment_begin = line.find('#');
		if (comment_begin != std::string::npos)
			line.erase(comment_begin);
		line = trim(line);
		if (line.empty())
			continue;

		auto const separator = line.find('=');
		if (separator == std::string::npos) {
			add_message(result, "Project manifest line " + std::to_string(line_number) + " is missing '='");
			continue;
		}

		std::string const key = trim(std::string_view{ line }.substr(0u, separator));
		std::string const value = trim(std::string_view{ line }.substr(separator + 1u));
		if (key.empty()) {
			add_message(result, "Project manifest line " + std::to_string(line_number) + " has an empty key");
			continue;
		}

		assign_key(result.project, manifest_path, key, value, result, line_number);
	}

	std::size_t const diagnostic_count_before_required_checks = result.messages.size();
	require_path(result, result.project.point_cloud_path, kPointCloudKey);
	require_path(result, result.project.camera_pose_path, kCameraPosesKey);
	require_path(result, result.project.surface_path, kSurfaceKey);
	require_path(result, result.project.image_path, kImageKey);

	if (result.messages.size() != diagnostic_count_before_required_checks) {
		add_message(result, "Project manifest load failed: missing required startup paths");
		return result;
	}
	if (!result.messages.empty()) {
		add_message(result, "Project manifest load failed: invalid manifest syntax or unsupported keys");
		return result;
	}

	result.succeeded = true;
	add_message(result, "Loaded sandbox project manifest '" + manifest_path.string() + "'");
	add_message(result, "Point cloud: " + result.project.point_cloud_path.string());
	add_message(result, "Camera poses: " + result.project.camera_pose_path.string());
	add_message(result, "Surface: " + result.project.surface_path.string());
	add_message(result, "Image: " + result.project.image_path.string());
	return result;
}

} // namespace sfm::scene
