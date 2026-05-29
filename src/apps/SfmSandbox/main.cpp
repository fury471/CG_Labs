#include "config.hpp"
#include "core/Bonobo.h"
#include "sandbox/core/FrameClock.hpp"
#include "sandbox/gfx/ClearPass.hpp"
#include "sandbox/gfx/OwnershipProbe.hpp"
#include "sandbox/gfx/Renderer.hpp"
#include "sandbox/gfx/ShaderProgramProbe.hpp"

#include <imgui.h>

#include <glm/gtc/constants.hpp>

#include <chrono>
#include <clocale>
#include <cstdlib>
#include <string>

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

	// Milestone 5 turns the renderer into a small 3D viewport: the app provides
	// the camera transform and the renderer owns world-space grid/axes drawing.
	sfm::gfx::Renderer renderer;
	sfm::gfx::RendererBuildResult const renderer_build = renderer.initialise();

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
			ImGui::TextUnformatted("Milestone 5: Camera transform path and grid/axes primitive");
			ImGui::Text("Renderer: %s", renderer.ready() ? "ready" : "failed");
			ImGui::Text("Line vertices: %d", renderer.line_vertex_count());
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
