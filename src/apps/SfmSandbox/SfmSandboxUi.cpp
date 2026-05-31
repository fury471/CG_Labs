#include "SfmSandboxUi.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdio>

namespace sfm::app::ui
{
namespace
{

void begin_property_row(char const* label)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::TextDisabled("%s", label);
	ImGui::TableNextColumn();
}

} // namespace

void set_next_inspector_window(int framebuffer_width, int framebuffer_height, float ui_scale)
{
	float const scale = std::max(ui_scale, 1.0f);
	float const width = std::clamp(static_cast<float>(framebuffer_width) * 0.34f, 460.0f * scale, 660.0f * scale);
	float const height = std::clamp(static_cast<float>(framebuffer_height) - 48.0f * scale, 360.0f * scale, 820.0f * scale);
	ImGui::SetNextWindowPos(ImVec2{ 16.0f * scale, 16.0f * scale }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2{ width, height }, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.94f);
}

bool begin_property_table(char const* id)
{
	ImGuiTableFlags const flags = ImGuiTableFlags_BordersInnerV |
	                             ImGuiTableFlags_RowBg |
	                             ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable(id, 2, flags))
		return false;
	ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 150.0f);
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
	return true;
}

void end_property_table()
{
	ImGui::EndTable();
}

void property(char const* label, char const* value)
{
	begin_property_row(label);
	ImGui::TextWrapped("%s", value);
}

void property(char const* label, std::string const& value)
{
	property(label, value.c_str());
}

void property(char const* label, bool value, char const* true_text, char const* false_text)
{
	property(label, value ? true_text : false_text);
}

void property(char const* label, int value)
{
	begin_property_row(label);
	ImGui::Text("%d", value);
}

void property(char const* label, std::size_t value)
{
	begin_property_row(label);
	ImGui::Text("%zu", value);
}

void property_float(char const* label, float value, char const* format)
{
	begin_property_row(label);
	char buffer[64]{};
	std::snprintf(buffer, sizeof(buffer), format, value);
	ImGui::TextUnformatted(buffer);
}

void metric(char const* label, std::string const& value)
{
	ImGui::TextUnformatted(value.c_str());
	ImGui::TextDisabled("%s", label);
}

void draw_messages(char const* label, std::vector<std::string> const& messages, bool default_open)
{
	if (messages.empty())
		return;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
	if (default_open)
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
	if (!ImGui::CollapsingHeader(label, flags)) {
		ImGui::SameLine();
		ImGui::TextDisabled("(%zu)", messages.size());
		return;
	}
	for (std::string const& message : messages)
		ImGui::BulletText("%s", message.c_str());
}

} // namespace sfm::app::ui
