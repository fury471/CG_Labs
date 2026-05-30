#include "CameraPoseLoader.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <locale>
#include <sstream>
#include <string_view>
#include <utility>

namespace sfm::scene
{
namespace
{

bool parse_float_token(std::string const& token, float& value)
{
	// Keep parsing behavior aligned with PointCloudLoader: portable across the
	// CI standard libraries and independent of the user's system locale.
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof() && std::isfinite(value);
}

void add_message(CameraPoseLoadResult& result, std::string message)
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

glm::vec3 normalize_colour(glm::vec3 colour) noexcept
{
	if (colour.r > 1.0f || colour.g > 1.0f || colour.b > 1.0f)
		colour /= 255.0f;
	return glm::clamp(colour, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
}

bool is_valid_vector(glm::vec3 const& value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool can_build_view(glm::vec3 const& eye, glm::vec3 const& target) noexcept
{
	glm::vec3 const forward = target - eye;
	float const length_squared = glm::dot(forward, forward);
	if (length_squared < 1.0e-8f)
		return false;

	// Avoid the singular case where forward is almost parallel to the fixed up
	// vector used by glm::lookAt.
	glm::vec3 const direction = glm::normalize(forward);
	float const parallel = std::abs(glm::dot(direction, glm::vec3{ 0.0f, 1.0f, 0.0f }));
	return parallel < 0.995f;
}

} // namespace

CameraPoseLoadResult load_camera_poses_from_text_file(std::filesystem::path const& path)
{
	CameraPoseLoadResult result{};
	std::ifstream file{ path };
	if (!file) {
		add_message(result, "Camera pose load failed: could not open '" + path.string() + "'");
		return result;
	}

	std::vector<CameraPose> poses;
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
		if (values.size() != 9u) {
			++result.skipped_lines;
			add_message(result, "Skipped line " + std::to_string(line_number) + ": expected 9 numeric fields");
			continue;
		}

		glm::vec3 const eye{ values[0], values[1], values[2] };
		glm::vec3 const target{ values[3], values[4], values[5] };
		if (!is_valid_vector(eye) || !is_valid_vector(target)) {
			++result.skipped_lines;
			add_message(result, "Skipped line " + std::to_string(line_number) + ": eye or target contains a non-finite value");
			continue;
		}
		if (!can_build_view(eye, target)) {
			++result.skipped_lines;
			add_message(result, "Skipped line " + std::to_string(line_number) + ": degenerate camera direction");
			continue;
		}

		CameraPose pose{};
		glm::mat4 const world_to_camera = glm::lookAt(eye, target, glm::vec3{ 0.0f, 1.0f, 0.0f });
		pose.camera_to_world = glm::inverse(world_to_camera);
		pose.colour = normalize_colour(glm::vec3{ values[6], values[7], values[8] });
		poses.push_back(pose);
	}

	if (poses.empty()) {
		add_message(result, "Camera pose load failed: file contained no valid poses");
		return result;
	}

	result.succeeded = true;
	result.poses = CameraPoseSet{ std::move(poses) };
	add_message(result, "Loaded camera poses '" + path.string() + "'");
	add_message(result, "Valid camera poses: " + std::to_string(result.poses.size()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped malformed pose lines: " + std::to_string(result.skipped_lines));
	return result;
}

} // namespace sfm::scene
