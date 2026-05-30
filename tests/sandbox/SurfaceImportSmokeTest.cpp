#include "sandbox/scene/SurfaceImport.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
	std::filesystem::path const path = std::filesystem::temp_directory_path() / "sfm_surface_smoke.obj";
	{
		std::ofstream file{ path };
		file << "v 0 0 0\n";
		file << "v 1 0 0\n";
		file << "v 0 1 0\n";
		file << "f 1 2 3\n";
	}

	sfm::scene::SurfaceImportResult const result = sfm::scene::import_surface(path);
	std::filesystem::remove(path);
	if (!result.succeeded) {
		std::cerr << "surface import failed\n";
		return 1;
	}
	if (result.mesh.vertex_count() != 3u || result.mesh.triangle_count() != 1u) {
		std::cerr << "unexpected surface counts\n";
		return 1;
	}
	return 0;
}
