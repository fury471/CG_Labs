#include "PointCloudLoader.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace sfm::scene
{
namespace
{

bool parse_float_token(std::string const& token, float& value)
{
	// Floating-point std::from_chars is not implemented consistently across the
	// project's CI standard libraries yet. A classic-locale stream parser is
	// slower, but it is portable and correct for the small milestone text files.
	//
	// Use noskipws and check EOF directly after numeric extraction. Calling
	// `stream >> std::ws` after an exact-token parse can set failbit on some
	// standard-library implementations once EOF has already been reached.
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;

	return !stream.fail() && stream.eof() && std::isfinite(value);
}

bool parse_size_token(std::string const& token, std::size_t& value)
{
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof();
}

void add_message(PointCloudLoadResult& result, std::string message)
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

std::string lowercase(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char value) {
		return static_cast<char>(std::tolower(value));
	});
	return text;
}

std::vector<std::string> split_tokens(std::string const& line)
{
	std::vector<std::string> tokens;
	std::istringstream stream{ line };
	std::string token;
	while (stream >> token)
		tokens.push_back(token);
	return tokens;
}

std::vector<float> parse_numeric_fields(std::string line)
{
	// Treat commas as separators too. This keeps the accepted format practical
	// for quick CSV-style point dumps without adding a full CSV parser.
	std::replace(line.begin(), line.end(), ',', ' ');

	std::vector<float> values;
	std::istringstream stream{ line };
	std::string token;
	while (stream >> token) {
		float value = 0.0f;
		if (!parse_float_token(token, value))
			return {};
		values.push_back(value);
	}
	return values;
}

bool position_is_valid(glm::vec3 const& position) noexcept
{
	return std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z);
}

glm::vec3 normalize_colour(glm::vec3 colour) noexcept
{
	// If any component is larger than 1, interpret the triplet as byte-style RGB.
	// Otherwise keep normalized colour values. Clamp in both cases so accidental
	// minor out-of-range values do not propagate to shaders.
	if (colour.r > 1.0f || colour.g > 1.0f || colour.b > 1.0f)
		colour /= 255.0f;
	return glm::clamp(colour, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
}

bool is_supported_ply_scalar_type(std::string const& type)
{
	std::string const lowered = lowercase(type);
	return lowered == "char" || lowered == "uchar" || lowered == "short" || lowered == "ushort" ||
	       lowered == "int" || lowered == "uint" || lowered == "float" || lowered == "double" ||
	       lowered == "int8" || lowered == "uint8" || lowered == "int16" || lowered == "uint16" ||
	       lowered == "int32" || lowered == "uint32" || lowered == "float32" || lowered == "float64";
}

struct PlyHeader final
{
	std::size_t vertex_count{ 0u };
	std::vector<std::string> vertex_property_names{};
	bool saw_format{ false };
};

std::optional<std::size_t> find_property_index(std::vector<std::string> const& names, std::string_view primary, std::string_view alternate = {})
{
	for (std::size_t i = 0; i < names.size(); ++i) {
		if (names[i] == primary || (!alternate.empty() && names[i] == alternate))
			return i;
	}
	return std::nullopt;
}

bool read_ply_header(std::ifstream& file, std::filesystem::path const& path, PlyHeader& header, PointCloudLoadResult& result)
{
	std::string line;
	std::size_t line_number = 0u;
	if (!std::getline(file, line)) {
		add_message(result, "PLY load failed: empty file '" + path.string() + "'");
		return false;
	}
	++line_number;
	if (trim(line) != "ply") {
		add_message(result, "PLY load failed: missing ply magic header in '" + path.string() + "'");
		return false;
	}

	std::string active_element;
	std::size_t active_element_count = 0u;
	bool saw_vertex_element = false;

	while (std::getline(file, line)) {
		++line_number;
		line = trim(line);
		if (line.empty())
			continue;

		std::vector<std::string> const tokens = split_tokens(line);
		if (tokens.empty())
			continue;

		std::string const keyword = lowercase(tokens[0]);
		if (keyword == "comment" || keyword == "obj_info")
			continue;

		if (keyword == "format") {
			if (tokens.size() != 3u || lowercase(tokens[1]) != "ascii" || tokens[2] != "1.0") {
				add_message(result, "PLY load failed: only format ascii 1.0 is supported");
				return false;
			}
			header.saw_format = true;
			continue;
		}

		if (keyword == "element") {
			if (tokens.size() != 3u || !parse_size_token(tokens[2], active_element_count)) {
				add_message(result, "PLY load failed: malformed element declaration on header line " + std::to_string(line_number));
				return false;
			}
			active_element = lowercase(tokens[1]);
			if (active_element == "vertex") {
				if (saw_vertex_element) {
					add_message(result, "PLY load failed: multiple vertex elements are not supported");
					return false;
				}
				saw_vertex_element = true;
				header.vertex_count = active_element_count;
			} else if (active_element_count > 0u) {
				add_message(result, "PLY load failed: non-vertex element '" + active_element + "' is not supported in this milestone");
				return false;
			}
			continue;
		}

		if (keyword == "property") {
			if (active_element != "vertex")
				continue;
			if (tokens.size() >= 2u && lowercase(tokens[1]) == "list") {
				add_message(result, "PLY load failed: list vertex properties are not supported");
				return false;
			}
			if (tokens.size() != 3u || !is_supported_ply_scalar_type(tokens[1])) {
				add_message(result, "PLY load failed: malformed or unsupported vertex property on header line " + std::to_string(line_number));
				return false;
			}
			header.vertex_property_names.push_back(lowercase(tokens[2]));
			continue;
		}

		if (keyword == "end_header") {
			if (!header.saw_format) {
				add_message(result, "PLY load failed: missing format declaration");
				return false;
			}
			if (!saw_vertex_element) {
				add_message(result, "PLY load failed: missing vertex element");
				return false;
			}
			if (header.vertex_count == 0u) {
				add_message(result, "PLY load failed: vertex element has zero points");
				return false;
			}
			return true;
		}

		add_message(result, "PLY load failed: unsupported header keyword '" + keyword + "'");
		return false;
	}

	add_message(result, "PLY load failed: missing end_header");
	return false;
}

} // namespace

