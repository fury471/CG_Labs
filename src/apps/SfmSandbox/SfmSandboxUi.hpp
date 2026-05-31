#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace sfm::app::ui
{

void set_next_inspector_window(int framebuffer_width, int framebuffer_height, float ui_scale);

[[nodiscard]] bool begin_property_table(char const* id);
void end_property_table();
void property(char const* label, char const* value);
void property(char const* label, std::string const& value);
void property(char const* label, bool value, char const* true_text = "on", char const* false_text = "off");
void property(char const* label, int value);
void property(char const* label, std::size_t value);
void property_float(char const* label, float value, char const* format = "%.3f");

void metric(char const* label, std::string const& value);
void draw_messages(char const* label, std::vector<std::string> const& messages, bool default_open = false);

} // namespace sfm::app::ui
