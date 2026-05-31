#pragma once

#include "sandbox/scene/PointCloudMesher.hpp"
#include "sandbox/scene/SurfaceExport.hpp"

#include <array>
#include <filesystem>
#include <future>
#include <optional>
#include <string>

namespace sfm::app
{

class SfmSandboxMeshWorkflow final
{
public:
	void configure(sfm::scene::PointToMeshBuildOptions options, std::filesystem::path const& export_path);

	[[nodiscard]] sfm::scene::PointToMeshBuildOptions& options() noexcept { return m_options; }
	[[nodiscard]] sfm::scene::PointToMeshBuildOptions const& options() const noexcept { return m_options; }
	[[nodiscard]] sfm::scene::PointToMeshBuildResult const& build() const noexcept { return m_build; }
	[[nodiscard]] sfm::scene::SurfaceExportResult const& export_result() const noexcept { return m_export_result; }
	[[nodiscard]] std::array<char, 512>& export_path_buffer() noexcept { return m_export_path_buffer; }
	[[nodiscard]] std::array<char, 512> const& export_path_buffer() const noexcept { return m_export_path_buffer; }
	[[nodiscard]] std::string const& last_status() const noexcept { return m_last_status; }
	[[nodiscard]] int build_count() const noexcept { return m_build_count; }
	[[nodiscard]] int export_count() const noexcept { return m_export_count; }
	[[nodiscard]] bool build_in_progress() const noexcept { return m_build_in_progress; }

	void request_build(sfm::scene::PointCloud const& point_cloud);
	[[nodiscard]] std::optional<sfm::scene::PointToMeshBuildResult> poll_completed_build();
	void record_build_failure(sfm::scene::PointToMeshBuildResult build_result);
	void record_upload_failure(sfm::scene::PointToMeshBuildResult build_result);
	void record_build_success(sfm::scene::PointToMeshBuildResult build_result);
	void export_surface(sfm::scene::SurfaceMesh const& surface);

private:
	sfm::scene::PointToMeshBuildOptions m_options{};
	sfm::scene::PointToMeshBuildResult m_build{};
	sfm::scene::SurfaceExportResult m_export_result{};
	std::future<sfm::scene::PointToMeshBuildResult> m_pending_build{};
	std::array<char, 512> m_export_path_buffer{};
	std::string m_last_status{ "No mesh build requested" };
	int m_build_count{ 0 };
	int m_export_count{ 0 };
	bool m_build_in_progress{ false };
};

} // namespace sfm::app
