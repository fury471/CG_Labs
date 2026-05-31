#pragma once

#include "sandbox/scene/PointCloudMesher.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace sfm::app
{

struct StartupOptions final
{
	std::filesystem::path project_manifest_path{};
	std::filesystem::path capture_baseline_path{};
	std::filesystem::path build_mesh_path{};
	sfm::scene::PointToMeshBuildOptions mesh_build_options{};
	bool show_help{ false };
	bool validate_install{ false };
	std::vector<std::string> messages{};
};

struct StartupValidationResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

struct MeshBuildCommandResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] StartupOptions parse_startup_options(int argc, char** argv);
[[nodiscard]] StartupValidationResult validate_startup_assets(StartupOptions const& options);
[[nodiscard]] MeshBuildCommandResult build_mesh_from_startup_project(StartupOptions const& options);
void print_startup_help();

} // namespace sfm::app
