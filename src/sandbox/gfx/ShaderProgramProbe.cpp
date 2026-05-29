#include "ShaderProgramProbe.hpp"

#include "ShaderProgram.hpp"

#include <array>
#include <string>
#include <utility>

namespace sfm::gfx
{
namespace
{

constexpr std::string_view kProbeVertexShader = R"glsl(
#version 460 core

uniform mat4 u_model_view_projection;

const vec2 kPositions[3] = vec2[3](
	vec2(-0.5, -0.5),
	vec2( 0.5, -0.5),
	vec2( 0.0,  0.5)
);

void main()
{
	gl_Position = u_model_view_projection * vec4(kPositions[gl_VertexID], 0.0, 1.0);
}
)glsl";

constexpr std::string_view kProbeFragmentShader = R"glsl(
#version 460 core

uniform vec4 u_tint;

layout(location = 0) out vec4 out_colour;

void main()
{
	out_colour = u_tint;
}
)glsl";

void add_message(ShaderProgramProbeResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

} // namespace

ShaderProgramProbeResult run_shader_program_probe()
{
	ShaderProgramProbeResult result{};
	result.passed = true;

	std::array<ShaderSource, 2> const sources{
		ShaderSource{ ShaderStage::vertex, kProbeVertexShader, "Milestone3 probe vertex shader" },
		ShaderSource{ ShaderStage::fragment, kProbeFragmentShader, "Milestone3 probe fragment shader" },
	};

	ShaderProgramBuildResult build = ShaderProgram::build(sources, "Milestone3 shader cache probe");
	if (!build.succeeded || !build.program) {
		result.passed = false;
		add_message(result, "Shader build failed; driver log follows:");
		add_message(result, build.log.empty() ? "<empty shader log>" : build.log);
		return result;
	}
	add_message(result, "Shader compile/link diagnostics succeeded");

	ShaderProgram& program = build.program;
	UniformLocation const mvp = program.uniform_location("u_model_view_projection");
	UniformLocation const tint = program.uniform_location("u_tint");
	UniformLocation const missing = program.uniform_location("u_missing_but_cached");
	if (!mvp || !tint || missing) {
		result.passed = false;
		add_message(result, "Uniform lookup returned unexpected active/missing locations");
		return result;
	}

	std::size_t const cache_after_first_queries = program.cached_uniform_count();
	(void)program.uniform_location("u_model_view_projection");
	(void)program.uniform_location("u_tint");
	(void)program.uniform_location("u_missing_but_cached");
	if (program.cached_uniform_count() != cache_after_first_queries) {
		result.passed = false;
		add_message(result, "Repeated uniform lookups changed the cache size");
		return result;
	}
	add_message(result, "Uniform locations are cached, including missing uniforms");

	program.set_uniform(mvp, glm::mat4{ 1.0f });
	program.set_uniform(tint, glm::vec4{ 0.2f, 0.7f, 1.0f, 1.0f });
	program.set_uniform(missing, 1.0f);
	add_message(result, "Cached uniform writes executed through glProgramUniform* helpers");

	program.clear_uniform_cache();
	if (program.cached_uniform_count() != 0u) {
		result.passed = false;
		add_message(result, "Uniform cache clear did not leave the cache empty");
		return result;
	}
	add_message(result, "Uniform cache clear leaves the program cache empty");

	return result;
}

} // namespace sfm::gfx
