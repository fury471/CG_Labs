#include "SurfaceImport.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>
#include <string_view>
#include <utility>

namespace sfm::scene
{
namespace
{

void add_message(SurfaceImportResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

std::string trim(std::string_view text)
{
	auto const first = text.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos)
		return {};
	auto const last = text.find_last_not_of(" \t\r\n");
	return std::string{ text.substr(first, last - first + 1u) };
}

bool parse_float(std::string const& token, float& value)
{
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof() && std::isfinite(value);
}

bool parse_index(std::string token, std::size_t vertex_count, std::size_t& zero_based_index)
{
	auto const slash = token.find('/');
	if (slash != std::string::npos)
		token.erase(slash);
	if (token.empty())
		return false;

	int one_based = 0;
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> one_based;
	if (stream.fail() || !stream.eof() || one_based <= 0)
		return false;

	std::size_t const candidate = static_cast<std::size_t>(one_based - 1);
	if (candidate >= vertex_count)
		return false;
	zero_based_index = candidate;
	return true;
}

std::string lowercase_ascii(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

} // namespace

SurfaceStatistics statistics_for(SurfaceMesh const& mesh) noexcept
{
	SurfaceStatistics result{};
	result.vertex_count = mesh.positions.size();
	result.triangle_count = mesh.triangles.size();
	result.approximate_cpu_bytes = mesh.positions.size() * sizeof(glm::vec3) + mesh.triangles.size() * sizeof(SurfaceTriangle);
	if (mesh.positions.empty())
		return result;

	glm::vec3 bounds_min = mesh.positions.front();
	glm::vec3 bounds_max = mesh.positions.front();
	for (glm::vec3 const& position : mesh.positions) {
		bounds_min = glm::min(bounds_min, position);
		bounds_max = glm::max(bounds_max, position);
	}

	result.has_bounds = true;
	result.bounds_min = bounds_min;
	result.bounds_max = bounds_max;
	result.bounds_extent = bounds_max - bounds_min;
	return result;
}

SurfaceImportResult import_surface_from_obj(std::filesystem::path const& path)
{
	SurfaceImportResult result{};
	result.source_format = "M16 ASCII OBJ subset";

	std::ifstream file{ path };
	if (!file) {
		add_message(result, "Surface import failed: could not open '" + path.string() + "'");
		return result;
	}

	std::string line;
	std::size_t line_number = 0u;
	while (std::getline(file, line)) {
		++line_number;

		auto const comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);
		line = trim(line);
		if (line.empty())
			continue;

		std::istringstream stream{ line };
		std::string tag;
		stream >> tag;
		if (tag == "v") {
			std::string sx, sy, sz, extra;
			stream >> sx >> sy >> sz >> extra;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			if (!extra.empty() || !parse_float(sx, x) || !parse_float(sy, y) || !parse_float(sz, z)) {
				++result.skipped_lines;
				add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed vertex");
				continue;
			}
			result.mesh.positions.emplace_back(x, y, z);
			continue;
		}

		if (tag == "f") {
			std::string sa, sb, sc, extra;
			stream >> sa >> sb >> sc >> extra;
			std::size_t a = 0u;
			std::size_t b = 0u;
			std::size_t c = 0u;
			if (!extra.empty() || !parse_index(sa, result.mesh.positions.size(), a) ||
			    !parse_index(sb, result.mesh.positions.size(), b) ||
			    !parse_index(sc, result.mesh.positions.size(), c) ||
			    a == b || b == c || a == c) {
				++result.skipped_lines;
				add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed triangle face");
				continue;
			}
			result.mesh.triangles.push_back(SurfaceTriangle{ a, b, c });
			continue;
		}

		if (tag == "vn" || tag == "vt" || tag == "o" || tag == "g" || tag == "s" || tag == "usemtl" || tag == "mtllib")
			continue;

		++result.skipped_lines;
		add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": unsupported statement '" + tag + "'");
	}

	if (result.mesh.positions.empty()) {
		add_message(result, "Surface import failed: OBJ contained no valid vertices");
		return result;
	}
	if (result.mesh.triangles.empty()) {
		add_message(result, "Surface import failed: OBJ contained no valid triangles");
		return result;
	}

	result.succeeded = true;
	add_message(result, "Loaded surface '" + path.string() + "'");
	add_message(result, "Format: " + result.source_format);
	add_message(result, "Vertices: " + std::to_string(result.mesh.vertex_count()));
	add_message(result, "Triangles: " + std::to_string(result.mesh.triangle_count()));
	return result;
}

SurfaceImportResult import_surface(std::filesystem::path const& path)
{
	if (lowercase_ascii(path.extension().string()) == ".obj")
		return import_surface_from_obj(path);

	SurfaceImportResult result{};
	add_message(result, "Surface import failed: unsupported file extension '" + path.extension().string() + "'");
	return result;
}

} // namespace sfm::scene
