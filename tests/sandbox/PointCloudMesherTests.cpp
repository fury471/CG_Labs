#include "sandbox/scene/PointCloudMesher.hpp"
#include "sandbox/scene/SurfaceExport.hpp"
#include "sandbox/scene/SurfaceImport.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

void require(bool condition, char const* message)
{
	if (!condition)
		throw std::runtime_error{ message };
}

sfm::scene::PointCloud make_square_cloud()
{
	std::vector<sfm::scene::PointSample> points{
		{ { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { 1.0f, 0.1f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.2f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { 1.0f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
	};
	return sfm::scene::PointCloud{ std::move(points) };
}

} // namespace

int main()
{
	try {
		sfm::scene::PointToMeshBuildOptions options{};
		options.projection = sfm::scene::PointMeshProjectionPlane::XZ;
		options.maximum_edge_length = 0.0f;

		sfm::scene::PointToMeshBuildResult const square = sfm::scene::build_surface_mesh_from_point_cloud(make_square_cloud(), options);
		require(square.succeeded, "square cloud should triangulate");
		require(square.mesh.vertex_count() == 4u, "square triangulation should keep four vertices");
		require(square.mesh.triangle_count() == 2u, "square triangulation should produce two triangles");

		std::vector<sfm::scene::PointSample> duplicate_points{
			{ { 0.0f, 0.0f, 0.0f }, {} },
			{ { 1.0f, 0.0f, 0.0f }, {} },
			{ { 0.0f, 0.0f, 1.0f }, {} },
			{ { 1.0f, 0.0f, 1.0f }, {} },
			{ { 1.0f, 1.0f, 1.0f }, {} },
		};
		sfm::scene::PointToMeshBuildResult const welded = sfm::scene::build_surface_mesh_from_point_cloud(sfm::scene::PointCloud{ std::move(duplicate_points) }, options);
		require(welded.succeeded, "duplicate projected point should be welded and still triangulate");
		require(welded.unique_point_count == 4u, "duplicate projected point should reduce unique count");
		require(welded.duplicate_point_count == 1u, "duplicate projected point should be counted");

		sfm::scene::PointToMeshBuildOptions low_limit_options = options;
		low_limit_options.maximum_projected_points = 3u;
		sfm::scene::PointToMeshBuildResult const limited = sfm::scene::build_surface_mesh_from_point_cloud(make_square_cloud(), low_limit_options);
		require(!limited.succeeded, "point limit should reject oversized projected input");
		require(limited.unique_point_count == 4u, "limited build should report unique projected point count");

		std::vector<sfm::scene::PointSample> collinear_points{
			{ { 0.0f, 0.0f, 0.0f }, {} },
			{ { 1.0f, 0.0f, 0.0f }, {} },
			{ { 2.0f, 0.0f, 0.0f }, {} },
		};
		sfm::scene::PointToMeshBuildResult const collinear = sfm::scene::build_surface_mesh_from_point_cloud(sfm::scene::PointCloud{ std::move(collinear_points) }, options);
		require(!collinear.succeeded, "collinear projected points should not produce a mesh");

		std::filesystem::path const export_path = std::filesystem::temp_directory_path() / "sfm_point_mesher_roundtrip.obj";
		sfm::scene::SurfaceExportResult const export_result = sfm::scene::export_surface_mesh_as_obj(square.mesh, export_path, "point mesher test\nprojection: XZ");
		require(export_result.succeeded, "generated mesh should export as OBJ");
		std::ifstream exported_file{ export_path };
		std::string exported_text{ std::istreambuf_iterator<char>{ exported_file }, std::istreambuf_iterator<char>{} };
		exported_file.close();
		require(exported_text.find("# point mesher test") != std::string::npos, "exported OBJ should include first comment line");
		require(exported_text.find("# projection: XZ") != std::string::npos, "exported OBJ should prefix multi-line comments");
		sfm::scene::SurfaceImportResult const import_result = sfm::scene::import_surface(export_path);
		std::filesystem::remove(export_path);
		require(import_result.succeeded, "exported OBJ should be readable by surface importer");
		require(import_result.mesh.vertex_count() == square.mesh.vertex_count(), "round-trip vertex count mismatch");
		require(import_result.mesh.triangle_count() == square.mesh.triangle_count(), "round-trip triangle count mismatch");

		return 0;
	} catch (std::exception const& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}
}
