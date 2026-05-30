#include "CameraPoseLoader.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
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

bool parse_int_token(std::string const& token, int& value)
{
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof();
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

std::string lowercase_ascii(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return text;
}

std::vector<std::string> parse_tokens(std::string line)
{
	std::replace(line.begin(), line.end(), ',', ' ');
	std::vector<std::string> tokens;
	std::istringstream stream{ line };
	std::string token;
	while (stream >> token)
		tokens.push_back(std::move(token));
	return tokens;
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

bool is_valid_matrix(glm::mat4 const& value) noexcept
{
	for (int column = 0; column < 4; ++column) {
		for (int row = 0; row < 4; ++row) {
			if (!std::isfinite(value[column][row]))
				return false;
		}
	}
	return true;
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

void annotate_pose(CameraPose& pose,
                   std::size_t index,
                   std::size_t source_line,
                   int source_id,
                   int camera_id,
                   std::filesystem::path const& path,
                   std::string source_format,
                   std::string source_convention,
                   std::string image_name)
{
	pose.metadata.index = index;
	pose.metadata.source_line = source_line;
	pose.metadata.source_id = source_id;
	pose.metadata.camera_id = camera_id;
	pose.metadata.source_file = path.string();
	pose.metadata.source_format = std::move(source_format);
	pose.metadata.source_convention = std::move(source_convention);
	pose.metadata.image_name = std::move(image_name);
}

bool quaternion_is_usable(glm::quat const& q) noexcept
{
	float const norm_squared = glm::dot(q, q);
	return std::isfinite(norm_squared) && norm_squared > 1.0e-8f;
}

glm::mat4 colmap_world_to_camera_to_internal_camera_to_world(glm::quat qvec, glm::vec3 const& translation)
{
	qvec = glm::normalize(qvec);
	glm::mat3 const rotation_world_to_colmap_camera = glm::mat3_cast(qvec);
	glm::mat3 const rotation_colmap_camera_to_world = glm::transpose(rotation_world_to_colmap_camera);

	// COLMAP/OpenCV camera axes: +X right, +Y down, +Z forward.
	// Sandbox graphics axes:   +X right, +Y up,   -Z forward.
	// This diagonal basis change flips Y and Z before mapping into world space.
	glm::mat3 const colmap_camera_from_internal_camera{
		glm::vec3{ 1.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, -1.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, -1.0f },
	};
	glm::mat3 const rotation_internal_camera_to_world = rotation_colmap_camera_to_world * colmap_camera_from_internal_camera;
	glm::vec3 const camera_center_world = -(rotation_colmap_camera_to_world * translation);

	glm::mat4 camera_to_world{ 1.0f };
	camera_to_world[0] = glm::vec4{ rotation_internal_camera_to_world[0], 0.0f };
	camera_to_world[1] = glm::vec4{ rotation_internal_camera_to_world[1], 0.0f };
	camera_to_world[2] = glm::vec4{ rotation_internal_camera_to_world[2], 0.0f };
	camera_to_world[3] = glm::vec4{ camera_center_world, 1.0f };
	return camera_to_world;
}

} // namespace

CameraPoseLoadResult load_camera_poses_from_text_file(std::filesystem::path const& path)
{
	CameraPoseLoadResult result{};
	result.source_format = "milestone eye-target text";
	result.source_convention = "graphics eye/target rows converted to camera-to-world, local -Z forward, +Y up";

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
		annotate_pose(pose,
		              poses.size(),
		              line_number,
		              static_cast<int>(poses.size()),
		              -1,
		              path,
		              result.source_format,
		              result.source_convention,
		              "text_pose_" + std::to_string(poses.size()));
		poses.push_back(std::move(pose));
	}

	if (poses.empty()) {
		add_message(result, "Camera pose load failed: file contained no valid poses");
		return result;
	}

	result.succeeded = true;
	result.poses = CameraPoseSet{ std::move(poses) };
	add_message(result, "Loaded camera poses '" + path.string() + "'");
	add_message(result, "Format: " + result.source_format);
	add_message(result, "Convention: " + result.source_convention);
	add_message(result, "Valid camera poses: " + std::to_string(result.poses.size()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped malformed pose lines: " + std::to_string(result.skipped_lines));
	return result;
}

CameraPoseLoadResult load_camera_poses_from_colmap_images_file(std::filesystem::path const& path)
{
	CameraPoseLoadResult result{};
	result.source_format = "COLMAP text images.txt";
	result.source_convention = "COLMAP/OpenCV world-to-camera, +X right, +Y down, +Z forward; converted to graphics camera-to-world, +Y up, -Z forward";

	std::ifstream file{ path };
	if (!file) {
		add_message(result, "COLMAP camera pose load failed: could not open '" + path.string() + "'");
		return result;
	}

	std::vector<CameraPose> poses;
	std::string line;
	std::size_t line_number = 0u;
	bool expect_points2d_line = false;

	while (std::getline(file, line)) {
		++line_number;
		std::string const raw_line = trim(line);
		if (raw_line.empty() || raw_line.front() == '#')
			continue;

		if (expect_points2d_line) {
			// COLMAP text images are stored as two non-comment lines per image. M14
			// imports pose metadata only, so the POINTS2D line is deliberately skipped.
			expect_points2d_line = false;
			continue;
		}

		std::vector<std::string> const tokens = parse_tokens(raw_line);
		if (tokens.size() < 10u) {
			++result.skipped_lines;
			add_message(result, "Skipped COLMAP line " + std::to_string(line_number) + ": expected at least 10 fields");
			continue;
		}

		int image_id = -1;
		int camera_id = -1;
		float qw = 0.0f;
		float qx = 0.0f;
		float qy = 0.0f;
		float qz = 0.0f;
		float tx = 0.0f;
		float ty = 0.0f;
		float tz = 0.0f;
		bool const parsed = parse_int_token(tokens[0], image_id) &&
		                    parse_float_token(tokens[1], qw) &&
		                    parse_float_token(tokens[2], qx) &&
		                    parse_float_token(tokens[3], qy) &&
		                    parse_float_token(tokens[4], qz) &&
		                    parse_float_token(tokens[5], tx) &&
		                    parse_float_token(tokens[6], ty) &&
		                    parse_float_token(tokens[7], tz) &&
		                    parse_int_token(tokens[8], camera_id);
		if (!parsed) {
			++result.skipped_lines;
			add_message(result, "Skipped COLMAP line " + std::to_string(line_number) + ": malformed numeric pose fields");
			continue;
		}

		glm::quat const qvec{ qw, qx, qy, qz };
		glm::vec3 const translation{ tx, ty, tz };
		if (!quaternion_is_usable(qvec) || !is_valid_vector(translation)) {
			++result.skipped_lines;
			add_message(result, "Skipped COLMAP line " + std::to_string(line_number) + ": invalid quaternion or translation");
			continue;
		}

		CameraPose pose{};
		pose.camera_to_world = colmap_world_to_camera_to_internal_camera_to_world(qvec, translation);
		if (!is_valid_matrix(pose.camera_to_world)) {
			++result.skipped_lines;
			add_message(result, "Skipped COLMAP line " + std::to_string(line_number) + ": converted pose is non-finite");
			continue;
		}

		float const t = static_cast<float>(poses.size() % 12u) / 11.0f;
		pose.colour = glm::vec3{ 0.30f + 0.70f * t, 0.85f - 0.35f * t, 1.0f - 0.55f * t };
		annotate_pose(pose,
		              poses.size(),
		              line_number,
		              image_id,
		              camera_id,
		              path,
		              result.source_format,
		              result.source_convention,
		              tokens[9]);
		poses.push_back(std::move(pose));
		expect_points2d_line = true;
	}

	if (poses.empty()) {
		add_message(result, "COLMAP camera pose load failed: file contained no valid image poses");
		return result;
	}

	result.succeeded = true;
	result.poses = CameraPoseSet{ std::move(poses) };
	add_message(result, "Loaded COLMAP image poses '" + path.string() + "'");
	add_message(result, "Format: " + result.source_format);
	add_message(result, "Convention: " + result.source_convention);
	add_message(result, "Valid camera poses: " + std::to_string(result.poses.size()));
	if (result.skipped_lines > 0u)
		add_message(result, "Skipped malformed COLMAP pose lines: " + std::to_string(result.skipped_lines));
	return result;
}

CameraPoseLoadResult load_camera_poses_from_file(std::filesystem::path const& path)
{
	std::string const filename = lowercase_ascii(path.filename().string());
	std::string const extension = lowercase_ascii(path.extension().string());
	if (filename == "images.txt")
		return load_camera_poses_from_colmap_images_file(path);
	if (extension == ".txt" || extension == ".poses" || extension == ".cam")
		return load_camera_poses_from_text_file(path);

	CameraPoseLoadResult result{};
	add_message(result, "Camera pose load failed: unsupported file extension '" + extension + "'");
	return result;
}

} // namespace sfm::scene
