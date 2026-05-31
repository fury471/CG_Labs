#pragma once

#include "sandbox/core/FrameProfiler.hpp"
#include "sandbox/gfx/ImagePlaneRenderer.hpp"
#include "sandbox/gfx/InstancedMarkerRenderer.hpp"
#include "sandbox/gfx/Renderer.hpp"
#include "sandbox/gfx/RenderTarget.hpp"
#include "sandbox/gfx/SurfaceRenderer.hpp"
#include "sandbox/scene/CameraPose.hpp"
#include "sandbox/scene/ImageImport.hpp"
#include "sandbox/scene/PointCloud.hpp"
#include "sandbox/scene/SurfaceImport.hpp"

#include <array>

namespace sfm::app
{

bool draw_path_input(char const* label, std::array<char, 512>& buffer);
bool draw_optional_visibility_checkbox(char const* label, bool& visible, bool available);
void draw_point_cloud_statistics(sfm::scene::PointCloudStatistics const& statistics);
void draw_surface_statistics(sfm::scene::SurfaceStatistics const& statistics);
void draw_image_statistics(sfm::scene::ImageResource const& image);
void draw_render_target_status(sfm::gfx::RenderTargetStatus const& status);
void draw_frame_profiler(sfm::core::FrameProfiler const& profiler);
void draw_marker_stress_statistics(sfm::gfx::MarkerStressStats const& statistics);
void draw_point_controls(sfm::gfx::PointCloudRenderSettings& settings);
void draw_camera_pose_metadata(sfm::scene::CameraPoseSet const& camera_poses, int& selected_pose);
void draw_renderer_frame_statistics(sfm::gfx::RendererFrameStatistics const& statistics,
                                    sfm::gfx::SurfaceRendererStats const& surface_statistics,
                                    sfm::gfx::ImagePlaneRendererStats const& image_statistics,
                                    sfm::gfx::MarkerStressStats const& marker_statistics);

} // namespace sfm::app
