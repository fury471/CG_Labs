#include "sandbox/scene/CameraPoseLoader.hpp"
#include "sandbox/scene/PointCloudLoader.hpp"
#include "sandbox/scene/SandboxProject.hpp"

#include <glm/glm.hpp>

#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{

class TestFailure final : public std::runtime_error
{
public:
	explicit TestFailure(std::string message)
		: std::runtime_error(std::move(message))
	{
	}
};

void require(bool condition, std::string const& message)
{
	if (!condition)
		throw TestFailure{ message };
}

void require_equal(std::size_t actual, std::size_t expected, std::string const& label)
{
	if (actual != expected) {
		std::ostringstream stream;
		stream << label << ": expected " << expected << ", got " << actual;
		throw TestFailure{ stream.str() };
	}
}

void require_near(float actual, float expected, float epsilon, std::string const& label)
{
	if (std::abs(actual - expected) > epsilon) {
		std::ostringstream stream;
		stream << label << ": expected " << expected << ", got " << actual;
		throw TestFailure{ stream.str() };
	}
}

void require_vec3_near(glm::vec3 actual, glm::vec3 expected, float epsilon, std::string const& label)
{
	require_near(actual.x, expected.x, epsilon, label + " x");
	require_near(actual.y, expected.y, epsilon, label + " y");
	require_near(actual.z, expected.z, epsilon, label + " z");
}

class TemporaryDirectory final
{
public:
	TemporaryDirectory()
	{
		std::filesystem::path base = std::filesystem::temp_directory_path();
		for (int attempt = 0; attempt < 100; ++attempt) {
			std::filesystem::path candidate = base / ("sfm_sandbox_scene_tests_" + std::to_string(attempt) + "_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
			std::error_code error;
			if (std::filesystem::create_directory(candidate, error)) {
				m_path = std::move(candidate);
				return;
			}
		}
		throw TestFailure{ "failed to create temporary test directory" };
	}

	~TemporaryDirectory()
	{
		std::error_code ignored;
		std::filesystem::remove_all(m_path, ignored);
	}

	TemporaryDirectory(TemporaryDirectory const&) = delete;
	TemporaryDirectory& operator=(TemporaryDirectory const&) = delete;

	[[nodiscard]] std::filesystem::path const& path() const noexcept { return m_path; }

private:
	std::filesystem::path m_path{};
};

std::filesystem::path write_text_file(TemporaryDirectory const& directory, std::string_view filename, std::string_view content)
{
	std::filesystem::path const path = directory.path() / filename;
	std::ofstream file{ path };
	if (!file)
		throw TestFailure{ "failed to create test file: " + path.string() };
	file << content;
	return path;
}

void test_point_cloud_loader_accepts_supported_rows()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "points.xyzrgb",
	                                                   R"data(# comments are ignored
1 2 3
4,5,6,255,128,0
7 8 9 0.25 0.5 0.75
not numeric
10 11
)data");

	sfm::scene::PointCloudLoadResult const result = sfm::scene::load_point_cloud_from_text_file(path);
	require(result.succeeded, "point cloud loader should succeed when at least one row is valid");
	require_equal(result.cloud.size(), 3u, "valid point count");
	require_equal(result.skipped_lines, 2u, "skipped point lines");

	auto const points = result.cloud.points();
	require_near(points[0].colour.r, 0.85f, 0.0001f, "default point colour r");
	require_near(points[1].colour.r, 1.0f, 0.0001f, "byte colour normalization r");
	require_near(points[1].colour.g, 128.0f / 255.0f, 0.0001f, "byte colour normalization g");
	require_near(points[2].colour.b, 0.75f, 0.0001f, "normalized colour preserved b");
}

void test_point_cloud_loader_fails_on_empty_or_missing_file()
{
	TemporaryDirectory directory;
	std::filesystem::path const malformed = write_text_file(directory,
	                                                        "malformed.xyzrgb",
	                                                        R"data(# no valid points here
abc
1 2
1 2 3 4
)data");

	sfm::scene::PointCloudLoadResult const malformed_result = sfm::scene::load_point_cloud_from_text_file(malformed);
	require(!malformed_result.succeeded, "point cloud loader should fail when no valid points exist");
	require_equal(malformed_result.skipped_lines, 3u, "all malformed point rows skipped");

	sfm::scene::PointCloudLoadResult const missing_result = sfm::scene::load_point_cloud_from_text_file(directory.path() / "missing.xyzrgb");
	require(!missing_result.succeeded, "point cloud loader should fail for missing files");
}

