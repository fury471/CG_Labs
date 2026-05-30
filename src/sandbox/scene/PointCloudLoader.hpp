#pragma once

#include "PointCloud.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

/// Result of reading a point-cloud text file.
///
/// Loading is expected to fail during development when files are malformed or
/// paths are wrong, so the loader returns diagnostics instead of throwing or
/// silently falling back. The application decides whether a fallback cloud is
/// acceptable for a given run.
struct PointCloudLoadResult final
{
	PointCloud cloud{};
	bool succeeded{ false };
	std::size_t skipped_lines{ 0u };
	std::vector<std::string> messages{};
};

/// Loads a small text point cloud.
///
/// Supported line formats:
///
/// ```text
/// x y z
/// x y z r g b
/// x,y,z
/// x,y,z,r,g,b
/// ```
///
/// Lines may contain comments after `#`. RGB values may be either normalized
/// `[0, 1]` floats or byte-style `[0, 255]` values. Position values must be
/// finite. This intentionally small format is enough for Milestone 7 validation
/// without pretending to be a full PLY/LAS/OBJ importer.
[[nodiscard]] PointCloudLoadResult load_point_cloud_from_text_file(std::filesystem::path const& path);

} // namespace sfm::scene
