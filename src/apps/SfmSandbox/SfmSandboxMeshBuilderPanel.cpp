#include "SfmSandboxMeshBuilderPanel.hpp"

#include "SfmSandboxUi.hpp"

#include <imgui.h>

#include <algorithm>
#include <limits>
#include <string>

namespace sfm::app
{
namespace
{

bool draw_projection_combo(char const* label, sfm::scene::PointMeshProjectionPlane& projection)
{
	bool changed = false;
	int selected = static_cast<int>(projection);
	if (ImGui::BeginCombo(label, sfm::scene::projection_plane_label(projection))) {
		for (int value = 0; value <= 3; ++value) {
			auto const candidate = static_cast<sfm::scene::PointMeshProjectionPlane>(value);
			bool const is_selected = selected == value;
			if (ImGui::Selectable(sfm::scene::projection_plane_label(candidate), is_selected)) {
				selected = value;
				changed = true;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	projection = static_cast<sfm::scene::PointMeshProjectionPlane>(selected);
	return changed;
}

void begin_control_row(char const* label)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::TextDisabled("%s", label);
	ImGui::TableNextColumn();
	ImGui::SetNextItemWidth(-FLT_MIN);
}

} // namespace

MeshBuilderPanelActions draw_mesh_builder_panel(SfmSandboxMeshWorkflow& workflow)
{
	MeshBuilderPanelActions actions{};

	sfm::scene::PointToMeshBuildOptions& mesh_options = workflow.options();
	if (ImGui::BeginTable("mesh_builder_settings", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed, 160.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		begin_control_row("Projection");
		draw_projection_combo("##mesh_projection", mesh_options.projection);

		begin_control_row("Weld tolerance");
		ImGui::InputFloat("##mesh_weld", &mesh_options.weld_epsilon, 0.0001f, 0.001f, "%.6f");

		begin_control_row("Minimum area");
		ImGui::InputFloat("##mesh_min_area", &mesh_options.minimum_triangle_area, 0.0001f, 0.001f, "%.6f");

		begin_control_row("Maximum edge");
		ImGui::InputFloat("##mesh_max_edge", &mesh_options.maximum_edge_length, 0.05f, 0.25f, "%.3f");

		int max_points = static_cast<int>(std::min<std::size_t>(mesh_options.maximum_projected_points,
		                                                        static_cast<std::size_t>(std::numeric_limits<int>::max())));
		begin_control_row("Max points");
		if (ImGui::InputInt("##mesh_max_points", &max_points, 100, 1000))
			mesh_options.maximum_projected_points = static_cast<std::size_t>(std::max(max_points, 3));

		ImGui::EndTable();
	}

	if (workflow.build_in_progress())
		ImGui::TextUnformatted("Build is running...");
	bool const can_start_build = !workflow.build_in_progress();
	if (!can_start_build)
		ImGui::BeginDisabled();
	if (ImGui::Button("Build mesh from point cloud"))
		actions.build_requested = true;
	if (!can_start_build)
		ImGui::EndDisabled();

	sfm::scene::PointToMeshBuildResult const& build = workflow.build();
	if (ui::begin_property_table("mesh_builder_status")) {
		ui::property("Status", workflow.last_status());
		ui::property("Successful builds", workflow.build_count());
		ui::property("Projection", sfm::scene::projection_plane_label(build.resolved_projection));
		ui::property("Input points", build.input_point_count);
		ui::property("Unique points", build.unique_point_count);
		ui::property("Generated vertices", build.mesh.vertex_count());
		ui::property("Generated triangles", build.mesh.triangle_count());
		ui::property("Welded duplicates", build.duplicate_point_count);
		ui::property("Invalid skipped", build.invalid_point_count);
		ui::property("Rejected triangles", build.rejected_triangle_count);
		ui::end_property_table();
	}
	ImGui::TextUnformatted("OBJ export path");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputText("##mesh_export_path", workflow.export_path_buffer().data(), workflow.export_path_buffer().size());
	if (ImGui::Button("Export active surface OBJ"))
		actions.export_requested = true;
	if (ui::begin_property_table("mesh_export_status")) {
		ui::property("Successful exports", workflow.export_count());
		ui::end_property_table();
	}
	ui::draw_messages("Build messages", build.messages);
	ui::draw_messages("Export messages", workflow.export_result().messages);

	return actions;
}

} // namespace sfm::app