void test_ascii_ply_loader_accepts_coloured_vertices_and_statistics()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "points.ply",
	                                                   R"data(ply
format ascii 1.0
comment test sample
element vertex 3
property float x
property float y
property float z
property uchar red
property uchar green
property uchar blue
end_header
-1 0 2 255 0 0
3 4 -2 0 128 255
0 2 1 0.25 0.50 0.75
)data");

	sfm::scene::PointCloudLoadResult const result = sfm::scene::load_point_cloud_from_file(path);
	require(result.succeeded, "ASCII PLY loader should accept the supported coloured subset");
	require_equal(result.cloud.size(), 3u, "PLY valid point count");
	require_equal(result.skipped_lines, 0u, "PLY skipped line count");

	auto const points = result.cloud.points();
	require_near(points[0].colour.r, 1.0f, 0.0001f, "PLY byte colour r");
	require_near(points[1].colour.g, 128.0f / 255.0f, 0.0001f, "PLY byte colour g");
	require_near(points[2].colour.b, 0.75f, 0.0001f, "PLY normalized colour b");

	sfm::scene::PointCloudStatistics const statistics = result.cloud.statistics();
	require(statistics.has_bounds, "PLY statistics should report bounds");
	require_equal(statistics.point_count, 3u, "PLY statistics point count");
	require_near(statistics.bounds_min.x, -1.0f, 0.0001f, "PLY bounds min x");
	require_near(statistics.bounds_min.z, -2.0f, 0.0001f, "PLY bounds min z");
	require_near(statistics.bounds_max.x, 3.0f, 0.0001f, "PLY bounds max x");
	require_near(statistics.bounds_max.y, 4.0f, 0.0001f, "PLY bounds max y");
	require_near(statistics.bounds_extent.x, 4.0f, 0.0001f, "PLY bounds extent x");
	require(statistics.approximate_cpu_bytes >= 3u * sizeof(sfm::scene::PointSample), "PLY statistics should report CPU storage");
}

void test_ascii_ply_loader_rejects_unsupported_format_and_elements()
{
	TemporaryDirectory directory;
	std::filesystem::path const binary_path = write_text_file(directory,
	                                                          "binary.ply",
	                                                          R"data(ply
format binary_little_endian 1.0
element vertex 1
property float x
property float y
property float z
end_header
)data");
	std::filesystem::path const face_path = write_text_file(directory,
	                                                        "faces.ply",
	                                                        R"data(ply
format ascii 1.0
element vertex 1
property float x
property float y
property float z
element face 1
property list uchar int vertex_indices
end_header
0 0 0
3 0 1 2
)data");
	std::filesystem::path const missing_position_path = write_text_file(directory,
	                                                                    "missing_position.ply",
	                                                                    R"data(ply
format ascii 1.0
element vertex 1
property float x
property float y
property uchar red
end_header
0 0 255
)data");

	require(!sfm::scene::load_point_cloud_from_ascii_ply_file(binary_path).succeeded, "binary PLY should be rejected");
	require(!sfm::scene::load_point_cloud_from_ascii_ply_file(face_path).succeeded, "PLY with non-zero face element should be rejected");
	require(!sfm::scene::load_point_cloud_from_ascii_ply_file(missing_position_path).succeeded, "PLY missing z should be rejected");
}

void test_point_cloud_dispatcher_preserves_text_loader()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory, "points.txt", "0 0 0\n1 2 3 255 255 255\n");
	std::filesystem::path const unsupported = write_text_file(directory, "points.unsupported", "0 0 0\n");

	require(sfm::scene::load_point_cloud_from_file(path).succeeded, "dispatcher should preserve text point-cloud loading");
	require(!sfm::scene::load_point_cloud_from_file(unsupported).succeeded, "dispatcher should reject unsupported extensions");
}

void test_camera_pose_loader_accepts_supported_rows()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "poses.txt",
	                                                   R"data(# eye target colour
0 1 4 0 1 0 255 128 0
4,2,0,0,1,0,0.2,0.4,0.6
bad row
0 0 0 0 0 0 1 1 1
)data");

	sfm::scene::CameraPoseLoadResult const result = sfm::scene::load_camera_poses_from_text_file(path);
	require(result.succeeded, "camera pose loader should succeed when at least one row is valid");
	require_equal(result.poses.size(), 2u, "valid camera pose count");
	require_equal(result.skipped_lines, 2u, "skipped camera pose lines");
	require(result.source_format == "milestone eye-target text", "text camera pose source format should be reported");

	auto const pose = result.poses.poses()[0];
	require_equal(pose.metadata.index, 0u, "text pose metadata index");
	require(pose.metadata.source_file == path.string(), "text pose metadata should preserve source file");
	require_vec3_near(glm::vec3{ pose.camera_to_world[3] }, glm::vec3{ 0.0f, 1.0f, 4.0f }, 0.0001f, "text pose camera position");
}

