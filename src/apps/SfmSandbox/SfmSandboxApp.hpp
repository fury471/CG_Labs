#pragma once

#include "sandbox/core/FrameClock.hpp"
#include "sandbox/core/FrameProfiler.hpp"
#include "sandbox/gfx/ClearPass.hpp"
#include "sandbox/gfx/GpuProfiler.hpp"
#include "sandbox/gfx/ImagePlaneRenderer.hpp"
#include "sandbox/gfx/InstancedMarkerRenderer.hpp"
#include "sandbox/gfx/OwnershipProbe.hpp"
#include "sandbox/gfx/Renderer.hpp"
#include "sandbox/gfx/RenderTarget.hpp"
#include "sandbox/gfx/ShaderProgramProbe.hpp"
#include "sandbox/gfx/SurfaceRenderer.hpp"
#include "sandbox/scene/CameraPoseLoader.hpp"
#include "sandbox/scene/ImageImport.hpp"
#include "sandbox/scene/PointCloudLoader.hpp"
#include "sandbox/scene/SandboxProject.hpp"
#include "sandbox/scene/SurfaceImport.hpp"

#include <glm/glm.hpp>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::app
{

struct StartupOptions final
{
	std::filesystem::path project_manifest_path{};
	std::filesystem::path capture_baseline_path{};
	bool show_help{ false };
	bool validate_install{ false };
	std::vector<std::string> messages{};
};

struct StartupValidationResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] StartupOptions parse_startup_options(int argc, char** argv);
[[nodiscard]] StartupValidationResult validate_startup_assets(StartupOptions const& options);
void print_startup_help();

class SfmSandboxApp final
{
public:
	explicit SfmSandboxApp(StartupOptions startup_options);

	[[nodiscard]] bool initialise();
	void ensure_render_target_size(int framebuffer_width, int framebuffer_height);
	void render_scene(glm::mat4 const& world_to_clip, int framebuffer_width, int framebuffer_height);
	[[nodiscard]] bool capture_baseline(int framebuffer_width, int framebuffer_height);
	void draw_status_panel(sfm::core::FrameTiming const& frame_timing,
	                       sfm::core::FrameProfiler const& frame_profiler,
	                       int framebuffer_width,
	                       int framebuffer_height,
	                       float camera_aspect);

	void toggle_gui() noexcept { m_show_gui = !m_show_gui; }
	void toggle_logs() noexcept { m_show_logs = !m_show_logs; }

	[[nodiscard]] bool show_gui() const noexcept { return m_show_gui; }
	[[nodiscard]] bool show_logs() const noexcept { return m_show_logs; }
	[[nodiscard]] bool capture_requested() const noexcept { return !m_startup_options.capture_baseline_path.empty(); }
	[[nodiscard]] float ui_scale() const noexcept { return m_ui_scale; }
	[[nodiscard]] std::vector<std::string> const& capture_messages() const noexcept { return m_capture_messages; }

private:
	struct PointCloudState final
	{
		std::array<char, 512> path_buffer{};
		sfm::scene::PointCloudLoadResult load{};
		sfm::scene::PointCloud data{};
		std::string active_source{};
		std::string last_reload_status{};
		int reload_count{ 0 };
		bool using_loaded_data{ false };
	};

	struct CameraPoseState final
	{
		std::array<char, 512> path_buffer{};
		sfm::scene::CameraPoseLoadResult load{};
		sfm::scene::CameraPoseSet data{};
		std::string active_source{};
		std::string last_reload_status{};
		int reload_count{ 0 };
		int selected_pose{ 0 };
		bool using_loaded_data{ false };
	};

	struct SurfaceState final
	{
		std::array<char, 512> path_buffer{};
		sfm::scene::SurfaceImportResult load{};
		sfm::scene::SurfaceMesh data{};
		sfm::scene::SurfaceStatistics statistics{};
		std::string active_source{};
		std::string last_reload_status{};
		int reload_count{ 0 };
		bool visible{ true };
		bool using_loaded_data{ false };
		glm::vec3 colour{ 0.86f, 0.78f, 0.58f };
	};

	struct ImageState final
	{
		std::array<char, 512> path_buffer{};
		sfm::scene::ImageImportResult load{};
		sfm::scene::ImageResource data{};
		std::string active_source{};
		std::string last_reload_status{};
		int reload_count{ 0 };
		bool visible{ true };
		bool using_loaded_data{ false };
		int associated_camera_index{ 0 };
		float plane_distance{ 0.65f };
		float plane_height{ 0.70f };
	};

	void initialise_project();
	void initialise_point_cloud();
	void initialise_camera_poses();
	void initialise_surface();
	void initialise_image();
	void initialise_renderers();

	void reload_point_cloud_from_ui();
	void reload_camera_poses_from_ui();
	void reload_surface_from_ui();
	void reload_image_from_ui();
	void rebuild_image_plane_for_active_camera();
	void update_surface_colour();

	void draw_project_section();
	void draw_marker_stress_section();
	void draw_render_target_section(sfm::core::FrameProfiler const& frame_profiler);
	void draw_gpu_profiler_section();
	void draw_image_section();
	void draw_surface_section();
	void draw_point_cloud_section();
	void draw_camera_pose_section();
	void draw_renderer_section();
	void draw_regression_probe_section();

	StartupOptions m_startup_options{};
	sfm::scene::SandboxProject m_sample_project{};
	sfm::scene::SandboxProjectLoadResult m_project_load{};
	sfm::scene::SandboxProject m_active_project{};

	std::filesystem::path m_legacy_text_point_cloud_path{};

	PointCloudState m_point_cloud{};
	CameraPoseState m_camera_poses{};
	SurfaceState m_surface{};
	ImageState m_image{};

	sfm::gfx::PointCloudRenderSettings m_point_settings{};
	sfm::gfx::MarkerStressSettings m_marker_settings{};

	sfm::gfx::ClearPass m_clear_pass{ { 0.035f, 0.055f, 0.090f, 1.0f } };
	sfm::gfx::GpuFrameProfiler m_gpu_profiler{};
	sfm::gfx::RenderTarget m_offscreen_probe{};
	sfm::gfx::Renderer m_renderer{};
	sfm::gfx::SurfaceRenderer m_surface_renderer{};
	sfm::gfx::ImagePlaneRenderer m_image_plane_renderer{};
	sfm::gfx::InstancedMarkerRenderer m_marker_renderer{};

	sfm::gfx::RendererBuildResult m_renderer_result{};
	sfm::gfx::SurfaceRendererResult m_surface_renderer_result{};
	sfm::gfx::ImagePlaneRendererResult m_image_plane_result{};
	sfm::gfx::MarkerRendererResult m_marker_renderer_result{};
	sfm::gfx::OwnershipProbeResult m_ownership_probe{};
	sfm::gfx::ShaderProgramProbeResult m_shader_probe{};
	std::vector<std::string> m_capture_messages{};

	bool m_show_gui{ true };
	bool m_show_logs{ false };
	float m_ui_scale{ 1.35f };
};

} // namespace sfm::app
