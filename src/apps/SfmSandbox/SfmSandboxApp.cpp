#include "SfmSandboxApp.hpp"

#include "SfmSandboxCaptureMetadata.hpp"
#include "SfmSandboxMeshBuilderPanel.hpp"
#include "SfmSandboxPanels.hpp"
#include "SfmSandboxUi.hpp"
#include "config.hpp"
#include "sandbox/gfx/FramebufferCapture.hpp"

#include <imgui.h>

#include <algorithm>
#include <optional>
#include <utility>

namespace sfm::app
{
namespace
{

constexpr char const* kLegacyTextPointCloudResource = "sandbox/sample_point_cloud.xyzrgb";
constexpr char const* kDefaultMeshExportPath = "build/sandbox-output/generated_surface.obj";

void copy_path_to_buffer(std::array<char, 512>& buffer, std::filesystem::path const& path)
{
	std::string const text = path.string();
	buffer.fill('\0');
	std::size_t const count = std::min(text.size(), buffer.size() - 1u);
	std::copy_n(text.data(), count, buffer.data());
}

} // namespace

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
	m_mesh_builder.configure(m_startup_options.mesh_build_options, std::filesystem::path{ kDefaultMeshExportPath });
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
	if (m_active_project.camera_pose_path.empty()) {
		m_camera_poses.load = {};
		m_camera_poses.data = {};
		m_camera_poses.using_loaded_data = false;
		m_camera_poses.visible = false;
		m_camera_poses.active_source = "disabled";
		m_camera_poses.last_reload_status = "Camera poses disabled by empty project parameter";
		return;
	}
	m_camera_poses.load = sfm::scene::load_camera_poses_from_file(m_active_project.camera_pose_path);
	m_camera_poses.using_loaded_data = m_camera_poses.load.succeeded;
	m_camera_poses.data = m_camera_poses.using_loaded_data ? std::move(m_camera_poses.load.poses) : sfm::scene::CameraPoseSet{};
	m_camera_poses.active_source = m_camera_poses.using_loaded_data ? m_active_project.camera_pose_path.string() : "no camera poses loaded";
	m_camera_poses.last_reload_status = m_camera_poses.using_loaded_data ? "Initial camera poses loaded" : "Initial camera pose load failed; camera layer off";
	m_camera_poses.visible = m_camera_poses.using_loaded_data && !m_camera_poses.data.empty();
}

void SfmSandboxApp::initialise_surface()
{
	copy_path_to_buffer(m_surface.path_buffer, m_active_project.surface_path);
	if (m_active_project.surface_path.empty()) {
		m_surface.load = {};
		m_surface.data = {};
		m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
		m_surface.using_loaded_data = false;
		m_surface.visible = false;
		m_surface.active_source = "disabled";
		m_surface.source_kind = "disabled";
		m_surface.last_reload_status = "Surface disabled by empty project parameter";
		return;
	}
	m_surface.load = sfm::scene::import_surface(m_active_project.surface_path);
	m_surface.using_loaded_data = m_surface.load.succeeded;
	m_surface.data = m_surface.using_loaded_data ? std::move(m_surface.load.mesh) : sfm::scene::SurfaceMesh{};
	m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
	m_surface.active_source = m_surface.using_loaded_data ? m_active_project.surface_path.string() : "no surface loaded";
	m_surface.source_kind = m_surface.using_loaded_data ? "imported" : "empty";
	m_surface.last_reload_status = m_surface.using_loaded_data ? "Initial surface loaded" : "Initial surface load failed";
	m_surface.visible = m_surface.using_loaded_data;
}

void SfmSandboxApp::initialise_image()
{
	copy_path_to_buffer(m_image.path_buffer, m_active_project.image_path);
	if (m_active_project.image_path.empty()) {
		m_image.load = {};
		m_image.data = {};
		m_image.using_loaded_data = false;
		m_image.visible = false;
		m_image.active_source = "disabled";
		m_image.last_reload_status = "Image plane disabled by empty project parameter";
		return;
	}
	m_image.load = sfm::scene::import_image(m_active_project.image_path);
	m_image.using_loaded_data = m_image.load.succeeded;
	m_image.data = m_image.using_loaded_data ? std::move(m_image.load.image) : sfm::scene::ImageResource{};
	m_image.active_source = m_image.using_loaded_data ? m_active_project.image_path.string() : "no image loaded";
	m_image.last_reload_status = m_image.using_loaded_data ? "Initial image loaded" : "Initial image load failed";
	m_image.visible = m_image.using_loaded_data;
}

