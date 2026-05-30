#include "config.hpp"
#include "core/Bonobo.h"
#include "sandbox/core/FrameClock.hpp"
#include "sandbox/gfx/ClearPass.hpp"
#include "sandbox/gfx/OwnershipProbe.hpp"
#include "sandbox/gfx/Renderer.hpp"
#include "sandbox/gfx/ShaderProgramProbe.hpp"
#include "sandbox/scene/CameraPose.hpp"
#include "sandbox/scene/CameraPoseLoader.hpp"
#include "sandbox/scene/PointCloud.hpp"
#include "sandbox/scene/PointCloudLoader.hpp"

#include <imgui.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <clocale>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>

namespace
{

constexpr char const* kDefaultPointCloudResource = "sandbox/sample_point_cloud_ascii.ply";
constexpr char const* kLegacyTextPointCloudResource = "sandbox/sample_point_cloud.xyzrgb";
constexpr char const* kDefaultCameraPoseResource = "sandbox/sample_camera_poses.txt";

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

char const* colour_mode_label(sfm::gfx::PointColourMode mode)
{
	switch (mode) {
	case sfm::gfx::PointColourMode::Source: return "Source colour";
	case sfm::gfx::PointColourMode::Height: return "Height gradient";
	case sfm::gfx::PointColourMode::Solid: return "Solid colour";
	}
	return "Unknown";
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

} // namespace

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(), static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y), 0.01f, 1000.0f);
	camera.mWorld.SetTranslate(glm::vec3(0.0f, 3.0f, 8.0f));
	camera.mMovementSpeed = glm::vec3(6.0f);
	camera.mMouseSensitivity = glm::vec2(0.003f);

	WindowManager& window_manager = framework.GetWindowManager();
	WindowManager::WindowDatum window_datum{ input_handler, camera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };
	GLFWwindow* const window = window_manager.CreateGLFWWindow("SfM Visualization Sandbox", window_datum, config::msaa_rate, false, true);
	if (window == nullptr) {
		LogError("Failed to create the SfMSandbox window: exiting.");
		return EXIT_FAILURE;
	}

	sfm::core::FrameClock frame_clock;
	sfm::gfx::ClearPass clear_pass({ 0.035f, 0.055f, 0.090f, 1.0f });
	clear_pass.initialise();
	sfm::gfx::OwnershipProbeResult const ownership_probe = sfm::gfx::run_ownership_probe("SfmSandbox ownership probe");
	sfm::gfx::ShaderProgramProbeResult const shader_program_probe = sfm::gfx::run_shader_program_probe();

	std::filesystem::path const default_point_cloud_path = config::resources_path(kDefaultPointCloudResource);
	std::filesystem::path const legacy_text_point_cloud_path = config::resources_path(kLegacyTextPointCloudResource);
	std::array<char, 512> point_cloud_path_buffer{};
	copy_path_to_buffer(point_cloud_path_buffer, default_point_cloud_path);

	sfm::scene::PointCloudLoadResult point_cloud_load = sfm::scene::load_point_cloud_from_file(default_point_cloud_path);
	bool using_loaded_point_cloud = point_cloud_load.succeeded;
	sfm::scene::PointCloud point_cloud = using_loaded_point_cloud ? std::move(point_cloud_load.cloud) : sfm::scene::PointCloud::make_debug_cluster();
	std::string active_point_cloud_source = using_loaded_point_cloud ? default_point_cloud_path.string() : "procedural fallback";
	std::string last_reload_status = using_loaded_point_cloud ? "Initial point cloud loaded" : "Initial load failed; using procedural fallback";
	int reload_count = 0;

	std::filesystem::path const default_camera_pose_path = config::resources_path(kDefaultCameraPoseResource);
	sfm::scene::CameraPoseLoadResult camera_pose_load = sfm::scene::load_camera_poses_from_text_file(default_camera_pose_path);
	bool const using_loaded_camera_poses = camera_pose_load.succeeded;
	sfm::scene::CameraPoseSet const camera_poses = using_loaded_camera_poses ? std::move(camera_pose_load.poses) : sfm::scene::CameraPoseSet::make_debug_orbit();
	std::string const active_camera_pose_source = using_loaded_camera_poses ? default_camera_pose_path.string() : "procedural fallback";

	sfm::gfx::PointCloudRenderSettings point_settings{};
	sfm::gfx::Renderer renderer;
	sfm::gfx::RendererBuildResult renderer_build = renderer.initialise(point_cloud, camera_poses);

	bool show_gui = true;
	bool show_logs = false;

	while (!glfwWindowShouldClose(window)) {
		sfm::core::FrameTiming const frame_timing = frame_clock.tick();
		auto const delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(frame_timing.delta_seconds));

		glfwPollEvents();
		ImGuiIO const& io = ImGui::GetIO();
		input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
		input_handler.Advance();
		camera.Update(delta_time, input_handler);

		if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (input_handler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			window_manager.ToggleFullscreenStatusForWindow(window);

		int framebuffer_width = 0;
		int framebuffer_height = 0;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		if (framebuffer_width > 0 && framebuffer_height > 0)
			camera.SetAspect(static_cast<float>(framebuffer_width) / static_cast<float>(framebuffer_height));

		window_manager.NewImGuiFrame();
		clear_pass.render(framebuffer_width, framebuffer_height);
		renderer.render(camera.GetWorldToClipMatrix(), point_settings);

		if (ImGui::Begin("Sandbox status")) {
			ImGui::TextUnformatted("SfM Visualization Sandbox");
			ImGui::Separator();
			ImGui::Text("Frame: %llu", static_cast<unsigned long long>(frame_timing.frame_index));
			ImGui::Text("CPU frame: %.3f ms", frame_timing.delta_milliseconds);
			ImGui::Text("FPS: %.1f", frame_timing.frames_per_second);
			ImGui::Text("Framebuffer: %d x %d", framebuffer_width, framebuffer_height);
			ImGui::Text("Camera aspect: %.3f", camera.GetAspect());
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 13: Point-cloud inspection controls and UX polish");
			draw_point_cloud_statistics(point_cloud.statistics());
			ImGui::Separator();
			ImGui::TextUnformatted("Point display");
			draw_point_controls(point_settings);
			ImGui::Separator();
			ImGui::TextUnformatted("Camera poses");
			ImGui::Text("Camera pose source: %s", active_camera_pose_source.c_str());
			ImGui::Text("Camera poses: %zu", camera_poses.size());
			ImGui::Text("Skipped pose lines: %zu", camera_pose_load.skipped_lines);
			for (std::string const& message : camera_pose_load.messages)
				ImGui::BulletText("%s", message.c_str());
			ImGui::Text("Camera line vertices: %d", renderer.camera_line_vertex_count());
			ImGui::Separator();
			ImGui::TextUnformatted("Point cloud reload");
			ImGui::InputText("Path", point_cloud_path_buffer.data(), point_cloud_path_buffer.size());
			if (ImGui::Button("Load / Reload")) {
				std::filesystem::path const requested_path{ std::string{ point_cloud_path_buffer.data() } };
				sfm::scene::PointCloudLoadResult candidate_load = sfm::scene::load_point_cloud_from_file(requested_path);
				if (candidate_load.succeeded) {
					sfm::scene::PointCloud candidate_cloud = std::move(candidate_load.cloud);
					sfm::gfx::RendererBuildResult candidate_renderer_build = renderer.reload_point_cloud(candidate_cloud);
					if (candidate_renderer_build.succeeded) {
						point_cloud = std::move(candidate_cloud);
						point_cloud_load = std::move(candidate_load);
						renderer_build = std::move(candidate_renderer_build);
						using_loaded_point_cloud = true;
						active_point_cloud_source = requested_path.string();
						last_reload_status = "Reload succeeded";
						++reload_count;
					} else {
						renderer_build = std::move(candidate_renderer_build);
						last_reload_status = "Reload failed during GPU upload; previous cloud kept";
					}
				} else {
					point_cloud_load = std::move(candidate_load);
					last_reload_status = "Reload failed during file load; previous cloud kept";
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset to PLY sample"))
				copy_path_to_buffer(point_cloud_path_buffer, default_point_cloud_path);
			ImGui::SameLine();
			if (ImGui::Button("Reset to text sample"))
				copy_path_to_buffer(point_cloud_path_buffer, legacy_text_point_cloud_path);
			ImGui::Text("Active source: %s", using_loaded_point_cloud ? active_point_cloud_source.c_str() : "procedural fallback");
			ImGui::Text("Last reload: %s", last_reload_status.c_str());
			ImGui::Text("Successful reloads: %d", reload_count);
			ImGui::Text("CPU point samples: %zu", point_cloud.size());
			ImGui::Text("Skipped input lines: %zu", point_cloud_load.skipped_lines);
			for (std::string const& message : point_cloud_load.messages)
				ImGui::BulletText("%s", message.c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Renderer status");
			ImGui::Text("Renderer: %s", renderer.ready() ? "ready" : "failed");
			ImGui::Text("Grid/axis line vertices: %d", renderer.line_vertex_count());
			ImGui::Text("Bounds line vertices: %d", renderer.bounds_line_vertex_count());
			ImGui::Text("GPU point vertices: %d", renderer.point_count());
			for (std::string const& message : renderer_build.messages)
				ImGui::BulletText("%s", message.c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 3 regression: ShaderProgram cache");
			ImGui::Text("Shader probe: %s", shader_program_probe.passed ? "passed" : "failed");
			ImGui::TextUnformatted("Milestone 2 regression: GPU RAII ownership");
			ImGui::Text("Ownership probe: %s", ownership_probe.passed ? "passed" : "failed");
			ImGui::Separator();
			ImGui::TextUnformatted("Controls: WASD/QE move, left mouse drag look, F2 UI, F3 logs, F11 fullscreen, Esc quit");
		}
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		window_manager.RenderImGuiFrame(show_gui);
		glfwSwapBuffers(window);
	}

	window_manager.DestroyWindow(window);
	return EXIT_SUCCESS;
}
