#pragma once

#include "CameraPose.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

/// Result of reading a camera-pose text file.
///
/// The loader reports all recoverable issues so the sandbox UI can explain why
/// a pose file failed instead of silently falling back to generated poses.
struct CameraPoseLoadResult final
{
	CameraPoseSet poses{};
	bool succeeded{ false };
	std::size_t skipped_lines{ 0u };
	std::vector<std::string> messages{};
};

/// Loads camera poses from a small text file.
///
/// Supported line format:
///
/// ```text
/// eye_x eye_y eye_z target_x target_y target_z r g b
/// ```
///
/// Commas are accepted as separators, text after `#` is a comment, and colour
/// values may be normalized `[0, 1]` floats or byte-style `[0, 255]` values.
/// This is intentionally a minimal milestone format, not a full SfM importer.
[[nodiscard]] CameraPoseLoadResult load_camera_poses_from_text_file(std::filesystem::path const& path);

} // namespace sfm::scene
