#pragma once

#include "SfmSandboxMeshWorkflow.hpp"

namespace sfm::app
{

struct MeshBuilderPanelActions final
{
	bool build_requested{ false };
	bool export_requested{ false };
};

[[nodiscard]] MeshBuilderPanelActions draw_mesh_builder_panel(SfmSandboxMeshWorkflow& workflow);

} // namespace sfm::app
