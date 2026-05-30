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
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return text;
}

} // namespace

SurfaceImportResult import_surface_from_obj(std::filesystem::path const& path)
{
	SurfaceImportResult result{};
	result.source_format = "M16 ASCII OBJ subset";

	std::ifstream file{ path };
	if (!file) {
		add_message(result, "Surface import failed: could not open '" + path.string() + "'");
		return result;
	}

	std::vector<glm::vec3> positions;
	std::vector<MeshTriangle> triangles;
	std::string line;
	std::size_t line_number = 0u;
	while (std::getline(file, line)) {
		++line_number;
		auto const comment_begin = line.find('#');
		if (comment_begin != std::string::npos)
			line.erase(comment_begin);
		line = trim(line);
		if (line.empty())
			continue;

		std::istringstream stream{ line };
		std::string tag;
		stream >> tag;
		if (tag == "v") {
			std::string sx;
			std::string sy;
			std::string sz;
			std::string extra;
			stream >> sx >> sy >> sz >> extra;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			if (!extra.empty() || !parse_float(sx, x) || !parse_float(sy, y) || !parse_float(sz, z)) {
				++result.skipped_lines;
				add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed vertex");
				continue;
			}
			positions.emplace_back(x, y, z);
			continue;
		}

		if (tag == "f") {
			std::string sa;
			std::string sb;
			std::string sc;
			std::string extra;
			stream >> sa >> sb >> sc >> extra;
			std::size_t a = 0u;
			std::size_t b = 0u;
			std::size_t c = 0u;
			if (!extra.empty() || !parse_index(sa, positions.size(), a) || !parse_index(sb, positions.size(), b) || !parse_index(sc, positions.size(), c)) {
				++result.skipped_lines;
				add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed or unsupported triangle face");
				continue;
			}
			if (a == b || b == c || a == c) {
				++result.skipped_lines;
				add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": degenerate triangle face");
				continue;
			}
			triangles.push_back(MeshTriangle{ a, b, c });
			continue;
		}

		if (tag == "vn" || tag == "vt" || tag == "o" || tag == "g" || tag == "s" || tag == "usemtl" || tag == "mtllib")
			continue;

		++result.skipped_lines;
		add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": unsupported statement '" + tag + "'");
	}

	if (positions.empty()) {
		add_message(result, "Surface import failed: OBJ contained no valid vertices");
		return result;
	}
	if (triangles.empty()) {
		add_message(result, "Surface import failed: OBJ contained no valid triangles");
		return result;
	}

	result.succeeded = true;
	result.mesh = Mesh{ std::move(positions), std::move(triangles) };
	add_message(result, "Loaded surface '" + path.string() + "'");
	add_message(result, "Format: " + result.source_format);
	add_message(result, "Vertices: " + std::to_string(result.mesh.vertex_count()));
	add_message(result, "Triangles: " + std::to_string(result.mesh.triangle_count()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped unsupported or malformed OBJ lines: " + std::to_string(result.skipped_lines));
	return result;
}

SurfaceImportResult import_surface(std::filesystem::path const& path)
{
	std::string const extension = lowercase_ascii(path.extension().string());
	if (extension == ".obj")
		return import_surface_from_obj(path);

	SurfaceImportResult result{};
	add_message(result, "Surface import failed: unsupported file extension '" + extension + "'");
	return result;
}

} // namespace sfm::scene