PointCloudLoadResult load_point_cloud_from_text_file(std::filesystem::path const& path)
{
	PointCloudLoadResult result{};
	std::ifstream file{ path };
	if (!file) {
		add_message(result, "Point cloud load failed: could not open '" + path.string() + "'");
		return result;
	}

	std::vector<PointSample> points;
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

		std::vector<float> const values = parse_numeric_fields(line);
		if (values.size() != 3u && values.size() != 6u) {
			++result.skipped_lines;
			add_message(result, "Skipped line " + std::to_string(line_number) + ": expected 3 or 6 numeric fields");
			continue;
		}

		PointSample sample{};
		sample.position = glm::vec3{ values[0], values[1], values[2] };
		if (!position_is_valid(sample.position)) {
			++result.skipped_lines;
			add_message(result, "Skipped line " + std::to_string(line_number) + ": position contains a non-finite value");
			continue;
		}

		if (values.size() == 6u)
			sample.colour = normalize_colour(glm::vec3{ values[3], values[4], values[5] });
		else
			sample.colour = glm::vec3{ 0.85f, 0.90f, 1.0f };

		points.push_back(sample);
	}

	if (points.empty()) {
		add_message(result, "Point cloud load failed: file contained no valid points");
		return result;
	}

	result.succeeded = true;
	result.cloud = PointCloud{ std::move(points) };
	add_message(result, "Loaded point cloud '" + path.string() + "'");
	add_message(result, "Valid points: " + std::to_string(result.cloud.size()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped malformed lines: " + std::to_string(result.skipped_lines));
	return result;
}

PointCloudLoadResult load_point_cloud_from_ascii_ply_file(std::filesystem::path const& path)
{
	PointCloudLoadResult result{};
	std::ifstream file{ path };
	if (!file) {
		add_message(result, "PLY load failed: could not open '" + path.string() + "'");
		return result;
	}

	PlyHeader header{};
	if (!read_ply_header(file, path, header, result))
		return result;

	std::optional<std::size_t> const x_index = find_property_index(header.vertex_property_names, "x");
	std::optional<std::size_t> const y_index = find_property_index(header.vertex_property_names, "y");
	std::optional<std::size_t> const z_index = find_property_index(header.vertex_property_names, "z");
	if (!x_index || !y_index || !z_index) {
		add_message(result, "PLY load failed: vertex properties must include x, y and z");
		return result;
	}

	std::optional<std::size_t> const r_index = find_property_index(header.vertex_property_names, "red", "r");
	std::optional<std::size_t> const g_index = find_property_index(header.vertex_property_names, "green", "g");
	std::optional<std::size_t> const b_index = find_property_index(header.vertex_property_names, "blue", "b");
	bool const has_colour = r_index && g_index && b_index;

	std::vector<PointSample> points;
	points.reserve(header.vertex_count);
	std::string line;
	std::size_t vertex_rows_seen = 0u;
	while (vertex_rows_seen < header.vertex_count && std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line.starts_with("comment"))
			continue;
		++vertex_rows_seen;

		std::vector<float> const values = parse_numeric_fields(line);
		if (values.size() != header.vertex_property_names.size()) {
			++result.skipped_lines;
			add_message(result, "Skipped PLY vertex row " + std::to_string(vertex_rows_seen) + ": property count mismatch");
			continue;
		}

		PointSample sample{};
		sample.position = glm::vec3{ values[*x_index], values[*y_index], values[*z_index] };
		if (!position_is_valid(sample.position)) {
			++result.skipped_lines;
			add_message(result, "Skipped PLY vertex row " + std::to_string(vertex_rows_seen) + ": position contains a non-finite value");
			continue;
		}
		sample.colour = has_colour ? normalize_colour(glm::vec3{ values[*r_index], values[*g_index], values[*b_index] })
		                           : glm::vec3{ 0.85f, 0.90f, 1.0f };
		points.push_back(sample);
	}

	if (vertex_rows_seen != header.vertex_count) {
		add_message(result, "PLY load failed: expected " + std::to_string(header.vertex_count) + " vertex rows, found " + std::to_string(vertex_rows_seen));
		return result;
	}
	if (points.empty()) {
		add_message(result, "PLY load failed: file contained no valid points");
		return result;
	}

	result.succeeded = true;
	result.cloud = PointCloud{ std::move(points) };
	add_message(result, "Loaded ASCII PLY point cloud '" + path.string() + "'");
	add_message(result, "Declared vertices: " + std::to_string(header.vertex_count));
	add_message(result, "Valid points: " + std::to_string(result.cloud.size()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped malformed PLY vertices: " + std::to_string(result.skipped_lines));
	return result;
}

PointCloudLoadResult load_point_cloud_from_file(std::filesystem::path const& path)
{
	std::string const extension = lowercase(path.extension().string());
	if (extension == ".ply")
		return load_point_cloud_from_ascii_ply_file(path);
	if (extension == ".xyz" || extension == ".xyzrgb" || extension == ".txt")
		return load_point_cloud_from_text_file(path);

	PointCloudLoadResult result{};
	add_message(result, "Point cloud load failed: unsupported extension '" + extension + "'");
	return result;
}

} // namespace sfm::scene
