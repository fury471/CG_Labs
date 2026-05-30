#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sfm::gfx
{

enum class ShaderStage : GLenum
{
	vertex = GL_VERTEX_SHADER,
	fragment = GL_FRAGMENT_SHADER,
	geometry = GL_GEOMETRY_SHADER,
	tess_control = GL_TESS_CONTROL_SHADER,
	tess_evaluation = GL_TESS_EVALUATION_SHADER,
	compute = GL_COMPUTE_SHADER,
};

struct ShaderSource final
{
	ShaderStage stage;
	std::string_view source;
	std::string_view debug_name;
};

struct ShaderFileSource final
{
	ShaderStage stage;
	std::filesystem::path path;
};

struct UniformLocation final
{
	GLint value{ -1 };

	[[nodiscard]] explicit operator bool() const noexcept { return value >= 0; }
};

class ShaderProgram;
struct ShaderProgramBuildResult;

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

	[[nodiscard]] static ShaderProgramBuildResult build(std::span<ShaderSource const> sources,
	                                                   std::string_view debug_label);
	[[nodiscard]] static ShaderProgramBuildResult build_from_files(std::span<ShaderFileSource const> sources,
	                                                              std::string_view debug_label);

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	void bind() const noexcept;
	void reset() noexcept;

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

struct ShaderProgramBuildResult final
{
	ShaderProgram program{};
	bool succeeded{ false };
	std::string log{};
};

} // namespace sfm::gfx
