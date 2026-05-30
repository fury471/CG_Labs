#pragma once

#include <string_view>

namespace sfm::gfx
{

/// Minimal renderer-facing material categories used by the M15 submission path.
///
/// This is deliberately small: it names the shader/material intent without
/// introducing texture binding, PBR parameters or a general asset system before
/// later milestones require them.
enum class MaterialKind
{
	LineColour,
	PointCloud,
};

struct MaterialDescriptor final
{
	MaterialKind kind{ MaterialKind::LineColour };
	std::string_view debug_name{};
};

} // namespace sfm::gfx
