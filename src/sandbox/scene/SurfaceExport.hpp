#pragma once

#include "SurfaceImport.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace sfm::scene
{

struct SurfaceExportResult final
{
	bool succeeded{ false };
	std::vector<std::string> messages{};
};

[[nodiscard]] SurfaceExportResult export_surface_mesh_as_obj(SurfaceMesh const& mesh,
                                                             std::filesystem::path const& path,
                                                             std::string_view comment = {});

} // namespace sfm::scene
