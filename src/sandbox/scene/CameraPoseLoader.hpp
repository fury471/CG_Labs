#pragma once

#include "CameraPose.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

/// Result of reading a camera-pose file.
///
/// The loader reports all recoverable issues so the sandbox UI can explain why
/// a pose file failed instead of silently falling back to generated poses.
struct CameraPoseLoadResult final
{
	CameraPoseSet poses{};
	bool succeeded{ false };
	std::size_t skipped_lines{ 0u };
	std::vector<std::string> messages{};
	std::string source_format{};
	std::string source_convention{};
};

/// Loads camera poses from the small milestone text format.
///
/// Supported line format:
///
/// ```text
/// eye_x eye_y eye_z target_x target_y target_z r g b
/// ```
///
/// Commas are accepted as separators, text after `#` is a comment, and colour
/// values may be normalized `[0, 1]` floats or byte-style `[0, 255]` values.
/// This format is already in the internal graphics convention: camera-to-world
/// is built from a graphics view where local camera space looks along negative Z
/// with positive Y as up.
[[nodiscard]] CameraPoseLoadResult load_camera_poses_from_text_file(std::filesystem::path const& path);

/// Loads COLMAP text-format image poses from `images.txt`.
///
/// COLMAP image records store world-to-camera extrinsics with camera axes
/// +X right, +Y down, +Z forward. The loader converts each pose into the
/// sandbox internal graphics camera-to-world convention: +X right, +Y up,
/// -Z forward.
[[nodiscard]] CameraPoseLoadResult load_camera_poses_from_colmap_images_file(std::filesystem::path const& path);

/// Dispatches camera-pose loading by filename/extension.
///
/// - `images.txt` uses the COLMAP text `images.txt` importer.
/// - `.txt`, `.poses` and `.cam` use the milestone eye/target text importer.
[[nodiscard]] CameraPoseLoadResult load_camera_poses_from_file(std::filesystem::path const& path);

} // namespace sfm::scene
