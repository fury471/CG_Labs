#pragma once

#include "SurfaceImport.hpp"

namespace sfm::scene
{

// Compatibility names retained for any experimental M16 code that included
// Mesh.hpp while the final active path settled on SurfaceImport.hpp.
// New code should use SurfaceMesh, SurfaceTriangle and SurfaceStatistics
// directly from SurfaceImport.hpp.
using MeshTriangle = SurfaceTriangle;
using MeshStatistics = SurfaceStatistics;
using Mesh = SurfaceMesh;

[[nodiscard]] inline MeshStatistics statistics_for_mesh(Mesh const& mesh) noexcept
{
	return statistics_for(mesh);
}

} // namespace sfm::scene
