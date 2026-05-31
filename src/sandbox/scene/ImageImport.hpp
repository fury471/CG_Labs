#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

struct ImageResource final
{
	int width{ 0 };
	int height{ 0 };
	std::vector<glm::vec3> pixels{};
	std::string source_file{};
	std::string source_format{};

	[[nodiscard]] bool empty() const noexcept { return width <= 0 || height <= 0 || pixels.empty(); }
	[[nodiscard]] std::size_t pixel_count() const noexcept { return pixels.size(); }
};

struct ImageImportResult final
{
	ImageResource image{};
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] ImageImportResult import_ppm_p3_image(std::filesystem::path const& path);
[[nodiscard]] ImageImportResult import_image(std::filesystem::path const& path);
[[nodiscard]] glm::vec3 sample_image_nearest(ImageResource const& image, int x, int y) noexcept;

} // namespace sfm::scene
