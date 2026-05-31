#include "SfmSandboxStartup.hpp"

#include "config.hpp"
#include "sandbox/scene/PointCloudLoader.hpp"
#include "sandbox/scene/SandboxProject.hpp"
#include "sandbox/scene/SurfaceExport.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <locale>
#include <sstream>
#include <string_view>
#include <system_error>

namespace sfm::app
{
namespace
{

constexpr char const* kDefaultProjectResource = "sandbox/default_project.sfmproj";

constexpr std::array<char const*, 6> kRequiredShaderFiles{
	"shaders/sandbox/line.vert",
	"shaders/sandbox/line.frag",
	"shaders/sandbox/point_cloud.vert",
	"shaders/sandbox/image_plane.vert",
	"shaders/sandbox/image_plane.frag",
	"shaders/sandbox/instanced_marker.vert",
};

std::filesystem::path resolve_shader_file_for_validation(std::filesystem::path const& path)
{
	std::error_code error;
	if (std::filesystem::is_regular_file(path, error))
		return path;

	std::string const generic_path = path.generic_string();
	constexpr std::string_view shader_prefix = "shaders/";
	if (generic_path.rfind(shader_prefix, 0u) != 0u)
		return path;
	return config::shaders_path(generic_path.substr(shader_prefix.size()));
}

void validate_existing_file(StartupValidationResult& result, std::string const& label, std::filesystem::path const& path)
{
	std::error_code error;
	if (std::filesystem::is_regular_file(path, error)) {
		result.messages.emplace_back(label + " ok: " + path.string());
		return;
	}
	result.succeeded = false;
	result.messages.emplace_back(label + " missing: " + path.string());
}

void validate_optional_file(StartupValidationResult& result, std::string const& label, std::filesystem::path const& path)
{
	if (path.empty()) {
		result.messages.emplace_back(label + " disabled");
		return;
	}
	validate_existing_file(result, label, path);
}

bool parse_float_argument(std::string const& text, float& value)
{
	std::istringstream stream{ text };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof() && std::isfinite(value);
}

bool parse_projection_argument(std::string text, sfm::scene::PointMeshProjectionPlane& projection)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	if (text == "auto") {
		projection = sfm::scene::PointMeshProjectionPlane::Auto;
		return true;
	}
	if (text == "xy") {
		projection = sfm::scene::PointMeshProjectionPlane::XY;
		return true;
	}
	if (text == "xz") {
		projection = sfm::scene::PointMeshProjectionPlane::XZ;
		return true;
	}
	if (text == "yz") {
		projection = sfm::scene::PointMeshProjectionPlane::YZ;
		return true;
	}
	return false;
}

void append_prefixed(std::vector<std::string>& destination, std::string const& prefix, std::vector<std::string> const& messages)
{
	for (std::string const& message : messages)
		destination.emplace_back(prefix + message);
}

std::string mesh_build_export_comment(std::filesystem::path const& point_cloud_path,
                                      sfm::scene::PointToMeshBuildResult const& mesh)
{
	std::ostringstream stream;
	stream << "generated from " << point_cloud_path.string() << '\n';
	stream << "algorithm: " << mesh.algorithm << '\n';
	stream << "projection: " << sfm::scene::projection_plane_label(mesh.resolved_projection) << '\n';
	stream << "input_points: " << mesh.input_point_count << '\n';
	stream << "unique_projected_points: " << mesh.unique_point_count << '\n';
	stream << "duplicate_projected_points: " << mesh.duplicate_point_count << '\n';
	stream << "invalid_input_points: " << mesh.invalid_point_count << '\n';
	stream << "rejected_triangles: " << mesh.rejected_triangle_count;
	return stream.str();
}

} // namespace

