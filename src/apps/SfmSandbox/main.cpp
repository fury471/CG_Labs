#include "config.hpp"
#include "core/Bonobo.h"
#include "sandbox/core/FrameClock.hpp"
#include "sandbox/gfx/ClearPass.hpp"

#include <imgui.h>

#include <glm/gtc/constants.hpp>

#include <chrono>
#include <clocale>
#include <cstdlib>

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(),
	                  static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                  0.01f,
	                  1000.0f);
	camera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 5.0f));
	camera.mMouseSensitivity = glm::vec2(0.003f);
	camera.mMovementSpeed = glm::vec3(3.0f);

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

		window_manager.NewImGuiFrame();
		clear_pass.render(framebuffer_width, framebuffer_height);

		if (ImGui::Begin("Sandbox status")) {
			ImGui::TextUnformatted("SfM Visualization Sandbox");
			ImGui::Separator();
			ImGui::Text("Frame: %llu", static_cast<unsigned long long>(frame_timing.frame_index));
			ImGui::Text("CPU frame: %.3f ms", frame_timing.delta_milliseconds);
			ImGui::Text("FPS: %.1f", frame_timing.frames_per_second);
			ImGui::Text("Framebuffer: %d x %d", framebuffer_width, framebuffer_height);
			ImGui::Separator();
			ImGui::TextUnformatted("Milestone 1: application shell");
			ImGui::BulletText("Platform window and ImGui connected");
			ImGui::BulletText("Rendering boundary: sandbox/gfx/ClearPass");
			ImGui::BulletText("Scene and SfM visualization arrive later");
			ImGui::Separator();
			ImGui::TextUnformatted("Controls: F2 UI, F3 logs, F11 fullscreen, Esc quit");
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