void test_camera_pose_loader_rejects_degenerate_inputs()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "bad_poses.txt",
	                                                   R"data(# no valid poses
0 0 0 0 0 0 1 1 1
0 0 0 0 5 0 1 1 1
nan 0 0 0 0 1 1 1 1
1 2 3
)data");

	sfm::scene::CameraPoseLoadResult const result = sfm::scene::load_camera_poses_from_text_file(path);
	require(!result.succeeded, "camera pose loader should fail when no valid poses exist");
	require_equal(result.skipped_lines, 4u, "all malformed pose rows skipped");
}

void test_camera_pose_dispatcher_rejects_unsupported_extensions()
{
	TemporaryDirectory directory;
	std::filesystem::path const unsupported = write_text_file(directory, "poses.json", "{}\n");
	sfm::scene::CameraPoseLoadResult const result = sfm::scene::load_camera_poses_from_file(unsupported);
	require(!result.succeeded, "camera pose dispatcher should reject unsupported extensions for M14");
}

void test_colmap_images_loader_converts_identity_pose_to_internal_graphics_convention()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "images.txt",
	                                                   R"data(# IMAGE_ID QW QX QY QZ TX TY TZ CAMERA_ID NAME
1 1 0 0 0 0 0 0 7 image_0001.jpg
0 0 -1 10 10 2
)data");

	sfm::scene::CameraPoseLoadResult const result = sfm::scene::load_camera_poses_from_colmap_images_file(path);
	require(result.succeeded, "COLMAP images.txt loader should accept an identity image pose");
	require_equal(result.poses.size(), 1u, "COLMAP valid pose count");
	require(result.source_format == "COLMAP text images.txt", "COLMAP source format should be reported");

	auto const pose = result.poses.poses()[0];
	require_vec3_near(glm::vec3{ pose.camera_to_world[3] }, glm::vec3{ 0.0f, 0.0f, 0.0f }, 0.0001f, "COLMAP identity camera center");
	require_vec3_near(glm::vec3{ pose.camera_to_world[0] }, glm::vec3{ 1.0f, 0.0f, 0.0f }, 0.0001f, "COLMAP identity right axis");
	require_vec3_near(glm::vec3{ pose.camera_to_world[1] }, glm::vec3{ 0.0f, -1.0f, 0.0f }, 0.0001f, "COLMAP identity internal up basis in world");
	require_vec3_near(-glm::vec3{ pose.camera_to_world[2] }, glm::vec3{ 0.0f, 0.0f, 1.0f }, 0.0001f, "COLMAP identity internal forward direction");
	require_equal(static_cast<std::size_t>(pose.metadata.camera_id), 7u, "COLMAP camera id metadata");
	require(pose.metadata.image_name == "image_0001.jpg", "COLMAP image name metadata should be preserved");
}

void test_colmap_images_loader_computes_camera_center_from_world_to_camera_translation()
{
	TemporaryDirectory directory;
	std::filesystem::path const path = write_text_file(directory,
	                                                   "images.txt",
	                                                   R"data(# identity rotation with non-zero world-to-camera translation
2 1 0 0 0 -1 -2 -3 4 image_0002.jpg

)data");

	sfm::scene::CameraPoseLoadResult const result = sfm::scene::load_camera_poses_from_file(path);
	require(result.succeeded, "dispatcher should route images.txt to the COLMAP importer");
	require_equal(result.poses.size(), 1u, "COLMAP translated valid pose count");

	auto const pose = result.poses.poses()[0];
	require_vec3_near(glm::vec3{ pose.camera_to_world[3] }, glm::vec3{ 1.0f, 2.0f, 3.0f }, 0.0001f, "COLMAP camera center from -R^T t");
}

void test_sandbox_project_manifest_resolves_relative_paths()
{
	TemporaryDirectory directory;
	std::filesystem::path const manifest = write_text_file(directory,
	                                                       "project.sfmproj",
	                                                       R"data(# relative paths resolve against this file
point_cloud = data/points.ply
camera_poses = poses.txt
surface = meshes/surface.obj
image = images/source.ppm
)data");

	sfm::scene::SandboxProjectLoadResult const result = sfm::scene::load_sandbox_project(manifest);
	require(result.succeeded, "sandbox project manifest should load supported keys");
	require(result.project.manifest_path == manifest, "manifest path should be preserved");
	require(result.project.point_cloud_path == (directory.path() / "data/points.ply").lexically_normal(), "point-cloud path should resolve relative to manifest");
	require(result.project.camera_pose_path == (directory.path() / "poses.txt").lexically_normal(), "camera-pose path should resolve relative to manifest");
	require(result.project.surface_path == (directory.path() / "meshes/surface.obj").lexically_normal(), "surface path should resolve relative to manifest");
	require(result.project.image_path == (directory.path() / "images/source.ppm").lexically_normal(), "image path should resolve relative to manifest");
}

