#pragma once

#include "PointCloud.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace sfm::scene
{

/// Result of reading a point-cloud file.
///
/// Loading is expected to fail during development when files are malformed,
/// paths are wrong or formats are unsupported, so the loader returns diagnostics
/// instead of throwing or silently falling back. The application decides whether
/// a fallback cloud is acceptable for a given run.
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
/// finite.
[[nodiscard]] PointCloudLoadResult load_point_cloud_from_text_file(std::filesystem::path const& path);

/// Loads an ASCII PLY point cloud with coloured vertices.
///
/// Supported subset:
///
/// - `ply` magic header;
/// - `format ascii 1.0`;
/// - one `element vertex <count>` section;
/// - scalar vertex properties containing at least `x`, `y`, `z`;
/// - optional colour properties `red`, `green`, `blue` or `r`, `g`, `b`.
///
/// Binary PLY, list properties, extra elements with non-zero counts and missing
/// position fields are rejected deliberately. This keeps the importer honest and
/// deterministic while later milestones decide whether broader PLY support is
/// worth the complexity.
[[nodiscard]] PointCloudLoadResult load_point_cloud_from_ascii_ply_file(std::filesystem::path const& path);

/// Dispatches to the supported point-cloud loader based on file extension.
/// Currently supported: `.xyz`, `.xyzrgb`, `.txt` and `.ply`.
[[nodiscard]] PointCloudLoadResult load_point_cloud_from_file(std::filesystem::path const& path);

} // namespace sfm::scene
