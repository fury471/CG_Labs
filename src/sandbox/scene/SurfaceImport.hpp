#pragma once

#include "Mesh.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

struct SurfaceImportResult final
{
	Mesh mesh{};
	bool succeeded{ false };
	std::size_t skipped_lines{ 0u };
	std::vector<std::string> messages{};
	std::string source_format{};
};

[[nodiscard]] SurfaceImportResult import_surface_from_obj(std::filesystem::path const& path);
[[nodiscard]] SurfaceImportResult import_surface(std::filesystem::path const& path);

} // namespace sfm::scene
