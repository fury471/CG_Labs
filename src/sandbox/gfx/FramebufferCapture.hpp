#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sfm::gfx
{

struct FramebufferCaptureResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] FramebufferCaptureResult capture_back_buffer_to_ppm(std::filesystem::path const& output_path,
                                                                  int framebuffer_width,
                                                                  int framebuffer_height);

} // namespace sfm::gfx
