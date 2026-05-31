#include "SfmSandboxPanels.hpp"

#include "SfmSandboxUi.hpp"

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>

namespace sfm::app
{
namespace
{

std::string format_vec3(glm::vec3 value)
{
	char buffer[96]{};
	std::snprintf(buffer, sizeof(buffer), "%.3f, %.3f, %.3f", value.x, value.y, value.z);
	return buffer;
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

} // namespace

bool draw_path_input(char const* label, std::array<char, 512>& buffer)
{
	ImGui::TextUnformatted(label);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::PushID(label);
	bool const changed = ImGui::InputText("##path", buffer.data(), buffer.size());
	ImGui::PopID();
	return changed;
}

bool draw_optional_visibility_checkbox(char const* label, bool& visible, bool available)
{
	if (!available)
		visible = false;
	if (!available)
		ImGui::BeginDisabled();
	bool const changed = ImGui::Checkbox(label, &visible);
	if (!available)
		ImGui::EndDisabled();
	return changed;
}

void draw_point_cloud_statistics(sfm::scene::PointCloudStatistics const& statistics)
{
	if (!ui::begin_property_table("point_cloud_statistics"))
		return;
	ui::property("Points", statistics.point_count);
	ui::property("CPU storage", std::to_string(statistics.approximate_cpu_bytes) + " bytes");
	if (!statistics.has_bounds) {
		ui::property("Bounds", "unavailable");
		ui::end_property_table();
		return;
	}
	ui::property("Bounds min", format_vec3(statistics.bounds_min));
	ui::property("Bounds max", format_vec3(statistics.bounds_max));
	ui::property("Bounds extent", format_vec3(statistics.bounds_extent));
	ui::end_property_table();
}

void draw_surface_statistics(sfm::scene::SurfaceStatistics const& statistics)
{
	if (!ui::begin_property_table("surface_statistics"))
		return;
	ui::property("Vertices", statistics.vertex_count);
	ui::property("Triangles", statistics.triangle_count);
	ui::property("CPU storage", std::to_string(statistics.approximate_cpu_bytes) + " bytes");
	if (!statistics.has_bounds) {
		ui::property("Bounds", "unavailable");
		ui::end_property_table();
		return;
	}
	ui::property("Bounds min", format_vec3(statistics.bounds_min));
	ui::property("Bounds max", format_vec3(statistics.bounds_max));
	ui::property("Bounds extent", format_vec3(statistics.bounds_extent));
	ui::end_property_table();
}

void draw_image_statistics(sfm::scene::ImageResource const& image)
{
	if (image.empty()) {
		ImGui::TextUnformatted("Associated image: unavailable");
		return;
	}
	if (!ui::begin_property_table("image_statistics"))
		return;
	ui::property("Size", std::to_string(image.width) + " x " + std::to_string(image.height));
	ui::property("Pixels", image.pixel_count());
	ui::property("Format", image.source_format);
	ui::property("Source", image.source_file);
	ui::end_property_table();
}

void draw_render_target_status(sfm::gfx::RenderTargetStatus const& status)
{
	if (!ui::begin_property_table("render_target_status"))
		return;
	ui::property("Offscreen target", status.ready, "complete", "not ready");
	ui::property("Target size", std::to_string(status.width) + " x " + std::to_string(status.height));
	ui::property("Framebuffer id", static_cast<int>(status.framebuffer_id));
	ui::property("Colour texture id", static_cast<int>(status.colour_texture_id));
	ui::property("Resize rebuilds", status.rebuild_count);
	ui::property("Status", status.message);
	ui::end_property_table();
}

void draw_frame_profiler(sfm::core::FrameProfiler const& profiler)
{
	if (!ui::begin_property_table("cpu_frame_profiler"))
		return;
	ui::property_float("CPU frame total", static_cast<float>(profiler.total_milliseconds()), "%.3f ms");
	ui::end_property_table();
	for (sfm::core::TimedPass const& pass : profiler.passes())
		ImGui::BulletText("%s: %.3f ms", pass.name.c_str(), pass.milliseconds);
}

void draw_marker_stress_statistics(sfm::gfx::MarkerStressStats const& statistics)
{
	if (!ui::begin_property_table("marker_statistics"))
		return;
	ui::property("Stress markers", statistics.marker_count);
	ui::property("Active draw calls", statistics.draw_calls);
	ui::property("Reference draw calls", statistics.reference_draw_calls);
	ui::property("Instanced draw calls", statistics.instanced_draw_calls);
	ui::property("Vertices drawn", statistics.vertices_drawn);
	ui::property("Draw-call reduction", statistics.reference_draw_calls - statistics.instanced_draw_calls);
	ui::property("Active path", statistics.using_instancing ? "instanced" : "reference");
	ui::end_property_table();
}

void draw_point_controls(sfm::gfx::PointCloudRenderSettings& settings)
{
	ImGui::TextUnformatted("Point size");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::SliderFloat("##point_size", &settings.point_size, 1.0f, 18.0f, "%.1f px");
	int colour_mode = static_cast<int>(settings.colour_mode);
	ImGui::TextUnformatted("Point colour mode");
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::BeginCombo("##point_colour_mode", colour_mode_label(settings.colour_mode))) {
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

	if (!ui::begin_property_table("camera_pose_metadata"))
		return;
	ui::property("Index", metadata.index);
	ui::property("Source id", metadata.source_id);
	ui::property("Camera id", metadata.camera_id);
	ui::property("Source line", metadata.source_line);
	ui::property("Image/name", metadata.image_name.empty() ? "<none>" : metadata.image_name);
	ui::property("Format", metadata.source_format);
	ui::property("Source file", metadata.source_file.empty() ? "<none>" : metadata.source_file);
	ui::property("Convention", metadata.source_convention);
	ui::property("Position", format_vec3(position));
	ui::property("Forward", format_vec3(forward));
	ui::property("Up", format_vec3(up));
	ui::property("Right", format_vec3(right));
	ui::end_property_table();
}

void draw_renderer_frame_statistics(sfm::gfx::RendererFrameStatistics const& statistics,
                                    sfm::gfx::SurfaceRendererStats const& surface_statistics,
                                    sfm::gfx::ImagePlaneRendererStats const& image_statistics,
                                    sfm::gfx::MarkerStressStats const& marker_statistics)
{
	if (!ui::begin_property_table("renderer_frame_statistics"))
		return;
	ui::property("Submitted items", statistics.submitted_items);
	ui::property("Draw calls", std::to_string(statistics.draw_calls) + " + surface " +
	                          std::to_string(surface_statistics.draw_calls) + " + image " +
	                          std::to_string(image_statistics.draw_calls) + " + markers " +
	                          std::to_string(marker_statistics.draw_calls));
	ui::property("Program binds", statistics.program_binds);
	ui::property("Vertex-array binds", statistics.vertex_array_binds);
	ui::property("Line vertices", statistics.line_vertices_drawn);
	ui::property("Point vertices", statistics.point_vertices_drawn);
	ui::property("Surface triangles", surface_statistics.triangles_drawn);
	ui::property("Image triangles", image_statistics.triangles_drawn);
	ui::property("Marker vertices", marker_statistics.vertices_drawn);
	ui::end_property_table();
}

} // namespace sfm::app
