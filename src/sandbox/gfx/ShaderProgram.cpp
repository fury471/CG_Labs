#include "ShaderProgram.hpp"

#include "DebugLabel.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

namespace sfm::gfx
{
namespace
{

[[nodiscard]] GLenum to_gl_enum(ShaderStage stage) noexcept
{
	return static_cast<GLenum>(stage);
}

[[nodiscard]] char const* stage_name(ShaderStage stage) noexcept
{
	switch (stage) {
	case ShaderStage::vertex: return "vertex";
	case ShaderStage::fragment: return "fragment";
	case ShaderStage::geometry: return "geometry";
	case ShaderStage::tess_control: return "tessellation-control";
	case ShaderStage::tess_evaluation: return "tessellation-evaluation";
	case ShaderStage::compute: return "compute";
	}
	return "unknown";
}

[[nodiscard]] std::string shader_log(GLuint shader)
{
	GLint length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length <= 1)
		return {};
	std::string log(static_cast<std::size_t>(length), '\0');
	GLsizei written = 0;
	glGetShaderInfoLog(shader, length, &written, log.data());
	log.resize(static_cast<std::size_t>(written));
	return log;
}

[[nodiscard]] std::string program_log(GLuint program)
{
	GLint length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	if (length <= 1)
		return {};
	std::string log(static_cast<std::size_t>(length), '\0');
	GLsizei written = 0;
	glGetProgramInfoLog(program, length, &written, log.data());
	log.resize(static_cast<std::size_t>(written));
	return log;
}

[[nodiscard]] bool source_size_is_supported(std::string_view source) noexcept
{
	return source.size() <= static_cast<std::size_t>(std::numeric_limits<GLint>::max());
}

void detach_and_delete_shaders(GLuint program, std::vector<GLuint>& shaders) noexcept
{
	for (GLuint shader : shaders) {
		if (shader == 0u)
			continue;
		if (program != 0u)
			glDetachShader(program, shader);
		glDeleteShader(shader);
	}
	shaders.clear();
}

[[nodiscard]] bool read_text_file(std::filesystem::path const& path, std::string& content, std::string& message)
{
	std::filesystem::path resolved_path = path;
	std::ifstream file{ resolved_path };
	if (!file) {
		resolved_path = std::filesystem::path{ SFM_SANDBOX_SOURCE_DIR } / path;
		file.open(resolved_path);
	}
	if (!file) {
		message = "could not open shader file '" + path.string() + "'";
		return false;
	}
	std::ostringstream stream;
	stream << file.rdbuf();
	if (file.bad()) {
		message = "failed while reading shader file '" + path.string() + "'";
		return false;
	}
	content = stream.str();
	if (content.empty()) {
		message = "shader file is empty '" + resolved_path.string() + "'";
		return false;
	}
	return true;
}

} // namespace

ShaderProgram::ShaderProgram(std::string_view debug_label) noexcept
	: m_id(glCreateProgram())
{
	detail::label_object(GL_PROGRAM, m_id, debug_label);
}

ShaderProgram::~ShaderProgram() noexcept
{
	reset();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
{
	m_uniform_locations.clear();
	other.m_uniform_locations.clear();
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
		m_uniform_locations.clear();
		other.m_uniform_locations.clear();
	}
	return *this;
}

