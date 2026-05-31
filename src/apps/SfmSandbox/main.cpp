#include "SfmSandboxApp.hpp"
#include "config.hpp"
#include "core/Bonobo.h"
#include "sandbox/core/FrameClock.hpp"
#include "sandbox/core/FrameProfiler.hpp"

#include <imgui.h>

#include <glm/gtc/constants.hpp>

#include <chrono>
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

int main(int argc, char** argv)
{
	std::setlocale(LC_ALL, "");

	sfm::app::StartupOptions startup_options = sfm::app::parse_startup_options(argc, argv);
	if (startup_options.show_help) {
		sfm::app::print_startup_help();
		return EXIT_SUCCESS;
	}
	if (startup_options.validate_install) {
		sfm::app::StartupValidationResult const validation = sfm::app::validate_startup_assets(startup_options);
		std::ostream& stream = validation.succeeded ? std::cout : std::cerr;
		for (std::string const& message : validation.messages)
			stream << message << '\n';
		return validation.succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
	}

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

	int exit_code = EXIT_SUCCESS;
	{
		sfm::app::SfmSandboxApp app{ std::move(startup_options) };
		[[maybe_unused]] bool const app_ready = app.initialise();
		sfm::core::FrameClock frame_clock;
		sfm::core::FrameProfiler frame_profiler;
		bool capture_completed = false;

		while (!glfwWindowShouldClose(window)) {
			frame_profiler.begin_frame();
			sfm::core::FrameTiming const frame_timing = frame_clock.tick();
			auto const delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(frame_timing.delta_seconds));

			{
				auto scope = frame_profiler.scope("Input and camera update");
				glfwPollEvents();
				ImGuiIO& io = ImGui::GetIO();
				io.FontGlobalScale = app.ui_scale();
				input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
				input_handler.Advance();
				camera.Update(delta_time, input_handler);

				if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
					app.toggle_gui();
				if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
					app.toggle_logs();
				if (input_handler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
					window_manager.ToggleFullscreenStatusForWindow(window);
			}

			int framebuffer_width = 0;
			int framebuffer_height = 0;
			{
				auto scope = frame_profiler.scope("Resize and render-target validation");
				glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
				if (framebuffer_width > 0 && framebuffer_height > 0) {
					camera.SetAspect(static_cast<float>(framebuffer_width) / static_cast<float>(framebuffer_height));
					app.ensure_render_target_size(framebuffer_width, framebuffer_height);
				}
			}

			{
				auto scope = frame_profiler.scope("ImGui frame setup");
				window_manager.NewImGuiFrame();
			}
			{
				auto scope = frame_profiler.scope("Scene clear and draw");
				app.render_scene(camera.GetWorldToClipMatrix(), framebuffer_width, framebuffer_height);
				if (app.capture_requested() && !capture_completed) {
					bool const captured = app.capture_baseline(framebuffer_width, framebuffer_height);
					std::ostream& stream = captured ? std::cout : std::cerr;
					for (std::string const& message : app.capture_messages())
						stream << message << '\n';
					exit_code = captured ? EXIT_SUCCESS : EXIT_FAILURE;
					capture_completed = true;
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}
			}
			{
				auto scope = frame_profiler.scope("Status panel UI");
				app.draw_status_panel(frame_timing, frame_profiler, framebuffer_width, framebuffer_height, camera.GetAspect());
			}
			{
				auto scope = frame_profiler.scope("Log and ImGui submit");
				if (app.show_logs())
					Log::View::Render();
				window_manager.RenderImGuiFrame(app.show_gui());
			}
			{
				auto scope = frame_profiler.scope("Swap buffers");
				glfwSwapBuffers(window);
			}
			frame_profiler.end_frame();
		}
	}

	window_manager.DestroyWindow(window);
	return exit_code;
}
