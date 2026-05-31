#include "SfmSandboxMeshWorkflow.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace
{

void require(bool condition, char const* message)
{
	if (!condition)
		throw std::runtime_error{ message };
}

sfm::scene::PointCloud make_square_cloud()
{
	std::vector<sfm::scene::PointSample> points{
		{ { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { 1.0f, 0.1f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.2f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { 1.0f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
	};
	return sfm::scene::PointCloud{ std::move(points) };
}

sfm::scene::PointToMeshBuildResult wait_for_build(sfm::app::SfmSandboxMeshWorkflow& workflow)
{
	for (int attempt = 0; attempt < 200; ++attempt) {
		std::optional<sfm::scene::PointToMeshBuildResult> result = workflow.poll_completed_build();
		if (result.has_value())
			return std::move(*result);
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	}
	throw std::runtime_error{ "mesh workflow async build did not finish in time" };
}

} // namespace

int main()
{
	try {
		std::filesystem::path const export_path = std::filesystem::temp_directory_path() / "sfm_mesh_workflow_export.obj";

		sfm::scene::PointToMeshBuildOptions options{};
		options.projection = sfm::scene::PointMeshProjectionPlane::XZ;
		options.maximum_projected_points = 3u;

		sfm::app::SfmSandboxMeshWorkflow workflow;
		workflow.configure(options, export_path);
		workflow.request_build(make_square_cloud());
		require(!workflow.build_in_progress(), "oversized workflow build should be refused synchronously");
		require(!workflow.build().succeeded, "refused workflow build must not report success");
		require(workflow.build().input_point_count == 4u, "refused workflow build should report input count");

		workflow.options().maximum_projected_points = 10u;
		workflow.request_build(make_square_cloud());
		require(workflow.build_in_progress(), "accepted workflow build should run asynchronously");
		sfm::scene::PointToMeshBuildResult result = wait_for_build(workflow);
		require(result.succeeded, "accepted workflow build should succeed");
		workflow.record_build_success(std::move(result));
		require(workflow.build_count() == 1, "successful workflow build count mismatch");
		require(workflow.build().mesh.triangle_count() == 2u, "workflow build should produce square triangles");

		workflow.export_surface(workflow.build().mesh);
		require(workflow.export_result().succeeded, "workflow should export active surface");
		require(workflow.export_count() == 1, "successful workflow export count mismatch");
		std::filesystem::remove(export_path);

		return 0;
	} catch (std::exception const& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}
}
