#include "SfmSandboxMeshWorkflow.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>
#include <utility>

namespace sfm::app
{
namespace
{

void copy_path_to_buffer(std::array<char, 512>& buffer, std::filesystem::path const& path)
{
	std::string const text = path.string();
	buffer.fill('\0');
	std::size_t const count = std::min(text.size(), buffer.size() - 1u);
	std::copy_n(text.data(), count, buffer.data());
}

std::string build_export_comment(sfm::scene::PointToMeshBuildResult const& build)
{
	if (!build.succeeded)
		return "exported from SfmSandbox active surface";

	std::ostringstream stream;
	stream << "exported from SfmSandbox generated surface\n";
	stream << "algorithm: " << build.algorithm << '\n';
	stream << "projection: " << sfm::scene::projection_plane_label(build.resolved_projection) << '\n';
	stream << "input_points: " << build.input_point_count << '\n';
	stream << "unique_projected_points: " << build.unique_point_count << '\n';
	stream << "duplicate_projected_points: " << build.duplicate_point_count << '\n';
	stream << "invalid_input_points: " << build.invalid_point_count << '\n';
	stream << "rejected_triangles: " << build.rejected_triangle_count;
	return stream.str();
}

} // namespace

void SfmSandboxMeshWorkflow::configure(sfm::scene::PointToMeshBuildOptions options, std::filesystem::path const& export_path)
{
	m_options = options;
	copy_path_to_buffer(m_export_path_buffer, export_path);
}

void SfmSandboxMeshWorkflow::request_build(sfm::scene::PointCloud const& point_cloud)
{
	if (m_build_in_progress) {
		m_last_status = "Mesh build is already running";
		return;
	}

	std::size_t const maximum_projected_points = std::max<std::size_t>(m_options.maximum_projected_points, 3u);
	if (point_cloud.size() > maximum_projected_points) {
		m_build = {};
		m_build.input_point_count = point_cloud.size();
		m_build.messages.push_back("Interactive mesh build refused: " + std::to_string(point_cloud.size()) +
		                           " input points exceed the current max point limit of " +
		                           std::to_string(maximum_projected_points));
		m_build.messages.push_back("Increase Max projected points only for small controlled data, or downsample/filter first");
		m_last_status = "Mesh build refused; point cloud is above safety limit";
		return;
	}

	sfm::scene::PointCloud point_cloud_copy = point_cloud;
	sfm::scene::PointToMeshBuildOptions options = m_options;
	m_build = {};
	m_export_result = {};
	m_last_status = "Mesh build running in background";
	m_build_in_progress = true;
	m_pending_build = std::async(std::launch::async, [point_cloud = std::move(point_cloud_copy), options]() {
		return sfm::scene::build_surface_mesh_from_point_cloud(point_cloud, options);
	});
}

std::optional<sfm::scene::PointToMeshBuildResult> SfmSandboxMeshWorkflow::poll_completed_build()
{
	if (!m_build_in_progress || !m_pending_build.valid())
		return std::nullopt;
	if (m_pending_build.wait_for(std::chrono::seconds{ 0 }) != std::future_status::ready)
		return std::nullopt;

	m_build_in_progress = false;
	return m_pending_build.get();
}

void SfmSandboxMeshWorkflow::record_build_failure(sfm::scene::PointToMeshBuildResult build_result)
{
	m_build = std::move(build_result);
	m_last_status = "Mesh build failed; previous surface kept";
}

void SfmSandboxMeshWorkflow::record_upload_failure(sfm::scene::PointToMeshBuildResult build_result)
{
	m_build = std::move(build_result);
	m_last_status = "Mesh build failed during GPU upload; previous surface kept";
}

void SfmSandboxMeshWorkflow::record_build_success(sfm::scene::PointToMeshBuildResult build_result)
{
	m_build = std::move(build_result);
	m_last_status = "Mesh build succeeded";
	++m_build_count;
}

void SfmSandboxMeshWorkflow::export_surface(sfm::scene::SurfaceMesh const& surface)
{
	std::filesystem::path const export_path{ std::string{ m_export_path_buffer.data() } };
	m_export_result = sfm::scene::export_surface_mesh_as_obj(surface, export_path, build_export_comment(m_build));
	if (m_export_result.succeeded) {
		m_last_status = "Surface export succeeded";
		++m_export_count;
	} else {
		m_last_status = "Surface export failed";
	}
}

} // namespace sfm::app
