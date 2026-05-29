#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sfm::gfx
{

/// Shader stage types accepted by the sandbox program builder.
///
/// The enum values deliberately match OpenGL constants so translation at the API
/// boundary remains explicit and cheap, while callers avoid passing arbitrary
/// GLenum values into the high-level shader interface.
enum class ShaderStage : GLenum
{
	vertex = GL_VERTEX_SHADER,
	fragment = GL_FRAGMENT_SHADER,
	geometry = GL_GEOMETRY_SHADER,
	tess_control = GL_TESS_CONTROL_SHADER,
	tess_evaluation = GL_TESS_EVALUATION_SHADER,
	compute = GL_COMPUTE_SHADER,
};

/// One shader source unit supplied to the linker.
///
/// `debug_name` is used only in diagnostics and debug object labels; it is not a
/// filesystem contract. Later asset-loading code can build these records from
/// real files without changing the program interface.
struct ShaderSource final
{
	ShaderStage stage;
	std::string_view source;
	std::string_view debug_name;
};

/// Cached location of a uniform inside a linked program.
///
/// OpenGL returns -1 when a uniform is absent or optimized away. Keeping that as
/// a valid cached state avoids repeating string lookups for missing uniforms and
/// lets uniform setters silently ignore inactive values, matching OpenGL usage.
struct UniformLocation final
{
	GLint value{ -1 };

	[[nodiscard]] explicit operator bool() const noexcept { return value >= 0; }
};

class ShaderProgram;
struct ShaderProgramBuildResult;

/// Owns one OpenGL program object and its cached binding metadata.
///
/// A linked program is still move-only GPU state, but Milestone 3 adds the
/// interface needed to use it safely: structured build diagnostics and cached
/// uniform locations. Rendering code should resolve uniform locations during
/// setup and then keep `UniformLocation` handles for repeated updates.
class ShaderProgram final
{
public:
	ShaderProgram() noexcept = default;
	explicit ShaderProgram(std::string_view debug_label) noexcept;
	~ShaderProgram() noexcept;

	ShaderProgram(ShaderProgram const&) = delete;
	ShaderProgram& operator=(ShaderProgram const&) = delete;
	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;

	/// Compiles all shader stages, links them into a program and returns the log.
	/// Failed builds return an empty `program` and `succeeded == false`.
	[[nodiscard]] static ShaderProgramBuildResult build(std::span<ShaderSource const> sources,
	                                                   std::string_view debug_label);

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	/// Bind this program for later draw calls. Uniform setters below do not need
	/// this binding because they use glProgramUniform* DSA entry points.
	void bind() const noexcept;

	void reset() noexcept;

	/// Returns a cached uniform location. The first request calls OpenGL; repeated
	/// requests for the same name reuse the stored value, including -1.
	[[nodiscard]] UniformLocation uniform_location(std::string_view name) const;
	[[nodiscard]] std::size_t cached_uniform_count() const noexcept;
	void clear_uniform_cache() const;

	void set_uniform(UniformLocation location, GLint value) const noexcept;
	void set_uniform(UniformLocation location, GLfloat value) const noexcept;
	void set_uniform(UniformLocation location, glm::vec2 const& value) const noexcept;
	void set_uniform(UniformLocation location, glm::vec3 const& value) const noexcept;
	void set_uniform(UniformLocation location, glm::vec4 const& value) const noexcept;
	void set_uniform(UniformLocation location, glm::mat4 const& value) const noexcept;

private:
	GLuint m_id{ 0u };
	mutable std::unordered_map<std::string, UniformLocation> m_uniform_locations{};
};

/// Result of compiling and linking a shader program.
///
/// The result is intentionally not an exception-only interface: shader compile
/// errors are expected development feedback, and returning the driver log makes
/// them visible in the sandbox UI and milestone probes.
struct ShaderProgramBuildResult final
{
	ShaderProgram program{};
	bool succeeded{ false };
	std::string log{};
};

} // namespace sfm::gfx