void SfmSandboxApp::initialise_renderers()
{
	m_renderer_result = m_renderer.initialise(m_point_cloud.data, m_camera_poses.data);
	if (!m_surface.data.empty())
		m_surface_renderer_result = m_surface_renderer.reload(m_surface.data, m_surface.colour);
	if (!m_image.data.empty())
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
		m_renderer.render(world_to_clip, m_point_settings, m_camera_poses.visible);
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
	BaselineCaptureMetadata metadata{};
	metadata.framebuffer_width = framebuffer_width;
	metadata.framebuffer_height = framebuffer_height;
	metadata.project_manifest = m_active_project.manifest_path;
	metadata.project_loaded = m_project_load.succeeded;
	metadata.point_cloud_source = m_point_cloud.active_source;
	metadata.point_count = m_point_cloud.data.size();
	metadata.camera_pose_source = m_camera_poses.active_source;
	metadata.camera_pose_count = m_camera_poses.data.size();
	metadata.camera_poses_visible = m_camera_poses.visible;
	metadata.surface_source = m_surface.active_source;
	metadata.surface_kind = m_surface.source_kind;
	metadata.surface_source_format = m_surface.load.source_format;
	metadata.surface_visible = m_surface.visible;
	metadata.surface_statistics = m_surface.statistics;
	metadata.generated_mesh_build = (m_surface.source_kind == "generated") ? &m_mesh_builder.build() : nullptr;
	metadata.image_source = m_image.active_source;
	metadata.image_visible = m_image.visible;
	metadata.image_width = m_image.data.width;
	metadata.image_height = m_image.data.height;
	metadata.marker_count = m_marker_renderer.frame_statistics().marker_count;
	metadata.marker_visible = m_marker_settings.visible;
	metadata.marker_instancing = m_marker_settings.use_instancing;
	metadata.point_colour_mode = static_cast<int>(m_point_settings.colour_mode);
	metadata.point_size = m_point_settings.point_size;

	std::string metadata_error{};
	if (!write_baseline_capture_metadata(metadata_path, metadata, metadata_error)) {
		m_capture_messages.emplace_back("Baseline capture failed: " + metadata_error);
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
	if (requested_path.empty()) {
		sfm::scene::CameraPoseSet empty_poses{};
		sfm::gfx::RendererBuildResult candidate_renderer_build = m_renderer.reload_camera_poses(empty_poses);
		if (candidate_renderer_build.succeeded)
			m_renderer_result = std::move(candidate_renderer_build);
		m_camera_poses.load = {};
		m_camera_poses.data = std::move(empty_poses);
		m_camera_poses.using_loaded_data = false;
		m_camera_poses.visible = false;
		m_camera_poses.active_source = "disabled";
		m_camera_poses.last_reload_status = "Camera poses disabled by empty path";
		m_camera_poses.selected_pose = 0;
		m_image.associated_camera_index = 0;
		++m_camera_poses.reload_count;
		return;
	}
	sfm::scene::CameraPoseLoadResult candidate_load = sfm::scene::load_camera_poses_from_file(requested_path);
	if (!candidate_load.succeeded) {
		m_camera_poses.load = std::move(candidate_load);
		m_camera_poses.last_reload_status = "Pose reload failed during file load; previous pose layer kept";
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
	m_camera_poses.visible = !m_camera_poses.data.empty();
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
	if (requested_path.empty()) {
		m_surface.load = {};
		m_surface.data = {};
		m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
		m_surface.using_loaded_data = false;
		m_surface.visible = false;
		m_surface.active_source = "disabled";
		m_surface.source_kind = "disabled";
		m_surface.last_reload_status = "Surface disabled by empty path";
		++m_surface.reload_count;
		return;
	}
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
	m_surface.source_kind = "imported";
	m_surface.last_reload_status = "Surface reload succeeded";
	++m_surface.reload_count;
}

void SfmSandboxApp::reload_image_from_ui()
{
	std::filesystem::path const requested_path{ std::string{ m_image.path_buffer.data() } };
	if (requested_path.empty()) {
		m_image.load = {};
		m_image.data = {};
		m_image.using_loaded_data = false;
		m_image.visible = false;
		m_image.active_source = "disabled";
		m_image.last_reload_status = "Image plane disabled by empty path";
		++m_image.reload_count;
		return;
	}
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

void SfmSandboxApp::build_surface_from_point_cloud()
{
	m_mesh_builder.request_build(m_point_cloud.data);
}

void SfmSandboxApp::poll_mesh_build_result()
{
	std::optional<sfm::scene::PointToMeshBuildResult> build_result = m_mesh_builder.poll_completed_build();
	if (build_result.has_value())
		apply_mesh_build_result(std::move(*build_result));
}

void SfmSandboxApp::apply_mesh_build_result(sfm::scene::PointToMeshBuildResult build_result)
{
	if (!build_result.succeeded) {
		m_mesh_builder.record_build_failure(std::move(build_result));
		return;
	}

	sfm::gfx::SurfaceRendererResult candidate_render = m_surface_renderer.reload(build_result.mesh, m_surface.colour);
	if (!candidate_render.succeeded) {
		m_surface_renderer_result = std::move(candidate_render);
		m_mesh_builder.record_upload_failure(std::move(build_result));
		return;
	}

	m_surface.data = build_result.mesh;
	m_surface.statistics = sfm::scene::statistics_for(m_surface.data);
	m_surface.load = {};
	m_surface.load.succeeded = true;
	m_surface.load.source_format = build_result.algorithm;
	m_surface.load.messages = build_result.messages;
	m_surface_renderer_result = std::move(candidate_render);
	m_surface.using_loaded_data = true;
	m_surface.visible = true;
	m_surface.active_source = "generated from point cloud: " + m_point_cloud.active_source;
	m_surface.source_kind = "generated";
	m_surface.last_reload_status = "Point-cloud mesh build succeeded";
	m_mesh_builder.record_build_success(std::move(build_result));
}

void SfmSandboxApp::export_active_surface_from_ui()
{
	m_mesh_builder.export_surface(m_surface.data);
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
	ui::set_next_inspector_window(framebuffer_width, framebuffer_height, m_ui_scale);
	if (ImGui::Begin("Sandbox inspector")) {
		if (ImGui::BeginTable("summary_metrics", 4, ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ui::metric("FPS", std::to_string(static_cast<int>(frame_timing.frames_per_second)));
			ImGui::TableNextColumn();
			ui::metric("Points", std::to_string(m_point_cloud.data.size()));
			ImGui::TableNextColumn();
			ui::metric("Triangles", std::to_string(m_surface.statistics.triangle_count));
			ImGui::TableNextColumn();
			ui::metric("Cameras", std::to_string(m_camera_poses.data.size()));
			ImGui::EndTable();
		}
		ImGui::Separator();

		if (ImGui::BeginTabBar("sandbox_inspector_tabs")) {
			if (ImGui::BeginTabItem("Overview")) {
				draw_overview_tab(frame_timing, framebuffer_width, framebuffer_height, camera_aspect);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Assets")) {
				draw_assets_tab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Mesh")) {
				draw_mesh_builder_section();
				ImGui::Separator();
				draw_surface_section();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Render")) {
				draw_render_tab(frame_profiler);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Diagnostics")) {
				draw_diagnostics_tab(frame_timing, frame_profiler, framebuffer_width, framebuffer_height, camera_aspect);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void SfmSandboxApp::draw_overview_tab(sfm::core::FrameTiming const& frame_timing,
                                      int framebuffer_width,
                                      int framebuffer_height,
                                      float camera_aspect)
{
	ImGui::SliderFloat("UI scale", &m_ui_scale, 1.0f, 2.0f, "%.2f x");
	if (ui::begin_property_table("overview_runtime")) {
		ui::property("Frame", std::to_string(static_cast<unsigned long long>(frame_timing.frame_index)));
		ui::property_float("CPU frame", frame_timing.delta_milliseconds, "%.3f ms");
		ui::property_float("FPS", frame_timing.frames_per_second, "%.1f");
		ui::property("Framebuffer", std::to_string(framebuffer_width) + " x " + std::to_string(framebuffer_height));
		ui::property_float("Camera aspect", camera_aspect);
		ui::end_property_table();
	}
	ImGui::Spacing();
	draw_project_section();
	ImGui::Spacing();
	if (ui::begin_property_table("overview_scene")) {
		ui::property("Point cloud", m_point_cloud.active_source);
		ui::property("Point samples", m_point_cloud.data.size());
		ui::property("Camera poses", m_camera_poses.data.size());
		ui::property("Surface kind", m_surface.source_kind);
		ui::property("Surface triangles", m_surface.statistics.triangle_count);
		ui::property("Image", m_image.active_source);
		ui::end_property_table();
	}
}

void SfmSandboxApp::draw_assets_tab()
{
	if (ImGui::CollapsingHeader("Point cloud", ImGuiTreeNodeFlags_DefaultOpen))
		draw_point_cloud_section();
	if (ImGui::CollapsingHeader("Camera poses"))
		draw_camera_pose_section();
	if (ImGui::CollapsingHeader("Image"))
		draw_image_section();
	if (ImGui::CollapsingHeader("Surface"))
		draw_surface_section();
}

void SfmSandboxApp::draw_render_tab(sfm::core::FrameProfiler const& frame_profiler)
{
	if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		draw_renderer_section();
	if (ImGui::CollapsingHeader("Markers"))
		draw_marker_stress_section();
	if (ImGui::CollapsingHeader("Targets and timing"))
		draw_render_target_section(frame_profiler);
}

void SfmSandboxApp::draw_diagnostics_tab(sfm::core::FrameTiming const& frame_timing,
                                         sfm::core::FrameProfiler const& frame_profiler,
                                         int framebuffer_width,
                                         int framebuffer_height,
                                         float camera_aspect)
{
	if (ui::begin_property_table("diagnostics_runtime")) {
		ui::property("Frame", std::to_string(static_cast<unsigned long long>(frame_timing.frame_index)));
		ui::property_float("CPU frame", frame_timing.delta_milliseconds, "%.3f ms");
		ui::property("Framebuffer", std::to_string(framebuffer_width) + " x " + std::to_string(framebuffer_height));
		ui::property_float("Camera aspect", camera_aspect);
		ui::end_property_table();
	}
	ImGui::Spacing();
	draw_frame_profiler(frame_profiler);
	ImGui::Spacing();
	draw_gpu_profiler_section();
	ImGui::Spacing();
	draw_regression_probe_section();
	ui::draw_messages("Startup warnings", m_startup_options.messages);
	ui::draw_messages("Project messages", m_project_load.messages);
}

void SfmSandboxApp::draw_project_section()
{
	if (!ui::begin_property_table("project_manifest"))
		return;
	ui::property("Manifest", m_active_project.manifest_path.string());
	ui::property("Project load", m_project_load.succeeded, "loaded", "sample fallback");
	ui::end_property_table();
	ui::draw_messages("Startup messages", m_startup_options.messages);
	ui::draw_messages("Project messages", m_project_load.messages);
}

void SfmSandboxApp::draw_marker_stress_section()
{
	bool marker_rebuild_requested = false;
	marker_rebuild_requested |= ImGui::Checkbox("Show marker stress scene", &m_marker_settings.visible);
	ImGui::Checkbox("Use instanced marker path", &m_marker_settings.use_instancing);
	marker_rebuild_requested |= ImGui::SliderInt("Stress marker count", &m_marker_settings.marker_count, 1, 5000);
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker radius", &m_marker_settings.radius, 0.5f, 10.0f, "%.2f");
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker height", &m_marker_settings.height, -2.0f, 4.0f, "%.2f");
	marker_rebuild_requested |= ImGui::SliderFloat("Stress marker scale", &m_marker_settings.marker_scale, 0.01f, 0.25f, "%.3f");
	if (marker_rebuild_requested || ImGui::Button("Rebuild marker stress scene"))
		m_marker_renderer_result = m_marker_renderer.rebuild(m_marker_settings);
	if (ui::begin_property_table("marker_status")) {
		ui::property("Marker renderer", m_marker_renderer.ready(), "ready", "not ready");
		ui::end_property_table();
	}
	draw_marker_stress_statistics(m_marker_renderer.frame_statistics());
	ui::draw_messages("Marker messages", m_marker_renderer_result.messages);
}

void SfmSandboxApp::draw_render_target_section(sfm::core::FrameProfiler const& frame_profiler)
{
	draw_render_target_status(m_offscreen_probe.status());
	ImGui::Spacing();
	draw_frame_profiler(frame_profiler);
	ImGui::Spacing();
	draw_gpu_profiler_section();
}

void SfmSandboxApp::draw_gpu_profiler_section()
{
	sfm::gfx::GpuProfilerSnapshot const snapshot = m_gpu_profiler.snapshot();
	if (ui::begin_property_table("gpu_profiler_status")) {
		ui::property("Timer queries", snapshot.supported, "available", "unavailable");
		ui::property("Pending frames", snapshot.pending_frame_count);
		ui::property("Status", snapshot.message);
		ui::end_property_table();
	}
	if (snapshot.passes.empty()) {
		ImGui::TextUnformatted("Last completed GPU passes: unavailable");
		return;
	}
	for (sfm::gfx::GpuTimedPass const& pass : snapshot.passes)
		ImGui::BulletText("%s: %.3f ms", pass.name.c_str(), pass.milliseconds);
}

void SfmSandboxApp::draw_image_section()
{
	draw_optional_visibility_checkbox("Show associated image plane", m_image.visible, !m_image.data.empty());
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
	draw_path_input("Image path", m_image.path_buffer);
	if (ImGui::Button("Load / Reload image"))
		reload_image_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to image sample"))
		copy_path_to_buffer(m_image.path_buffer, m_sample_project.image_path);
	if (ui::begin_property_table("image_reload_status")) {
		ui::property("Active source", m_image.using_loaded_data ? m_image.active_source : "no image loaded");
		ui::property("Last reload", m_image.last_reload_status);
		ui::property("Reloads", m_image.reload_count);
		ui::property("Renderer", m_image_plane_renderer.ready(), "ready", "not ready");
		ui::end_property_table();
	}
	ui::draw_messages("Image load messages", m_image.load.messages);
	ui::draw_messages("Image renderer messages", m_image_plane_result.messages);
}

void SfmSandboxApp::draw_surface_section()
{
	draw_optional_visibility_checkbox("Show surface mesh", m_surface.visible, !m_surface.data.empty());
	if (ImGui::ColorEdit3("Surface colour", &m_surface.colour.x))
		update_surface_colour();
	if (ImGui::Button("Rebuild surface colour"))
		m_surface_renderer_result = m_surface_renderer.reload(m_surface.data, m_surface.colour);
	draw_surface_statistics(m_surface.statistics);
	if (ui::begin_property_table("surface_status")) {
		ui::property("Kind", m_surface.source_kind);
		ui::property("Source", m_surface.active_source);
		ui::property("Format", m_surface.load.source_format.empty() ? "<none>" : m_surface.load.source_format);
		ui::property("Renderer", m_surface_renderer.ready(), "ready", "not ready");
		ui::property("GPU triangles", m_surface_renderer.triangle_count());
		ui::end_property_table();
	}
	draw_path_input("Surface path", m_surface.path_buffer);
	if (ImGui::Button("Load / Reload surface"))
		reload_surface_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to surface sample"))
		copy_path_to_buffer(m_surface.path_buffer, m_sample_project.surface_path);
	if (ui::begin_property_table("surface_reload_status")) {
		ui::property("Active source", m_surface.using_loaded_data ? m_surface.active_source : "no surface loaded");
		ui::property("Last reload", m_surface.last_reload_status);
		ui::property("Reloads", m_surface.reload_count);
		ui::property("Skipped lines", m_surface.load.skipped_lines);
		ui::end_property_table();
	}
	ui::draw_messages("Surface load messages", m_surface.load.messages);
	ui::draw_messages("Surface renderer messages", m_surface_renderer_result.messages);
}

void SfmSandboxApp::draw_mesh_builder_section()
{
	poll_mesh_build_result();
	MeshBuilderPanelActions const actions = draw_mesh_builder_panel(m_mesh_builder);
	if (actions.build_requested)
		build_surface_from_point_cloud();
	if (actions.export_requested)
		export_active_surface_from_ui();
}

void SfmSandboxApp::draw_point_cloud_section()
{
	draw_point_controls(m_point_settings);
	ImGui::Spacing();
	draw_point_cloud_statistics(m_point_cloud.data.statistics());
	ImGui::Spacing();
	draw_path_input("Point cloud path", m_point_cloud.path_buffer);
	if (ImGui::Button("Load / Reload"))
		reload_point_cloud_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to PLY sample"))
		copy_path_to_buffer(m_point_cloud.path_buffer, m_sample_project.point_cloud_path);
	ImGui::SameLine();
	if (ImGui::Button("Reset to text sample"))
		copy_path_to_buffer(m_point_cloud.path_buffer, m_legacy_text_point_cloud_path);
	if (ui::begin_property_table("point_cloud_reload_status")) {
		ui::property("Active source", m_point_cloud.using_loaded_data ? m_point_cloud.active_source : "procedural fallback");
		ui::property("Last reload", m_point_cloud.last_reload_status);
		ui::property("Reloads", m_point_cloud.reload_count);
		ui::property("CPU samples", m_point_cloud.data.size());
		ui::property("Skipped lines", m_point_cloud.load.skipped_lines);
		ui::end_property_table();
	}
	ui::draw_messages("Point-cloud load messages", m_point_cloud.load.messages);
}

void SfmSandboxApp::draw_camera_pose_section()
{
	draw_optional_visibility_checkbox("Show camera poses", m_camera_poses.visible, !m_camera_poses.data.empty());

	if (ui::begin_property_table("camera_pose_status")) {
		ui::property("Source", m_camera_poses.active_source);
		ui::property("Visible", m_camera_poses.visible);
		ui::property("Poses", m_camera_poses.data.size());
		ui::property("Format", m_camera_poses.load.source_format.empty() ? "<unknown>" : m_camera_poses.load.source_format);
		ui::property("Skipped lines", m_camera_poses.load.skipped_lines);
		ui::property("Line vertices", m_renderer.camera_line_vertex_count());
		ui::end_property_table();
	}
	draw_camera_pose_metadata(m_camera_poses.data, m_camera_poses.selected_pose);
	ui::draw_messages("Pose load messages", m_camera_poses.load.messages);
	draw_path_input("Camera pose path", m_camera_poses.path_buffer);
	if (ImGui::Button("Load / Reload poses"))
		reload_camera_poses_from_ui();
	ImGui::SameLine();
	if (ImGui::Button("Reset to pose sample"))
		copy_path_to_buffer(m_camera_poses.path_buffer, m_sample_project.camera_pose_path);
	if (ui::begin_property_table("camera_pose_reload_status")) {
		ui::property("Active source", m_camera_poses.active_source);
		ui::property("Last reload", m_camera_poses.last_reload_status);
		ui::property("Reloads", m_camera_poses.reload_count);
		ui::end_property_table();
	}
}

void SfmSandboxApp::draw_renderer_section()
{
	if (ui::begin_property_table("renderer_status")) {
		ui::property("Renderer", m_renderer.ready(), "ready", "failed");
		ui::property("Surface renderer", m_surface_renderer.ready(), "ready", "not ready");
		ui::property("Image renderer", m_image_plane_renderer.ready(), "ready", "not ready");
		ui::property("Marker renderer", m_marker_renderer.ready(), "ready", "not ready");
		ui::property("Grid/axis vertices", m_renderer.line_vertex_count());
		ui::property("Bounds vertices", m_renderer.bounds_line_vertex_count());
		ui::property("Point vertices", m_renderer.point_count());
		ui::end_property_table();
	}
	draw_renderer_frame_statistics(m_renderer.frame_statistics(), m_surface_renderer.frame_statistics(), m_image_plane_renderer.frame_statistics(), m_marker_renderer.frame_statistics());
	ui::draw_messages("Renderer messages", m_renderer_result.messages);
}

void SfmSandboxApp::draw_regression_probe_section()
{
	if (!ui::begin_property_table("regression_probe_status"))
		return;
	ui::property("Shader cache probe", m_shader_probe.passed, "passed", "failed");
	ui::property("GPU ownership probe", m_ownership_probe.passed, "passed", "failed");
	ui::end_property_table();
}

} // namespace sfm::app
