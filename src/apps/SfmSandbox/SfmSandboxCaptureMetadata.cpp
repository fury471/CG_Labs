#include "SfmSandboxCaptureMetadata.hpp"

#include <fstream>

namespace sfm::app
{

bool write_baseline_capture_metadata(std::filesystem::path const& metadata_path,
                                     BaselineCaptureMetadata const& metadata,
                                     std::string& error_message)
{
	std::ofstream metadata_file{ metadata_path };
	if (!metadata_file) {
		error_message = "could not open metadata output '" + metadata_path.string() + "'";
		return false;
	}

	metadata_file << "SfM Visualization Sandbox visual baseline\n";
#ifdef NDEBUG
	metadata_file << "build_mode = Release-like\n";
#else
	metadata_file << "build_mode = Debug-like\n";
#endif
	metadata_file << "framebuffer = " << metadata.framebuffer_width << " x " << metadata.framebuffer_height << '\n';
	metadata_file << "project_manifest = " << metadata.project_manifest.string() << '\n';
	metadata_file << "project_load_status = " << (metadata.project_loaded ? "loaded" : "sample fallback") << '\n';
	metadata_file << "point_cloud = " << metadata.point_cloud_source << '\n';
	metadata_file << "point_count = " << metadata.point_count << '\n';
	metadata_file << "camera_poses = " << metadata.camera_pose_source << '\n';
	metadata_file << "camera_pose_count = " << metadata.camera_pose_count << '\n';
	metadata_file << "camera_poses_visible = " << (metadata.camera_poses_visible ? "on" : "off") << '\n';
	metadata_file << "surface = " << metadata.surface_source << '\n';
	metadata_file << "surface_kind = " << metadata.surface_kind << '\n';
	metadata_file << "surface_visible = " << (metadata.surface_visible ? "on" : "off") << '\n';
	metadata_file << "surface_vertices = " << metadata.surface_statistics.vertex_count << '\n';
	metadata_file << "surface_triangles = " << metadata.surface_statistics.triangle_count << '\n';
	metadata_file << "surface_source_format = " << (metadata.surface_source_format.empty() ? "<none>" : metadata.surface_source_format) << '\n';
	if (metadata.generated_mesh_build != nullptr) {
		sfm::scene::PointToMeshBuildResult const& build = *metadata.generated_mesh_build;
		metadata_file << "mesh_builder_algorithm = " << build.algorithm << '\n';
		metadata_file << "mesh_builder_projection = " << sfm::scene::projection_plane_label(build.resolved_projection) << '\n';
		metadata_file << "mesh_builder_input_points = " << build.input_point_count << '\n';
		metadata_file << "mesh_builder_unique_projected_points = " << build.unique_point_count << '\n';
		metadata_file << "mesh_builder_duplicate_projected_points = " << build.duplicate_point_count << '\n';
		metadata_file << "mesh_builder_invalid_input_points = " << build.invalid_point_count << '\n';
		metadata_file << "mesh_builder_rejected_triangles = " << build.rejected_triangle_count << '\n';
	}
	metadata_file << "image = " << metadata.image_source << '\n';
	metadata_file << "image_visible = " << (metadata.image_visible ? "on" : "off") << '\n';
	metadata_file << "image_size = " << metadata.image_width << " x " << metadata.image_height << '\n';
	metadata_file << "marker_count = " << metadata.marker_count << '\n';
	metadata_file << "marker_visible = " << (metadata.marker_visible ? "on" : "off") << '\n';
	metadata_file << "marker_instancing = " << (metadata.marker_instancing ? "on" : "off") << '\n';
	metadata_file << "point_colour_mode = " << metadata.point_colour_mode << '\n';
	metadata_file << "point_size = " << metadata.point_size << '\n';

	if (!metadata_file) {
		error_message = "could not write metadata output '" + metadata_path.string() + "'";
		return false;
	}
	return true;
}

} // namespace sfm::app
