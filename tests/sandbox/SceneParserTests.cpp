#include "sandbox/scene/CameraPoseLoader.hpp"
#include "sandbox/scene/PointCloudLoader.hpp"

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
		{ "camera pose loader accepts supported rows", test_camera_pose_loader_accepts_supported_rows },
		{ "camera pose loader rejects degenerate inputs", test_camera_pose_loader_rejects_degenerate_inputs },
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
