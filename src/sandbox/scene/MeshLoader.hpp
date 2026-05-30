#pragma once

#include "SurfaceImport.hpp"

namespace sfm::scene
{

using MeshLoadResult = SurfaceImportResult;

[[nodiscard]] inline MeshLoadResult load_mesh_from_obj_file(std::filesystem::path const& path)
{
	return import_surface_from_obj(path);
}

[[nodiscard]] inline MeshLoadResult load_mesh_from_file(std::filesystem::path const& path)
{
	return import_surface(path);
}

} // namespace sfm::scene