ShaderProgramBuildResult ShaderProgram::build(std::span<ShaderSource const> sources,
                                              std::string_view debug_label)
{
	ShaderProgramBuildResult result{};
	std::ostringstream log;
	if (sources.empty()) {
		log << "ShaderProgram build failed: no shader sources were provided.\n";
		result.log = log.str();
		return result;
	}
	ShaderProgram program{ debug_label };
	if (!program) {
		log << "ShaderProgram build failed: glCreateProgram returned 0.\n";
		result.log = log.str();
		return result;
	}
	std::vector<GLuint> compiled_shaders;
	compiled_shaders.reserve(sources.size());
	for (ShaderSource const& shader_source : sources) {
		if (!source_size_is_supported(shader_source.source)) {
			log << "ShaderProgram build failed: " << stage_name(shader_source.stage) << " source '" << shader_source.debug_name << "' is too large for OpenGL length parameters.\n";
			detach_and_delete_shaders(program.id(), compiled_shaders);
			program.reset();
			result.log = log.str();
			return result;
		}
		GLuint const shader = glCreateShader(to_gl_enum(shader_source.stage));
		if (shader == 0u) {
			log << "ShaderProgram build failed: glCreateShader returned 0 for " << stage_name(shader_source.stage) << " source '" << shader_source.debug_name << "'.\n";
			detach_and_delete_shaders(program.id(), compiled_shaders);
			program.reset();
			result.log = log.str();
			return result;
		}
		detail::label_object(GL_SHADER, shader, shader_source.debug_name);
		char const* source_data = shader_source.source.data();
		GLint const source_length = static_cast<GLint>(shader_source.source.size());
		glShaderSource(shader, 1, &source_data, &source_length);
		glCompileShader(shader);
		GLint compiled = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled != GL_TRUE) {
			log << "Shader compile failed [" << stage_name(shader_source.stage) << ": " << shader_source.debug_name << "]\n" << shader_log(shader) << '\n';
			glDeleteShader(shader);
			detach_and_delete_shaders(program.id(), compiled_shaders);
			program.reset();
			result.log = log.str();
			return result;
		}
		glAttachShader(program.id(), shader);
		compiled_shaders.push_back(shader);
		log << "Shader compile passed [" << stage_name(shader_source.stage) << ": " << shader_source.debug_name << "]\n";
	}
	glLinkProgram(program.id());
	GLint linked = GL_FALSE;
	glGetProgramiv(program.id(), GL_LINK_STATUS, &linked);
	std::string const link_log = program_log(program.id());
	detach_and_delete_shaders(program.id(), compiled_shaders);
	if (linked != GL_TRUE) {
		log << "Shader link failed [" << debug_label << "]\n" << link_log << '\n';
		program.reset();
		result.log = log.str();
		return result;
	}
	log << "Shader link passed [" << debug_label << "]\n";
	if (!link_log.empty())
		log << link_log << '\n';
	result.succeeded = true;
	result.program = std::move(program);
	result.log = log.str();
	return result;
}

ShaderProgramBuildResult ShaderProgram::build_from_files(std::span<ShaderFileSource const> sources,
                                                         std::string_view debug_label)
{
	ShaderProgramBuildResult result{};
	std::ostringstream log;
	std::vector<std::string> owned_sources;
	std::vector<std::string> owned_debug_names;
	std::vector<ShaderSource> loaded_sources;
	owned_sources.reserve(sources.size());
	owned_debug_names.reserve(sources.size());
	loaded_sources.reserve(sources.size());
	for (ShaderFileSource const& file_source : sources) {
		std::string content;
		std::string message;
		if (!read_text_file(file_source.path, content, message)) {
			log << "ShaderProgram file build failed: " << message << "\n";
			result.log = log.str();
			return result;
		}
		owned_sources.push_back(std::move(content));
		owned_debug_names.push_back(file_source.path.string());
		loaded_sources.push_back(ShaderSource{ file_source.stage, owned_sources.back(), owned_debug_names.back() });
		log << "Loaded shader file [" << stage_name(file_source.stage) << ": " << owned_debug_names.back() << "]\n";
	}
	ShaderProgramBuildResult build_result = build(loaded_sources, debug_label);
	log << build_result.log;
	build_result.log = log.str();
	return build_result;
}

void ShaderProgram::bind() const noexcept
{
	glUseProgram(m_id);
}

void ShaderProgram::reset() noexcept
{
	if (m_id == 0u)
		return;
	glDeleteProgram(m_id);
	m_id = 0u;
	m_uniform_locations.clear();
}

UniformLocation ShaderProgram::uniform_location(std::string_view name) const
{
	std::string key{ name };
	auto const found = m_uniform_locations.find(key);
	if (found != m_uniform_locations.end())
		return found->second;
	UniformLocation const location{ m_id != 0u ? glGetUniformLocation(m_id, key.c_str()) : -1 };
	m_uniform_locations.emplace(std::move(key), location);
	return location;
}

std::size_t ShaderProgram::cached_uniform_count() const noexcept
{
	return m_uniform_locations.size();
}

void ShaderProgram::clear_uniform_cache() const
{
	m_uniform_locations.clear();
}

void ShaderProgram::set_uniform(UniformLocation location, GLint value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniform1i(m_id, location.value, value);
}

void ShaderProgram::set_uniform(UniformLocation location, GLfloat value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniform1f(m_id, location.value, value);
}

void ShaderProgram::set_uniform(UniformLocation location, glm::vec2 const& value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniform2fv(m_id, location.value, 1, glm::value_ptr(value));
}

void ShaderProgram::set_uniform(UniformLocation location, glm::vec3 const& value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniform3fv(m_id, location.value, 1, glm::value_ptr(value));
}

void ShaderProgram::set_uniform(UniformLocation location, glm::vec4 const& value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniform4fv(m_id, location.value, 1, glm::value_ptr(value));
}

void ShaderProgram::set_uniform(UniformLocation location, glm::mat4 const& value) const noexcept
{
	if (!location || m_id == 0u)
		return;
	glProgramUniformMatrix4fv(m_id, location.value, 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace sfm::gfx
