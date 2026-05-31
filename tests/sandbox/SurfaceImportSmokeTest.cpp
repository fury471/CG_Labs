#include "sandbox/scene/SurfaceImport.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

void require(bool condition, char const* message)
{
	if (!condition)
		throw std::runtime_error{ message };
}

std::filesystem::path write_file(std::string const& filename, std::string const& contents)
{
	std::filesystem::path const path = std::filesystem::temp_directory_path() / filename;
	{
		std::ofstream file{ path };
		file << contents;
	}
	return path;
}

} // namespace

int main()
{
	try {
	std::filesystem::path const path = write_file("sfm_surface_smoke.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
	sfm::scene::SurfaceImportResult const result = sfm::scene::import_surface(path);
	std::filesystem::remove(path);
	require(result.succeeded, "surface import should load the supported OBJ subset");
	require(result.mesh.vertex_count() == 3u, "unexpected surface vertex count");
	require(result.mesh.triangle_count() == 1u, "unexpected surface triangle count");

	sfm::scene::SurfaceStatistics const statistics = sfm::scene::statistics_for(result.mesh);
	require(statistics.has_bounds, "surface statistics should include bounds");
	require(statistics.vertex_count == 3u, "surface statistics vertex count mismatch");
	require(statistics.triangle_count == 1u, "surface statistics triangle count mismatch");

	std::filesystem::path const malformed = write_file("sfm_surface_bad.obj", "v 0 0 0\nv 1 0 0\nf 1 2 3\n");
	sfm::scene::SurfaceImportResult const malformed_result = sfm::scene::import_surface(malformed);
	std::filesystem::remove(malformed);
	require(!malformed_result.succeeded, "surface import should reject faces with missing vertices");
	require(malformed_result.skipped_lines == 1u, "malformed surface should count skipped face line");

	std::filesystem::path const unsupported = write_file("sfm_surface_bad.ply", "ply\n");
	sfm::scene::SurfaceImportResult const unsupported_result = sfm::scene::import_surface(unsupported);
	std::filesystem::remove(unsupported);
	require(!unsupported_result.succeeded, "surface import should reject unsupported extensions");
	return 0;
	} catch (std::exception const& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}
}
