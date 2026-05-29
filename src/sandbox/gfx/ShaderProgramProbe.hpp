#pragma once

#include <string>
#include <vector>

namespace sfm::gfx
{

/// Result of the Milestone 3 shader-interface validation.
///
/// The probe checks the behavior this milestone promises: compile/link
/// diagnostics, uniform-location caching, and uniform writes through cached
/// locations. It deliberately does not draw anything yet.
struct ShaderProgramProbeResult final
{
	bool passed{ false };
	std::vector<std::string> messages{};
};

/// Builds a tiny shader program and verifies the cached-uniform interface.
/// Call only while an OpenGL context is current.
[[nodiscard]] ShaderProgramProbeResult run_shader_program_probe();

} // namespace sfm::gfx
