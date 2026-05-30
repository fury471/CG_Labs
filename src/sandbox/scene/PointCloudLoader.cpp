#include "PointCloudLoader.hpp"

#include <algorithm>
#include <locale>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string_view>

namespace sfm::scene
{
namespace
{

	bool parse_float_token(std::string const& token, float& value)
	{
		std::istringstream stream{ token };
		stream.imbue(std::locale::classic());

		stream >> value;
		stream >> std::ws;

		return !stream.fail() && stream.eof() && std::isfinite(value);
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
			char const* first = token.data();
			char const* last = token.data() + token.size();
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

} // namespace sfm::scene