StartupOptions parse_startup_options(int argc, char** argv)
{
	StartupOptions options{};
	options.project_manifest_path = config::resources_path(kDefaultProjectResource);
	for (int i = 1; i < argc; ++i) {
		std::string const argument = argv[i] != nullptr ? argv[i] : "";
		if (argument == "--help" || argument == "-h") {
			options.show_help = true;
			continue;
		}
		if (argument == "--project") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option --project requires a manifest path; using default project");
				continue;
			}
			options.project_manifest_path = argv[++i];
			continue;
		}
		if (argument == "--capture-baseline") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option --capture-baseline requires an output .ppm path; capture disabled");
				continue;
			}
			options.capture_baseline_path = argv[++i];
			continue;
		}
		if (argument == "--build-mesh") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option --build-mesh requires an output .obj path; mesh build disabled");
				continue;
			}
			options.build_mesh_path = argv[++i];
			continue;
		}
		if (argument == "--mesh-projection") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option --mesh-projection requires auto, xy, xz or yz; keeping default");
				continue;
			}
			sfm::scene::PointMeshProjectionPlane projection{};
			std::string const value = argv[++i];
			if (parse_projection_argument(value, projection))
				options.mesh_build_options.projection = projection;
			else
				options.messages.emplace_back("Startup option --mesh-projection ignored unsupported value '" + value + "'");
			continue;
		}
		if (argument == "--mesh-weld" || argument == "--mesh-min-area" || argument == "--mesh-max-edge") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option " + argument + " requires a finite numeric value; keeping default");
				continue;
			}
			float value = 0.0f;
			std::string const text = argv[++i];
			if (!parse_float_argument(text, value)) {
				options.messages.emplace_back("Startup option " + argument + " ignored non-finite value '" + text + "'");
				continue;
			}
			if (argument == "--mesh-weld")
				options.mesh_build_options.weld_epsilon = value;
			else if (argument == "--mesh-min-area")
				options.mesh_build_options.minimum_triangle_area = value;
			else
				options.mesh_build_options.maximum_edge_length = value;
			continue;
		}
		if (argument == "--mesh-max-points") {
			if (i + 1 >= argc) {
				options.messages.emplace_back("Startup option --mesh-max-points requires an integer value; keeping default");
				continue;
			}
			std::string const text = argv[++i];
			float value = 0.0f;
			if (!parse_float_argument(text, value) || value < 3.0f) {
				options.messages.emplace_back("Startup option --mesh-max-points ignored invalid value '" + text + "'");
				continue;
			}
			options.mesh_build_options.maximum_projected_points = static_cast<std::size_t>(value);
			continue;
		}
		if (argument == "--validate-install") {
			options.validate_install = true;
			continue;
		}
		options.messages.emplace_back("Ignoring unsupported startup argument '" + argument + "'");
	}
	return options;
}

StartupValidationResult validate_startup_assets(StartupOptions const& options)
{
	StartupValidationResult result{};
	result.succeeded = true;

	for (std::string const& message : options.messages)
		result.messages.emplace_back("startup warning: " + message);

	validate_existing_file(result, "project manifest", options.project_manifest_path);
	sfm::scene::SandboxProjectLoadResult project = sfm::scene::load_sandbox_project(options.project_manifest_path);
	for (std::string const& message : project.messages)
		result.messages.emplace_back("project: " + message);
	if (!project.succeeded) {
		result.succeeded = false;
		return result;
	}

	validate_existing_file(result, "point cloud", project.project.point_cloud_path);
	validate_optional_file(result, "camera poses", project.project.camera_pose_path);
	validate_optional_file(result, "surface", project.project.surface_path);
	validate_optional_file(result, "image", project.project.image_path);
	for (char const* shader_file : kRequiredShaderFiles)
		validate_existing_file(result, "shader", resolve_shader_file_for_validation(shader_file));
	return result;
}

MeshBuildCommandResult build_mesh_from_startup_project(StartupOptions const& options)
{
	MeshBuildCommandResult result{};
	for (std::string const& message : options.messages)
		result.messages.emplace_back("startup warning: " + message);

	sfm::scene::SandboxProjectLoadResult project = sfm::scene::load_sandbox_project(options.project_manifest_path);
	append_prefixed(result.messages, "project: ", project.messages);
	if (!project.succeeded) {
		result.messages.emplace_back("Mesh build failed: selected project manifest could not be loaded");
		return result;
	}

	sfm::scene::PointCloudLoadResult point_cloud = sfm::scene::load_point_cloud_from_file(project.project.point_cloud_path);
	append_prefixed(result.messages, "point cloud: ", point_cloud.messages);
	if (!point_cloud.succeeded) {
		result.messages.emplace_back("Mesh build failed: point cloud could not be loaded");
		return result;
	}

	sfm::scene::PointToMeshBuildResult mesh = sfm::scene::build_surface_mesh_from_point_cloud(point_cloud.cloud, options.mesh_build_options);
	append_prefixed(result.messages, "mesh: ", mesh.messages);
	if (!mesh.succeeded) {
		result.messages.emplace_back("Mesh build failed: point cloud triangulation did not produce a valid mesh");
		return result;
	}

	sfm::scene::SurfaceExportResult export_result = sfm::scene::export_surface_mesh_as_obj(mesh.mesh,
	                                                                                       options.build_mesh_path,
	                                                                                       mesh_build_export_comment(project.project.point_cloud_path, mesh));
	append_prefixed(result.messages, "export: ", export_result.messages);
	if (!export_result.succeeded) {
		result.messages.emplace_back("Mesh build failed: generated mesh could not be exported");
		return result;
	}

	result.succeeded = true;
	result.messages.emplace_back("Mesh build completed");
	return result;
}

void print_startup_help()
{
	std::cout << "SfM Visualization Sandbox\n";
	std::cout << "Usage: SfmSandbox.exe [--project path-to-project.sfmproj] [--capture-baseline output.ppm] [--validate-install] [--build-mesh output.obj]\n";
	std::cout << "The default project is res/sandbox/default_project.sfmproj.\n";
	std::cout << "--validate-install checks startup resources and shaders, then exits before opening a window.\n";
	std::cout << "--build-mesh converts the project point cloud to OBJ without opening a window.\n";
	std::cout << "--mesh-max-points limits projected samples so experimental triangulation cannot silently freeze large builds.\n";
}

} // namespace sfm::app
