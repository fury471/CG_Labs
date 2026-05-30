#include "config.hpp"
#include "core/Bonobo.h"
#include "sandbox/core/FrameClock.hpp"
#include "sandbox/gfx/ClearPass.hpp"
#include "sandbox/gfx/OwnershipProbe.hpp"
#include "sandbox/gfx/Renderer.hpp"
#include "sandbox/gfx/ShaderProgramProbe.hpp"
#include "sandbox/scene/CameraPose.hpp"
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

constexpr char const* kDefaultPointCloudResource = "sandbox/sample_point_cloud.xyzrgb";

void copy_path_to_buffer(std::array<char, 512>& buffer, std::filesystem::path const& path)
{
	std::string const text = path.string();
	buffer.fill('\0');
	std::size_t const count = std::min(text.size(), buffer.size() - 1u);
	std::copy_n(text.data(), count, buffer.data());
}

} // namespace

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(),
	                  static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                  0.01f,
	                  1000.0f);
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

	// Everything below this point may assume that an OpenGL context is current.
	// GPU-owning RAII objects must also be destroyed before DestroyWindow() tears
	// the context down, so keep future long-lived owners inside this lifetime.
	sfm::core::FrameClock frame_clock;
	sfm::gfx::ClearPass clear_pass({ 0.035f, 0.055f, 0.090f, 1.0f });
	clear_pass.initialise();

	// Milestone probes intentionally run once after context creation. They are
	// development diagnostics, not per-frame rendering logic.
	sfm::gfx::OwnershipProbeResult const ownership_probe = sfm::gfx::run_ownership_probe("SfmSandbox ownership probe");
	sfm::gfx::ShaderProgramProbeResult const shader_program_probe = sfm::gfx::run_shader_program_probe();

	// Milestone 8 keeps the resource-file loader from Milestone 7, but makes the
	// source path editable at runtime. A failed reload does not destroy the last
	// valid cloud; the renderer is updated only after a file has loaded correctly.
	std::filesystem::path const default_point_cloud_path = config::resources_path(kDefaultPointCloudResource);
	std::array<char, 512> point_cloud_path_buffer{};
	copy_path_to_buffer(point_cloud_path_buffer, default_point_cloud_path);

	sfm::scene::PointCloudLoadResult point_cloud_load = sfm::scene::load_point_cloud_from_text_file(default_point_cloud_path);
	bool using_loaded_point_cloud = point_cloud_load.succeeded;
	sfm::scene::PointCloud point_cloud = using_loaded_point_cloud
		? std::move(point_cloud_load.cloud)
		: sfm::scene::PointCloud::make_debug_cluster();
	std::string active_point_cloud_source = using_loaded_point_cloud ? default_point_cloud_path.string() : "procedural fallback";
	std::string last_reload_status = using_loaded_point_cloud ? "Initial resource file loaded" : "Initial load failed; using procedural fallback";
	int reload_count = 0;

	// Milestone 9 adds a deterministic camera-pose sample so frustums and a
	// trajectory can be validated before a real SfM camera import exists.
	sfm::scene::CameraPoseSet const camera_poses = sfm::scene::CameraPoseSet::make_debug_orbit();

	sfm::gfx::Renderer renderer;
	sfm::gfx::RendererBuildResult renderer_build = renderer.initialise(point_cloud, camera_poses);

	bool show_gui = true;
	bool show_logs = false;

	while (!glfwWindowShouldClose(window)) {
		sfm::core::FrameTiming const frame_timing = frame_clock.tick();
		auto const delta_time = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::duration<float>(frame_timing.delta_seconds));

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
		if (framebuffer_width > 0 && framebuffer_height > 0) {
			float const aspect = static_cast<float>(framebuffer_width) / static_cast<float>(framebuffer_height);
			camera.SetAspect(aspect);
		}

		window_manager.NewImGuiFrame();
		clear_pass.render(framebuffer_width, framebuffer_height);
		renderer.render(camera.GetWorldToClipMatrix());

		if (ImGui::Begin("Sandbox status")) {
			ImGui::TextUnformatted("SfM Visualization Sandbox");
			ImGui::Separator();
			ImGui::Text("Frame: %llu", static_cast<unsigned long long>(frame_timing.frame_index));
			ImGui::Text("CPU frame: %.3f ms", frame_timing.delta_milliseconds);
			ImGui::Text("FPS: %.1f", frame_timing.frames_per_second);
			ImGui::Text("Framebuffer: %d x %d", framebuffer_width, framebuffer_height);
			ImGui::Text("Camera aspect: %.3f", camera.GetAspect());
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 9: Camera pose / frustum visualization");
			ImGui::Text("Camera poses: %zu", camera_poses.size());
			ImGui::Text("Camera line vertices: %d", renderer.camera_line_vertex_count());
			ImGui::Separator();
			ImGui::TextUnformatted("Point cloud reload");
			ImGui::InputText("Path", point_cloud_path_buffer.data(), point_cloud_path_buffer.size());
			if (ImGui::Button("Load / Reload")) {
				std::filesystem::path const requested_path{ std::string{ point_cloud_path_buffer.data() } };
				sfm::scene::PointCloudLoadResult candidate_load = sfm::scene::load_point_cloud_from_text_file(requested_path);
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
			if (ImGui::Button("Reset to sample")) {
				copy_path_to_buffer(point_cloud_path_buffer, default_point_cloud_path);
			}
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
			ImGui::Text("GPU point vertices: %d", renderer.point_count());
			for (std::string const& message : renderer_build.messages)
				ImGui::BulletText("%s", message.c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 3 regression: ShaderProgram cache");
			ImGui::Text("Shader probe: %s", shader_program_probe.passed ? "passed" : "failed");
			for (std::string const& message : shader_program_probe.messages)
				ImGui::BulletText("%s", message.c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 2 regression: GPU RAII ownership");
			ImGui::Text("Ownership probe: %s", ownership_probe.passed ? "passed" : "failed");
			for (std::string const& message : ownership_probe.messages)
				ImGui::BulletText("%s", message.c_str());
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
