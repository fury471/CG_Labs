#include "SurfaceExport.hpp"

#include <fstream>
#include <locale>
#include <string_view>
#include <system_error>
#include <utility>

namespace sfm::scene
{
namespace
{

void add_message(SurfaceExportResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

bool mesh_indices_are_valid(SurfaceMesh const& mesh) noexcept
{
	for (SurfaceTriangle const& triangle : mesh.triangles) {
		if (triangle.a >= mesh.positions.size() || triangle.b >= mesh.positions.size() || triangle.c >= mesh.positions.size())
			return false;
		if (triangle.a == triangle.b || triangle.b == triangle.c || triangle.a == triangle.c)
			return false;
	}
	return true;
}

void write_obj_comment(std::ofstream& file, std::string_view comment)
{
	std::size_t start = 0u;
	while (start <= comment.size()) {
		std::size_t const end = comment.find('\n', start);
		std::string_view line = end == std::string_view::npos ? comment.substr(start) : comment.substr(start, end - start);
		if (!line.empty() && line.back() == '\r')
			line.remove_suffix(1u);
		file << "# " << line << '\n';
		if (end == std::string_view::npos)
			break;
		start = end + 1u;
	}
}

} // namespace

SurfaceExportResult export_surface_mesh_as_obj(SurfaceMesh const& mesh,
                                               std::filesystem::path const& path,
                                               std::string_view comment)
{
	SurfaceExportResult result{};
	if (mesh.empty()) {
		add_message(result, "Surface export failed: mesh is empty");
		return result;
	}
	if (!mesh_indices_are_valid(mesh)) {
		add_message(result, "Surface export failed: mesh contains invalid triangle indices");
		return result;
	}

	if (path.has_parent_path()) {
		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);
		if (error) {
			add_message(result, "Surface export failed: could not create output directory '" + path.parent_path().string() + "'");
			return result;
		}
	}

	std::ofstream file{ path };
	if (!file) {
		add_message(result, "Surface export failed: could not open '" + path.string() + "'");
		return result;
	}
	file.imbue(std::locale::classic());
	file << "# SfM Visualization Sandbox surface mesh\n";
	if (!comment.empty())
		write_obj_comment(file, comment);
	file << "# vertices " << mesh.vertex_count() << '\n';
	file << "# triangles " << mesh.triangle_count() << '\n';
	for (glm::vec3 const& position : mesh.positions)
		file << "v " << position.x << ' ' << position.y << ' ' << position.z << '\n';
	for (SurfaceTriangle const& triangle : mesh.triangles)
		file << "f " << (triangle.a + 1u) << ' ' << (triangle.b + 1u) << ' ' << (triangle.c + 1u) << '\n';

	if (!file) {
		add_message(result, "Surface export failed: could not write '" + path.string() + "'");
		return result;
	}

	result.succeeded = true;
	add_message(result, "Surface export wrote OBJ '" + path.string() + "'");
	add_message(result, "Vertices: " + std::to_string(mesh.vertex_count()));
	add_message(result, "Triangles: " + std::to_string(mesh.triangle_count()));
	return result;
}

} // namespace sfm::scene
