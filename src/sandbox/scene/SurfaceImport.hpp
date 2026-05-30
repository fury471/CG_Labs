#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <locale>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sfm::scene
{

struct SurfaceTriangle final { std::size_t a{0u}; std::size_t b{0u}; std::size_t c{0u}; };

struct SurfaceStatistics final
{
	std::size_t vertex_count{0u};
	std::size_t triangle_count{0u};
	std::size_t approximate_cpu_bytes{0u};
	bool has_bounds{false};
	glm::vec3 bounds_min{};
	glm::vec3 bounds_max{};
	glm::vec3 bounds_extent{};
};

struct SurfaceMesh final
{
	std::vector<glm::vec3> positions{};
	std::vector<SurfaceTriangle> triangles{};
	[[nodiscard]] bool empty() const noexcept { return positions.empty() || triangles.empty(); }
	[[nodiscard]] std::size_t vertex_count() const noexcept { return positions.size(); }
	[[nodiscard]] std::size_t triangle_count() const noexcept { return triangles.size(); }
	[[nodiscard]] std::span<glm::vec3 const> vertices() const noexcept { return positions; }
	[[nodiscard]] std::span<SurfaceTriangle const> faces() const noexcept { return triangles; }
};

[[nodiscard]] inline SurfaceStatistics statistics_for(SurfaceMesh const& mesh) noexcept
{
	SurfaceStatistics result{};
	result.vertex_count = mesh.positions.size();
	result.triangle_count = mesh.triangles.size();
	result.approximate_cpu_bytes = mesh.positions.size() * sizeof(glm::vec3) + mesh.triangles.size() * sizeof(SurfaceTriangle);
	if (mesh.positions.empty()) return result;
	glm::vec3 mn = mesh.positions.front();
	glm::vec3 mx = mesh.positions.front();
	for (glm::vec3 const& p : mesh.positions) { mn = glm::min(mn, p); mx = glm::max(mx, p); }
	result.has_bounds = true;
	result.bounds_min = mn;
	result.bounds_max = mx;
	result.bounds_extent = mx - mn;
	return result;
}

struct SurfaceImportResult final
{
	SurfaceMesh mesh{};
	bool succeeded{false};
	std::size_t skipped_lines{0u};
	std::vector<std::string> messages{};
	std::string source_format{};
};

namespace surface_import_detail
{
inline void add_message(SurfaceImportResult& r, std::string m) { r.messages.emplace_back(std::move(m)); }
inline std::string trim(std::string_view t)
{
	auto const first = t.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos) return {};
	auto const last = t.find_last_not_of(" \t\r\n");
	return std::string{ t.substr(first, last - first + 1u) };
}
inline bool parse_float(std::string const& token, float& value)
{
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof() && std::isfinite(value);
}
inline bool parse_index(std::string token, std::size_t vertex_count, std::size_t& zero_based_index)
{
	auto const slash = token.find('/');
	if (slash != std::string::npos) token.erase(slash);
	if (token.empty()) return false;
	int one_based = 0;
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> one_based;
	if (stream.fail() || !stream.eof() || one_based <= 0) return false;
	std::size_t const candidate = static_cast<std::size_t>(one_based - 1);
	if (candidate >= vertex_count) return false;
	zero_based_index = candidate;
	return true;
}
inline std::string lowercase_ascii(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return text;
}
}

[[nodiscard]] inline SurfaceImportResult import_surface_from_obj(std::filesystem::path const& path)
{
	SurfaceImportResult result{};
	result.source_format = "M16 ASCII OBJ subset";
	std::ifstream file{ path };
	if (!file) { surface_import_detail::add_message(result, "Surface import failed: could not open '" + path.string() + "'"); return result; }
	std::string line;
	std::size_t line_number = 0u;
	while (std::getline(file, line)) {
		++line_number;
		auto const comment = line.find('#');
		if (comment != std::string::npos) line.erase(comment);
		line = surface_import_detail::trim(line);
		if (line.empty()) continue;
		std::istringstream stream{ line };
		std::string tag;
		stream >> tag;
		if (tag == "v") {
			std::string sx, sy, sz, extra;
			stream >> sx >> sy >> sz >> extra;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			if (!extra.empty() || !surface_import_detail::parse_float(sx, x) || !surface_import_detail::parse_float(sy, y) || !surface_import_detail::parse_float(sz, z)) { ++result.skipped_lines; surface_import_detail::add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed vertex"); continue; }
			result.mesh.positions.emplace_back(x, y, z);
			continue;
		}
		if (tag == "f") {
			std::string sa, sb, sc, extra;
			stream >> sa >> sb >> sc >> extra;
			std::size_t a = 0u, b = 0u, c = 0u;
			if (!extra.empty() || !surface_import_detail::parse_index(sa, result.mesh.positions.size(), a) || !surface_import_detail::parse_index(sb, result.mesh.positions.size(), b) || !surface_import_detail::parse_index(sc, result.mesh.positions.size(), c) || a == b || b == c || a == c) { ++result.skipped_lines; surface_import_detail::add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": malformed triangle face"); continue; }
			result.mesh.triangles.push_back(SurfaceTriangle{a, b, c});
			continue;
		}
		if (tag == "vn" || tag == "vt" || tag == "o" || tag == "g" || tag == "s" || tag == "usemtl" || tag == "mtllib") continue;
		++result.skipped_lines;
		surface_import_detail::add_message(result, "Skipped OBJ line " + std::to_string(line_number) + ": unsupported statement '" + tag + "'");
	}
	if (result.mesh.positions.empty()) { surface_import_detail::add_message(result, "Surface import failed: OBJ contained no valid vertices"); return result; }
	if (result.mesh.triangles.empty()) { surface_import_detail::add_message(result, "Surface import failed: OBJ contained no valid triangles"); return result; }
	result.succeeded = true;
	surface_import_detail::add_message(result, "Loaded surface '" + path.string() + "'");
	surface_import_detail::add_message(result, "Format: " + result.source_format);
	surface_import_detail::add_message(result, "Vertices: " + std::to_string(result.mesh.vertex_count()));
	surface_import_detail::add_message(result, "Triangles: " + std::to_string(result.mesh.triangle_count()));
	return result;
}

[[nodiscard]] inline SurfaceImportResult import_surface(std::filesystem::path const& path)
{
	if (surface_import_detail::lowercase_ascii(path.extension().string()) == ".obj") return import_surface_from_obj(path);
	SurfaceImportResult result{};
	surface_import_detail::add_message(result, "Surface import failed: unsupported file extension '" + path.extension().string() + "'");
	return result;
}

} // namespace sfm::scene