void test_sandbox_project_manifest_allows_empty_optional_paths()
{
	TemporaryDirectory directory;
	std::filesystem::path const manifest = write_text_file(directory,
	                                                       "optional.sfmproj",
	                                                       R"data(point_cloud = data/points.ply
camera_poses =
surface =
image =
)data");

	sfm::scene::SandboxProjectLoadResult const result = sfm::scene::load_sandbox_project(manifest);
	require(result.succeeded, "sandbox project manifest should allow empty optional asset paths");
	require(result.project.point_cloud_path == (directory.path() / "data/points.ply").lexically_normal(), "point-cloud path should still resolve");
	require(result.project.camera_pose_path.empty(), "empty camera pose path should disable camera layer");
	require(result.project.surface_path.empty(), "empty surface path should disable surface layer");
	require(result.project.image_path.empty(), "empty image path should disable image layer");
}

void test_sandbox_project_manifest_rejects_invalid_input()
{
	TemporaryDirectory directory;
	std::filesystem::path const missing_key_manifest = write_text_file(directory,
	                                                                   "missing.sfmproj",
	                                                                   R"data(camera_poses = poses.txt
surface = surface.obj
image = image.ppm
)data");
	std::filesystem::path const unsupported_key_manifest = write_text_file(directory,
	                                                                       "unsupported.sfmproj",
	                                                                       R"data(point_cloud = points.ply
camera_poses = poses.txt
surface = surface.obj
image = image.ppm
unknown = value
)data");

	require(!sfm::scene::load_sandbox_project(missing_key_manifest).succeeded, "project manifest should reject missing required paths");
	require(!sfm::scene::load_sandbox_project(unsupported_key_manifest).succeeded, "project manifest should reject unsupported keys");
	require(!sfm::scene::load_sandbox_project(directory.path() / "does_not_exist.sfmproj").succeeded, "project manifest should reject missing files");
}

using TestFunction = void (*)();

struct TestCase final
{
	std::string_view name;
	TestFunction function;
};

} // namespace

int main()
{
	std::vector<TestCase> const tests{
		{ "point cloud loader accepts supported rows", test_point_cloud_loader_accepts_supported_rows },
		{ "point cloud loader fails on empty or missing file", test_point_cloud_loader_fails_on_empty_or_missing_file },
		{ "ASCII PLY loader accepts coloured vertices and statistics", test_ascii_ply_loader_accepts_coloured_vertices_and_statistics },
		{ "ASCII PLY loader rejects unsupported format and elements", test_ascii_ply_loader_rejects_unsupported_format_and_elements },
		{ "point cloud dispatcher preserves text loader", test_point_cloud_dispatcher_preserves_text_loader },
		{ "camera pose loader accepts supported rows", test_camera_pose_loader_accepts_supported_rows },
		{ "camera pose loader rejects degenerate inputs", test_camera_pose_loader_rejects_degenerate_inputs },
		{ "camera pose dispatcher rejects unsupported extensions", test_camera_pose_dispatcher_rejects_unsupported_extensions },
		{ "COLMAP images loader converts identity pose to internal graphics convention", test_colmap_images_loader_converts_identity_pose_to_internal_graphics_convention },
		{ "COLMAP images loader computes camera center from world-to-camera translation", test_colmap_images_loader_computes_camera_center_from_world_to_camera_translation },
		{ "sandbox project manifest resolves relative paths", test_sandbox_project_manifest_resolves_relative_paths },
		{ "sandbox project manifest allows empty optional paths", test_sandbox_project_manifest_allows_empty_optional_paths },
		{ "sandbox project manifest rejects invalid input", test_sandbox_project_manifest_rejects_invalid_input },
	};

	int failed = 0;
	for (TestCase const& test : tests) {
		try {
			test.function();
			std::cout << "[PASS] " << test.name << '\n';
		} catch (std::exception const& exception) {
			++failed;
			std::cerr << "[FAIL] " << test.name << ": " << exception.what() << '\n';
		}
	}

	if (failed != 0) {
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}
	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
