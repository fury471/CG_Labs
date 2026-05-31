#include "SfmSandboxApp.hpp"

#include "config.hpp"
#include "sandbox/gfx/FramebufferCapture.hpp"

#include <imgui.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

namespace sfm::app
{
namespace
{

constexpr char const* kLegacyTextPointCloudResource = "sandbox/sample_point_cloud.xyzrgb";
constexpr char const* kDefaultProjectResource = "sandbox/default_project.sfmproj";

constexpr std::array<char const*, 6> kRequiredShaderFiles{
	"shaders/sandbox/line.vert",
	"shaders/sandbox/line.frag",
	"shaders/sandbox/point_cloud.vert",
	"shaders/sandbox/image_plane.vert",
	"shaders/sandbox/image_plane.frag",
	"shaders/sandbox/instanced_marker.vert",
};

void copy_path_to_buffer(std::array<char, 512>& buffer, std::filesystem::path const& path)
{
	std::string const text = path.string();
	buffer.fill('\0');
	std::size_t const count = std::min(text.size(), buffer.size() - 1u);
	std::copy_n(text.data(), count, buffer.data());
}

void draw_point_cloud_statistics(sfm::scene::PointCloudStatistics const& statistics)
{
	ImGui::Text("Dataset point count: %zu", statistics.point_count);
	ImGui::Text("Approx. CPU storage: %zu bytes", statistics.approximate_cpu_bytes);
	if (!statistics.has_bounds) {
		ImGui::TextUnformatted("Bounds: unavailable");
		return;
	}
	ImGui::Text("Bounds min: %.3f, %.3f, %.3f", statistics.bounds_min.x, statistics.bounds_min.y, statistics.bounds_min.z);
	ImGui::Text("Bounds max: %.3f, %.3f, %.3f", statistics.bounds_max.x, statistics.bounds_max.y, statistics.bounds_max.z);
	ImGui::Text("Bounds extent: %.3f, %.3f, %.3f", statistics.bounds_extent.x, statistics.bounds_extent.y, statistics.bounds_extent.z);
}

void draw_surface_statistics(sfm::scene::SurfaceStatistics const& statistics)
{
	ImGui::Text("Surface vertices: %zu", statistics.vertex_count);
	ImGui::Text("Surface triangles: %zu", statistics.triangle_count);
	ImGui::Text("Approx. surface CPU storage: %zu bytes", statistics.approximate_cpu_bytes);
	if (!statistics.has_bounds) {
		ImGui::TextUnformatted("Surface bounds: unavailable");
		return;
	}
	ImGui::Text("Surface bounds min: %.3f, %.3f, %.3f", statistics.bounds_min.x, statistics.bounds_min.y, statistics.bounds_min.z);
	ImGui::Text("Surface bounds max: %.3f, %.3f, %.3f", statistics.bounds_max.x, statistics.bounds_max.y, statistics.bounds_max.z);
	ImGui::Text("Surface bounds extent: %.3f, %.3f, %.3f", statistics.bounds_extent.x, statistics.bounds_extent.y, statistics.bounds_extent.z);
}

void draw_image_statistics(sfm::scene::ImageResource const& image)
{
	if (image.empty()) {
		ImGui::TextUnformatted("Associated image: unavailable");
		return;
	}
	ImGui::Text("Image size: %d x %d", image.width, image.height);
	ImGui::Text("Image pixels: %zu", image.pixel_count());
	ImGui::Text("Image format: %s", image.source_format.c_str());
	ImGui::Text("Image source: %s", image.source_file.c_str());
}

void draw_render_target_status(sfm::gfx::RenderTargetStatus const& status)
{
	ImGui::Text("Offscreen target: %s", status.ready ? "complete" : "not ready");
	ImGui::Text("Target size: %d x %d", status.width, status.height);
	ImGui::Text("Framebuffer id: %u", status.framebuffer_id);
	ImGui::Text("Colour texture id: %u", status.colour_texture_id);
	ImGui::Text("Rebuilds after resize: %d", status.rebuild_count);
	ImGui::TextWrapped("Framebuffer status: %s", status.message.c_str());
}

void draw_frame_profiler(sfm::core::FrameProfiler const& profiler)
{
	ImGui::Text("Last completed CPU frame total: %.3f ms", profiler.total_milliseconds());
	for (sfm::core::TimedPass const& pass : profiler.passes())
		ImGui::BulletText("%s: %.3f ms", pass.name.c_str(), pass.milliseconds);
}

void draw_marker_stress_statistics(sfm::gfx::MarkerStressStats const& statistics)
{
	ImGui::Text("Stress markers: %d", statistics.marker_count);
	ImGui::Text("Active marker draw calls: %d", statistics.draw_calls);
	ImGui::Text("Reference non-instanced draw calls: %d", statistics.reference_draw_calls);
	ImGui::Text("Instanced draw calls: %d", statistics.instanced_draw_calls);
	ImGui::Text("Marker vertices drawn: %d", statistics.vertices_drawn);
	ImGui::Text("Draw-call reduction: %d", statistics.reference_draw_calls - statistics.instanced_draw_calls);
	ImGui::Text("Active path: %s", statistics.using_instancing ? "instanced" : "reference non-instanced");
}

char const* colour_mode_label(sfm::gfx::PointColourMode mode)
{
	switch (mode) {
	case sfm::gfx::PointColourMode::Source: return "Source colour";
	case sfm::gfx::PointColourMode::Height: return "Height gradient";
	case sfm::gfx::PointColourMode::Solid: return "Solid colour";
	}
	return "Unknown";
}

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

void draw_point_controls(sfm::gfx::PointCloudRenderSettings& settings)
{
	ImGui::SliderFloat("Point size", &settings.point_size, 1.0f, 18.0f, "%.1f px");
	int colour_mode = static_cast<int>(settings.colour_mode);
	if (ImGui::BeginCombo("Point colour mode", colour_mode_label(settings.colour_mode))) {
		for (int value = 0; value <= 2; ++value) {
			auto const mode = static_cast<sfm::gfx::PointColourMode>(value);
			bool const selected = colour_mode == value;
			if (ImGui::Selectable(colour_mode_label(mode), selected))
				colour_mode = value;
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	settings.colour_mode = static_cast<sfm::gfx::PointColourMode>(colour_mode);
	if (settings.colour_mode == sfm::gfx::PointColourMode::Solid)
		ImGui::ColorEdit3("Solid point colour", &settings.solid_colour.x);
	ImGui::Checkbox("Show point-cloud bounds", &settings.show_bounds);
}

void draw_camera_pose_metadata(sfm::scene::CameraPoseSet const& camera_poses, int& selected_pose)
{
	if (camera_poses.empty()) {
		ImGui::TextUnformatted("Pose metadata: unavailable");
		return;
	}

	int const max_pose_index = static_cast<int>(camera_poses.size() - 1u);
	selected_pose = std::clamp(selected_pose, 0, max_pose_index);
	ImGui::SliderInt("Selected pose", &selected_pose, 0, max_pose_index);

	auto const poses = camera_poses.poses();
	sfm::scene::CameraPose const& pose = poses[static_cast<std::size_t>(selected_pose)];
	sfm::scene::CameraPoseMetadata const& metadata = pose.metadata;
	glm::vec3 const position{ pose.camera_to_world[3] };
	glm::vec3 const right{ pose.camera_to_world[0] };
	glm::vec3 const up{ pose.camera_to_world[1] };
	glm::vec3 const forward = -glm::vec3{ pose.camera_to_world[2] };

	ImGui::Text("Index: %zu", metadata.index);
	ImGui::Text("Source id: %d", metadata.source_id);
	ImGui::Text("Camera id: %d", metadata.camera_id);
	ImGui::Text("Source line: %zu", metadata.source_line);
	ImGui::Text("Image/name: %s", metadata.image_name.empty() ? "<none>" : metadata.image_name.c_str());
	ImGui::Text("Format: %s", metadata.source_format.c_str());
	ImGui::Text("Source file: %s", metadata.source_file.empty() ? "<none>" : metadata.source_file.c_str());
	ImGui::TextWrapped("Source convention: %s", metadata.source_convention.c_str());
	ImGui::Text("Position: %.3f, %.3f, %.3f", position.x, position.y, position.z);
	ImGui::Text("Forward: %.3f, %.3f, %.3f", forward.x, forward.y, forward.z);
	ImGui::Text("Up: %.3f, %.3f, %.3f", up.x, up.y, up.z);
	ImGui::Text("Right: %.3f, %.3f, %.3f", right.x, right.y, right.z);
}

void draw_renderer_frame_statistics(sfm::gfx::RendererFrameStatistics const& statistics,
                                    sfm::gfx::SurfaceRendererStats const& surface_statistics,
                                    sfm::gfx::ImagePlaneRendererStats const& image_statistics,
                                    sfm::gfx::MarkerStressStats const& marker_statistics)
{
	ImGui::Text("Submitted items: %d", statistics.submitted_items);
	ImGui::Text("Draw calls: %d + surface %d + image %d + markers %d", statistics.draw_calls, surface_statistics.draw_calls, image_statistics.draw_calls, marker_statistics.draw_calls);
	ImGui::Text("Program binds: %d", statistics.program_binds);
	ImGui::Text("Vertex-array binds: %d", statistics.vertex_array_binds);
	ImGui::Text("Line vertices drawn: %d", statistics.line_vertices_drawn);
	ImGui::Text("Point vertices drawn: %d", statistics.point_vertices_drawn);
	ImGui::Text("Surface vertices drawn: %d", surface_statistics.vertices_drawn);
	ImGui::Text("Surface triangles drawn: %d", surface_statistics.triangles_drawn);
	ImGui::Text("Image-plane vertices drawn: %d", image_statistics.vertices_drawn);
	ImGui::Text("Image-plane triangles drawn: %d", image_statistics.triangles_drawn);
	ImGui::Text("Marker vertices drawn: %d", marker_statistics.vertices_drawn);
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
	validate_existing_file(result, "camera poses", project.project.camera_pose_path);
	validate_existing_file(result, "surface", project.project.surface_path);
	validate_existing_file(result, "image", project.project.image_path);
	for (char const* shader_file : kRequiredShaderFiles)
		validate_existing_file(result, "shader", resolve_shader_file_for_validation(shader_file));
	return result;
}

void print_startup_help()
{
	std::cout << "SfM Visualization Sandbox\n";
	std::cout << "Usage: SfmSandbox.exe [--project path-to-project.sfmproj] [--capture-baseline output.ppm] [--validate-install]\n";
	std::cout << "The default project is res/sandbox/default_project.sfmproj.\n";
	std::cout << "--validate-install checks startup resources and shaders, then exits before opening a window.\n";
}

SfmSandboxApp::SfmSandboxApp(StartupOptions startup_options)
	: m_startup_options(std::move(startup_options))
{
}

bool SfmSandboxApp::initialise()
{
	m_clear_pass.initialise();
	m_ownership_probe = sfm::gfx::run_ownership_probe("SfmSandbox ownership probe");
	m_shader_probe = sfm::gfx::run_shader_program_probe();

	initialise_project();
	initialise_point_cloud();
	initialise_camera_poses();
	initialise_surface();
	initialise_image();
	initialise_renderers();
	return m_renderer.ready();
}

void SfmSandboxApp::initialise_project()
{
	m_sample_project = sfm::scene::make_sample_project(std::filesystem::path{ config::resources_path("") });
	m_project_load = sfm::scene::load_sandbox_project(m_startup_options.project_manifest_path);
	m_active_project = m_project_load.succeeded ? m_project_load.project : m_sample_project;
	m_legacy_text_point_cloud_path = config::resources_path(kLegacyTextPointCloudResource);
}

void SfmSandboxApp::initialise_point_cloud()
{
	copy_path_to_buffer(m_point_cloud.path_buffer, m_active_project.point_cloud_path);
	m_point_cloud.load = sfm::scene::load_point_cloud_from_file(m_active_project.point_cloud_path);
	m_point_cloud.using_loaded_data = m_point_cloud.load.succeeded;
	m_point_cloud.data = m_point_cloud.using_loaded_data ? std::move(m_point_cloud.load.cloud) : sfm::scene::PointCloud::make_debug_cluster();
	m_point_cloud.active_source = m_point_cloud.using_loaded_data ? m_active_project.point_cloud_path.string() : "procedural fallback";
	m_point_cloud.last_reload_status = m_point_cloud.using_loaded_data ? "Initial point cloud loaded" : "Initial load failed; using procedural fallback";
}

void SfmSandboxApp::initialise_camera_poses()
{
	copy_path_to_buffer(m_camera_poses.path_buffer, m_active_project.camera_pose_path);
	m_camera_poses.load = sfm::scene::load_camera_poses_from_file(m_active_project.camera_pose_path);
	m_camera_poses.using_loaded_data = m_camera_poses.load.succeeded;
	m_camera_poses.data = m_camera_poses.using_loaded_data ? std::move(m_camera_poses.load.poses) : sfm::scene::CameraPoseSet::make_debug_orbit();
	m_camera_poses.active_source = m_camera_poses.using_loaded_data ? m_active_project.camera_pose_path.string() : "procedural fallback";
	m_camera_poses.last_reload_status = m_camera_poses.using_loaded_data ? "Initial camera poses loaded" : "Initial load failed; using procedural fallback";
}

void SfmSandboxApp::initialise_surface()
{
	copy_path_to_buffer(m_surface.path_buffer, m_active_project.surface_path);
	m_surface.load = sfm::scene::import_surface(m_active_project.surface_path);
	m_surface.using_loaded_data = m_surface.load.succeeded;
	m_surface.data = m_surface.using_loaded_data ? std::move(m_surface.load.mesh) : sfm::scene::SurfaceMesh{};
	m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
	m_surface.active_source = m_surface.using_loaded_data ? m_active_project.surface_path.string() : "no surface loaded";
	m_surface.last_reload_status = m_surface.using_loaded_data ? "Initial surface loaded" : "Initial surface load failed";
}

void SfmSandboxApp::initialise_image()
{
	copy_path_to_buffer(m_image.path_buffer, m_active_project.image_path);
	m_image.load = sfm::scene::import_image(m_active_project.image_path);
	m_image.using_loaded_data = m_image.load.succeeded;
	m_image.data = m_image.using_loaded_data ? std::move(m_image.load.image) : sfm::scene::ImageResource{};
	m_image.active_source = m_image.using_loaded_data ? m_active_project.image_path.string() : "no image loaded";
	m_image.last_reload_status = m_image.using_loaded_data ? "Initial image loaded" : "Initial image load failed";
}

void SfmSandboxApp::initialise_renderers()
{
	m_renderer_result = m_renderer.initialise(m_point_cloud.data, m_camera_poses.data);
	m_surface_renderer_result = m_surface_renderer.reload(m_surface.data, m_surface.colour);
	rebuild_image_plane_for_active_camera();
	m_marker_renderer_result = m_marker_renderer.rebuild(m_marker_settings);
}

void SfmSandboxApp::ensure_render_target_size(int framebuffer_width, int framebuffer_height)
{
	if (framebuffer_width > 0 && framebuffer_height > 0)
		[[maybe_unused]] bool const render_target_ready = m_offscreen_probe.ensure_size(framebuffer_width, framebuffer_height);
}

void SfmSandboxApp::render_scene(glm::mat4 const& world_to_clip, int framebuffer_width, int framebuffer_height)
{
	m_gpu_profiler.begin_frame();
	{
		auto scope = m_gpu_profiler.scope("clear");
		m_clear_pass.render(framebuffer_width, framebuffer_height);
	}
	{
		auto scope = m_gpu_profiler.scope("grid, points and cameras");
		m_renderer.render(world_to_clip, m_point_settings);
	}
	{
		auto scope = m_gpu_profiler.scope("surface mesh");
		m_surface_renderer.render(world_to_clip, m_surface.visible);
	}
	{
		auto scope = m_gpu_profiler.scope("image plane");
		m_image_plane_renderer.render(world_to_clip, m_image.visible);
	}
	{
		auto scope = m_gpu_profiler.scope("marker stress scene");
		m_marker_renderer.render(world_to_clip, m_marker_settings);
	}
	m_gpu_profiler.end_frame();
}

bool SfmSandboxApp::capture_baseline(int framebuffer_width, int framebuffer_height)
{
	m_capture_messages.clear();
	if (!capture_requested()) {
		m_capture_messages.emplace_back("Baseline capture skipped: no capture path was requested");
		return false;
	}
	if (framebuffer_width <= 0 || framebuffer_height <= 0) {
		m_capture_messages.emplace_back("Baseline capture failed: framebuffer size is invalid");
		return false;
	}

	std::filesystem::path const output_path = m_startup_options.capture_baseline_path;
	sfm::gfx::FramebufferCaptureResult image_capture = sfm::gfx::capture_back_buffer_to_ppm(output_path, framebuffer_width, framebuffer_height);
	m_capture_messages.insert(m_capture_messages.end(), image_capture.messages.begin(), image_capture.messages.end());
	if (!image_capture.succeeded)
		return false;

	std::filesystem::path metadata_path = output_path;
	metadata_path += ".txt";
	std::ofstream metadata_file{ metadata_path };
	if (!metadata_file) {
		m_capture_messages.emplace_back("Baseline capture failed: could not open metadata output '" + metadata_path.string() + "'");
		return false;
	}
	metadata_file << "SfM Visualization Sandbox visual baseline\n";
#ifdef NDEBUG
	metadata_file << "build_mode = Release-like\n";
#else
	metadata_file << "build_mode = Debug-like\n";
#endif
	metadata_file << "framebuffer = " << framebuffer_width << " x " << framebuffer_height << '\n';
	metadata_file << "project_manifest = " << m_active_project.manifest_path.string() << '\n';
	metadata_file << "project_load_status = " << (m_project_load.succeeded ? "loaded" : "sample fallback") << '\n';
	metadata_file << "point_cloud = " << m_point_cloud.active_source << '\n';
	metadata_file << "point_count = " << m_point_cloud.data.size() << '\n';
	metadata_file << "camera_poses = " << m_camera_poses.active_source << '\n';
	metadata_file << "camera_pose_count = " << m_camera_poses.data.size() << '\n';
	metadata_file << "surface = " << m_surface.active_source << '\n';
	metadata_file << "surface_visible = " << (m_surface.visible ? "on" : "off") << '\n';
	metadata_file << "surface_triangles = " << m_surface.statistics.triangle_count << '\n';
	metadata_file << "image = " << m_image.active_source << '\n';
	metadata_file << "image_visible = " << (m_image.visible ? "on" : "off") << '\n';
	metadata_file << "image_size = " << m_image.data.width << " x " << m_image.data.height << '\n';
	metadata_file << "marker_count = " << m_marker_renderer.frame_statistics().marker_count << '\n';
	metadata_file << "marker_visible = " << (m_marker_settings.visible ? "on" : "off") << '\n';
	metadata_file << "marker_instancing = " << (m_marker_settings.use_instancing ? "on" : "off") << '\n';
	metadata_file << "point_colour_mode = " << static_cast<int>(m_point_settings.colour_mode) << '\n';
	metadata_file << "point_size = " << m_point_settings.point_size << '\n';
	if (!metadata_file) {
		m_capture_messages.emplace_back("Baseline capture failed: could not write metadata output '" + metadata_path.string() + "'");
		return false;
	}

	m_capture_messages.emplace_back("Baseline capture wrote metadata '" + metadata_path.string() + "'");
	return true;
}

void SfmSandboxApp::reload_point_cloud_from_ui()
{
	std::filesystem::path const requested_path{ std::string{ m_point_cloud.path_buffer.data() } };
	sfm::scene::PointCloudLoadResult candidate_load = sfm::scene::load_point_cloud_from_file(requested_path);
	if (!candidate_load.succeeded) {
		m_point_cloud.load = std::move(candidate_load);
		m_point_cloud.last_reload_status = "Reload failed during file load; previous cloud kept";
		return;
	}

	sfm::scene::PointCloud candidate_cloud = std::move(candidate_load.cloud);
	sfm::gfx::RendererBuildResult candidate_renderer_build = m_renderer.reload_point_cloud(candidate_cloud);
	if (!candidate_renderer_build.succeeded) {
		m_renderer_result = std::move(candidate_renderer_build);
		m_point_cloud.last_reload_status = "Reload failed during GPU upload; previous cloud kept";
		return;
	}

	m_point_cloud.data = std::move(candidate_cloud);
	m_point_cloud.load = std::move(candidate_load);
	m_renderer_result = std::move(candidate_renderer_build);
	m_point_cloud.using_loaded_data = true;
	m_point_cloud.active_source = requested_path.string();
	m_point_cloud.last_reload_status = "Reload succeeded";
	++m_point_cloud.reload_count;
}

void SfmSandboxApp::reload_camera_poses_from_ui()
{
	std::filesystem::path const requested_path{ std::string{ m_camera_poses.path_buffer.data() } };
	sfm::scene::CameraPoseLoadResult candidate_load = sfm::scene::load_camera_poses_from_file(requested_path);
	if (!candidate_load.succeeded) {
		m_camera_poses.load = std::move(candidate_load);
		m_camera_poses.last_reload_status = "Pose reload failed during file load; previous poses kept";
		return;
	}

	sfm::scene::CameraPoseSet candidate_poses = std::move(candidate_load.poses);
	sfm::gfx::RendererBuildResult candidate_renderer_build = m_renderer.reload_camera_poses(candidate_poses);
	if (!candidate_renderer_build.succeeded) {
		m_renderer_result = std::move(candidate_renderer_build);
		m_camera_poses.last_reload_status = "Pose reload failed during GPU upload; previous poses kept";
		return;
	}

	m_camera_poses.data = std::move(candidate_poses);
	m_camera_poses.load = std::move(candidate_load);
	m_renderer_result = std::move(candidate_renderer_build);
	m_camera_poses.using_loaded_data = true;
	m_camera_poses.active_source = requested_path.string();
	m_camera_poses.last_reload_status = "Pose reload succeeded";
	m_camera_poses.selected_pose = 0;
	m_image.associated_camera_index = 0;
	++m_camera_poses.reload_count;
	rebuild_image_plane_for_active_camera();
}

void SfmSandboxApp::reload_surface_from_ui()
{
	std::filesystem::path const requested_path{ std::string{ m_surface.path_buffer.data() } };
	sfm::scene::SurfaceImportResult candidate_load = sfm::scene::import_surface(requested_path);
	if (!candidate_load.succeeded) {
		m_surface.load = std::move(candidate_load);
		m_surface.last_reload_status = "Surface reload failed during file load; previous surface kept";
		return;
	}

	sfm::scene::SurfaceMesh candidate_mesh = std::move(candidate_load.mesh);
	sfm::gfx::SurfaceRendererResult candidate_render = m_surface_renderer.reload(candidate_mesh, m_surface.colour);
	if (!candidate_render.succeeded) {
		m_surface_renderer_result = std::move(candidate_render);
		m_surface.last_reload_status = "Surface reload failed during GPU upload; previous surface kept";
		return;
	}

	m_surface.data = std::move(candidate_mesh);
	m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
	m_surface.load = std::move(candidate_load);
	m_surface_renderer_result = std::move(candidate_render);
	m_surface.using_loaded_data = true;
	m_surface.active_source = requested_path.string();
	m_surface.last_reload_status = "Surface reload succeeded";
	++m_surface.reload_count;
}

void SfmSandboxApp::reload_image_from_ui()
{
	std::filesystem::path const requested_path{ std::string{ m_image.path_buffer.data() } };
	sfm::scene::ImageImportResult candidate_load = sfm::scene::import_image(requested_path);
	if (!candidate_load.succeeded) {
		m_image.load = std::move(candidate_load);
		m_image.last_reload_status = "Image reload failed during file load; previous image kept";
		return;
	}

	sfm::scene::ImageResource candidate_image = std::move(candidate_load.image);
	auto const poses = m_camera_poses.data.poses();
	glm::mat4 const pose = poses.empty() ? glm::mat4{ 1.0f } : poses[static_cast<std::size_t>(std::clamp(m_image.associated_camera_index, 0, static_cast<int>(poses.size() - 1u)))].camera_to_world;
	sfm::gfx::ImagePlaneRendererResult candidate_plane = m_image_plane_renderer.reload(candidate_image, pose, m_image.plane_distance, m_image.plane_height);
	if (!candidate_plane.succeeded) {
		m_image_plane_result = std::move(candidate_plane);
		m_image.last_reload_status = "Image reload failed during GPU upload; previous image kept";
		return;
	}

	m_image.data = std::move(candidate_image);
	m_image.load = std::move(candidate_load);
	m_image_plane_result = std::move(candidate_plane);
	m_image.using_loaded_data = true;
	m_image.active_source = requested_path.string();
	m_image.last_reload_status = "Image reload succeeded";
	++m_image.reload_count;
}

void SfmSandboxApp::rebuild_image_plane_for_active_camera()
{
	auto const poses = m_camera_poses.data.poses();
	if (!poses.empty())
		m_image.associated_camera_index = std::clamp(m_image.associated_camera_index, 0, static_cast<int>(poses.size() - 1u));
	glm::mat4 const pose = poses.empty() ? glm::mat4{ 1.0f } : poses[static_cast<std::size_t>(m_image.associated_camera_index)].camera_to_world;
	m_image_plane_result = m_image_plane_renderer.reload(m_image.data, pose, m_image.plane_distance, m_image.plane_height);
}

void SfmSandboxApp::update_surface_colour()
{
	sfm::gfx::SurfaceRendererResult colour_update = m_surface_renderer.reload(m_surface.data, m_surface.colour);
	if (colour_update.succeeded)
		m_surface_renderer_result = std::move(colour_update);
	else
		m_surface.last_reload_status = "Surface colour update failed; previous surface kept";
}

void SfmSandboxApp::draw_status_panel(sfm::core::FrameTiming const& frame_timing,
                                      sfm::core::FrameProfiler const& frame_profiler,
                                      int framebuffer_width,
                                      int framebuffer_height,
                                      float camera_aspect)
{
	if (ImGui::Begin("Sandbox status")) {
		ImGui::TextUnformatted("SfM Visualization Sandbox");
		ImGui::Separator();
		ImGui::Text("Frame: %llu", static_cast<unsigned long long>(frame_timing.frame_index));
		ImGui::Text("CPU frame: %.3f ms", frame_timing.delta_milliseconds);
		ImGui::Text("FPS: %.1f", frame_timing.frames_per_second);
		ImGui::Text("Framebuffer: %d x %d", framebuffer_width, framebuffer_height);
		ImGui::Text("Camera aspect: %.3f", camera_aspect);
		ImGui::SliderFloat("UI scale", &m_ui_scale, 1.0f, 2.0f, "%.2f x");
		ImGui::Separator();
		draw_project_section();
		draw_marker_stress_section();
		draw_render_target_section(frame_profiler);
		ImGui::Separator();
		draw_point_cloud_statistics(m_point_cloud.data.statistics());
		ImGui::Separator();
		draw_image_section();
		draw_surface_section();
		draw_point_cloud_section();
		draw_camera_pose_section();
		draw_renderer_section();
		draw_regression_probe_section();
		ImGui::Separator();
		ImGui::TextUnformatted("Controls: WASD/QE move, left mouse drag look, F2 UI, F3 logs, F11 fullscreen, Esc quit");
	}
	ImGui::End();
}

void SfmSandboxApp::draw_project_section()
{
	ImGui::TextUnformatted("Project manifest");
	ImGui::Text("Manifest: %s", m_active_project.manifest_path.string().c_str());
	ImGui::Text("Project load: %s", m_project_load.succeeded ? "loaded" : "using sample fallback");
	for (std::string const& message : m_startup_options.messages)
		ImGui::BulletText("%s", message.c_str());
	for (std::string const& message : m_project_load.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_marker_stress_section()
{
	ImGui::TextUnformatted("Milestone 19: Instancing and workload stress scenes");
	bool marker_rebuild_requested = false;
	marker_rebuild_requested |= ImGui::Checkbox("Show marker stress scene", &m_marker_settings.visible);
	ImGui::Checkbox("Use instanced marker path", &m_marker_settings.use_instancing);
	marker_rebuild_requested |= ImGui::SliderInt("Stress marker count", &m_marker_settings.marker_count, 1, 5000);
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker radius", &m_marker_settings.radius, 0.5f, 10.0f, "%.2f");
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker height", &m_marker_settings.height, -2.0f, 4.0f, "%.2f");
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker scale", &m_marker_settings.marker_scale, 0.01f, 0.25f, "%.3f");
	if (marker_rebuild_requested || ImGui::Button("Rebuild marker stress scene"))
		m_marker_renderer_result = m_marker_renderer.rebuild(m_marker_settings);
	ImGui::Text("Marker renderer: %s", m_marker_renderer.ready() ? "ready" : "not ready");
	draw_marker_stress_statistics(m_marker_renderer.frame_statistics());
	ImGui::TextWrapped("M19 comparison: reference mode issues one draw call per marker; instanced mode draws the same marker workload with one instanced draw call.");
	for (std::string const& message : m_marker_renderer_result.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_render_target_section(sfm::core::FrameProfiler const& frame_profiler)
{
	ImGui::TextUnformatted("M18 render-target and profiling baseline");
	draw_render_target_status(m_offscreen_probe.status());
	draw_frame_profiler(frame_profiler);
	draw_gpu_profiler_section();
	ImGui::TextWrapped("Baseline scene: sample PLY point cloud + sample surface OBJ + sample PPM image plane + eight sample camera frustums + M19 marker stress scene.");
	ImGui::TextWrapped("Timing results are reported honestly; M19 should demonstrate or refute the expected instancing improvement on local hardware.");
}

void SfmSandboxApp::draw_gpu_profiler_section()
{
	sfm::gfx::GpuProfilerSnapshot const snapshot = m_gpu_profiler.snapshot();
	ImGui::TextUnformatted("M25 GPU pass telemetry");
	ImGui::Text("GPU timer queries: %s", snapshot.supported ? "available" : "unavailable");
	ImGui::TextWrapped("%s", snapshot.message.c_str());
	ImGui::Text("Pending GPU timing frames: %zu", snapshot.pending_frame_count);
	if (snapshot.passes.empty()) {
		ImGui::TextUnformatted("Last completed GPU passes: unavailable");
		return;
	}
	for (sfm::gfx::GpuTimedPass const& pass : snapshot.passes)
		ImGui::BulletText("%s: %.3f ms", pass.name.c_str(), pass.milliseconds);
}

void SfmSandboxApp::draw_image_section()
{
	ImGui::TextUnformatted("Camera image association");
	ImGui::Checkbox("Show associated image plane", &m_image.visible);
	draw_image_statistics(m_image.data);
	if (!m_camera_poses.data.empty()) {
		int const max_camera_index = static_cast<int>(m_camera_poses.data.size() - 1u);
		if (ImGui::SliderInt("Associated camera", &m_image.associated_camera_index, 0, max_camera_index))
			rebuild_image_plane_for_active_camera();
	}
	bool image_plane_changed = false;
	image_plane_changed |= ImGui::SliderFloat("Image plane distance", &m_image.plane_distance, 0.10f, 2.00f, "%.2f");
	image_plane_changed |= ImGui::SliderFloat("Image plane height", &m_image.plane_height, 0.10f, 2.00f, "%.2f");
	if (image_plane_changed && !m_camera_poses.data.empty())
		rebuild_image_plane_for_active_camera();
	ImGui::InputText("Image path", m_image.path_buffer.data(), m_image.path_buffer.size());
	if (ImGui::Button("Load / Reload image"))
		reload_image_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to image sample"))
		copy_path_to_buffer(m_image.path_buffer, m_sample_project.image_path);
	ImGui::Text("Active image source: %s", m_image.using_loaded_data ? m_image.active_source.c_str() : "no image loaded");
	ImGui::Text("Last image reload: %s", m_image.last_reload_status.c_str());
	ImGui::Text("Successful image reloads: %d", m_image.reload_count);
	ImGui::Text("Image-plane renderer: %s", m_image_plane_renderer.ready() ? "ready" : "not ready");
	ImGui::TextWrapped("Projection sanity: image card is placed in the selected camera's local -Z direction; its centre should lie on that camera frustum forward ray.");
	for (std::string const& message : m_image.load.messages)
		ImGui::BulletText("%s", message.c_str());
	for (std::string const& message : m_image_plane_result.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_surface_section()
{
	ImGui::TextUnformatted("Surface mesh");
	ImGui::Checkbox("Show surface mesh", &m_surface.visible);
	if (ImGui::ColorEdit3("Surface colour", &m_surface.colour.x))
		update_surface_colour();
	if (ImGui::Button("Rebuild surface colour"))
		m_surface_renderer_result = m_surface_renderer.reload(m_surface.data, m_surface.colour);
	draw_surface_statistics(m_surface.statistics);
	ImGui::Text("Surface source: %s", m_surface.active_source.c_str());
	ImGui::Text("Surface renderer: %s", m_surface_renderer.ready() ? "ready" : "not ready");
	ImGui::Text("Surface GPU triangles: %d", m_surface_renderer.triangle_count());
	ImGui::InputText("Surface path", m_surface.path_buffer.data(), m_surface.path_buffer.size());
	if (ImGui::Button("Load / Reload surface"))
		reload_surface_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to surface sample"))
		copy_path_to_buffer(m_surface.path_buffer, m_sample_project.surface_path);
	ImGui::Text("Active surface source: %s", m_surface.using_loaded_data ? m_surface.active_source.c_str() : "no surface loaded");
	ImGui::Text("Last surface reload: %s", m_surface.last_reload_status.c_str());
	ImGui::Text("Successful surface reloads: %d", m_surface.reload_count);
	ImGui::Text("Skipped surface lines: %zu", m_surface.load.skipped_lines);
	for (std::string const& message : m_surface.load.messages)
		ImGui::BulletText("%s", message.c_str());
	for (std::string const& message : m_surface_renderer_result.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_point_cloud_section()
{
	ImGui::TextUnformatted("Point display");
	draw_point_controls(m_point_settings);
	ImGui::Separator();
	ImGui::TextUnformatted("Point cloud reload");
	ImGui::InputText("Path", m_point_cloud.path_buffer.data(), m_point_cloud.path_buffer.size());
	if (ImGui::Button("Load / Reload"))
		reload_point_cloud_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to PLY sample"))
		copy_path_to_buffer(m_point_cloud.path_buffer, m_sample_project.point_cloud_path);
	ImGui::SameLine();
	if (ImGui::Button("Reset to text sample"))
		copy_path_to_buffer(m_point_cloud.path_buffer, m_legacy_text_point_cloud_path);
	ImGui::Text("Active source: %s", m_point_cloud.using_loaded_data ? m_point_cloud.active_source.c_str() : "procedural fallback");
	ImGui::Text("Last reload: %s", m_point_cloud.last_reload_status.c_str());
	ImGui::Text("Successful reloads: %d", m_point_cloud.reload_count);
	ImGui::Text("CPU point samples: %zu", m_point_cloud.data.size());
	ImGui::Text("Skipped input lines: %zu", m_point_cloud.load.skipped_lines);
	for (std::string const& message : m_point_cloud.load.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_camera_pose_section()
{
	ImGui::TextUnformatted("Camera poses");
	ImGui::Text("Camera pose source: %s", m_camera_poses.active_source.c_str());
	ImGui::Text("Camera poses: %zu", m_camera_poses.data.size());
	ImGui::Text("Pose format: %s", m_camera_poses.load.source_format.empty() ? "<unknown>" : m_camera_poses.load.source_format.c_str());
	ImGui::Text("Skipped pose lines: %zu", m_camera_poses.load.skipped_lines);
	ImGui::Text("Camera line vertices: %d", m_renderer.camera_line_vertex_count());
	draw_camera_pose_metadata(m_camera_poses.data, m_camera_poses.selected_pose);
	for (std::string const& message : m_camera_poses.load.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
	ImGui::TextUnformatted("Camera pose reload");
	ImGui::InputText("Pose path", m_camera_poses.path_buffer.data(), m_camera_poses.path_buffer.size());
	if (ImGui::Button("Load / Reload poses"))
		reload_camera_poses_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to pose sample"))
		copy_path_to_buffer(m_camera_poses.path_buffer, m_sample_project.camera_pose_path);
	ImGui::Text("Active pose source: %s", m_camera_poses.using_loaded_data ? m_camera_poses.active_source.c_str() : "procedural fallback");
	ImGui::Text("Last pose reload: %s", m_camera_poses.last_reload_status.c_str());
	ImGui::Text("Successful pose reloads: %d", m_camera_poses.reload_count);
	ImGui::Separator();
}

void SfmSandboxApp::draw_renderer_section()
{
	ImGui::TextUnformatted("Renderer status");
	ImGui::Text("Renderer: %s", m_renderer.ready() ? "ready" : "failed");
	ImGui::Text("Surface renderer: %s", m_surface_renderer.ready() ? "ready" : "not ready");
	ImGui::Text("Image-plane renderer: %s", m_image_plane_renderer.ready() ? "ready" : "not ready");
	ImGui::Text("Marker renderer: %s", m_marker_renderer.ready() ? "ready" : "not ready");
	ImGui::Text("Grid/axis line vertices: %d", m_renderer.line_vertex_count());
	ImGui::Text("Bounds line vertices: %d", m_renderer.bounds_line_vertex_count());
	ImGui::Text("GPU point vertices: %d", m_renderer.point_count());
	draw_renderer_frame_statistics(m_renderer.frame_statistics(), m_surface_renderer.frame_statistics(), m_image_plane_renderer.frame_statistics(), m_marker_renderer.frame_statistics());
	for (std::string const& message : m_renderer_result.messages)
		ImGui::BulletText("%s", message.c_str());
	ImGui::Separator();
}

void SfmSandboxApp::draw_regression_probe_section()
{
	ImGui::TextUnformatted("Milestone 3 regression: ShaderProgram cache");
	ImGui::Text("Shader probe: %s", m_shader_probe.passed ? "passed" : "failed");
	ImGui::TextUnformatted("Milestone 2 regression: GPU RAII ownership");
	ImGui::Text("Ownership probe: %s", m_ownership_probe.passed ? "passed" : "failed");
}

} // namespace sfm::app
