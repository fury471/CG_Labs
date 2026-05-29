#include "OwnershipProbe.hpp"

#include "Buffer.hpp"
#include "Framebuffer.hpp"
#include "Sampler.hpp"
#include "ShaderProgram.hpp"
#include "Texture2D.hpp"
#include "VertexArray.hpp"

#include <glad/gl.h>

#include <string>
#include <type_traits>
#include <utility>

namespace sfm::gfx
{
namespace
{

template <typename Resource>
void check_type_contract(std::string_view resource_name, OwnershipProbeResult& result)
{
	// Compile-time checks document the ownership contract directly next to the
	// runtime probe. If a future edit accidentally makes a GPU owner copyable,
	// the sandbox stops compiling instead of becoming double-delete-prone.
	static_assert(!std::is_copy_constructible_v<Resource>);
	static_assert(!std::is_copy_assignable_v<Resource>);
	static_assert(std::is_nothrow_move_constructible_v<Resource>);
	static_assert(std::is_nothrow_move_assignable_v<Resource>);
	static_assert(std::is_nothrow_destructible_v<Resource>);

	result.messages.emplace_back(std::string(resource_name) + ": compile-time move-only contract is valid");
}

template <typename Resource>
void check_move_runtime(std::string_view label_prefix, std::string_view resource_name, OwnershipProbeResult& result)
{
	std::string const label = std::string(label_prefix) + " " + std::string(resource_name);

	Resource source{ label };
	if (!source) {
		result.passed = false;
		result.messages.emplace_back(std::string(resource_name) + ": creation returned an empty OpenGL name");
		return;
	}

	GLuint const original_id = source.id();
	Resource move_constructed{ std::move(source) };
	if (source || !move_constructed || move_constructed.id() != original_id) {
		result.passed = false;
		result.messages.emplace_back(std::string(resource_name) + ": move construction failed ownership transfer");
		return;
	}

	Resource move_assigned;
	move_assigned = std::move(move_constructed);
	if (move_constructed || !move_assigned || move_assigned.id() != original_id) {
		result.passed = false;
		result.messages.emplace_back(std::string(resource_name) + ": move assignment failed ownership transfer");
		return;
	}

	move_assigned.reset();
	if (move_assigned) {
		result.passed = false;
		result.messages.emplace_back(std::string(resource_name) + ": reset did not leave the owner empty");
		return;
	}

	result.messages.emplace_back(std::string(resource_name) + ": create, move and reset invariants passed");
}

template <typename Resource>
void check_resource(std::string_view label_prefix, std::string_view resource_name, OwnershipProbeResult& result)
{
	check_type_contract<Resource>(resource_name, result);
	check_move_runtime<Resource>(label_prefix, resource_name, result);
}

} // namespace

OwnershipProbeResult run_ownership_probe(std::string_view label_prefix)
{
	OwnershipProbeResult result{};
	result.passed = true;

	// These checks intentionally create real OpenGL objects. The function must be
	// called while a context is current; all owners are destroyed before return.
	check_resource<Buffer>(label_prefix, "Buffer", result);
	check_resource<VertexArray>(label_prefix, "VertexArray", result);
	check_resource<Texture2D>(label_prefix, "Texture2D", result);
	check_resource<Sampler>(label_prefix, "Sampler", result);
	check_resource<ShaderProgram>(label_prefix, "ShaderProgram", result);
	check_resource<Framebuffer>(label_prefix, "Framebuffer", result);

	return result;
}

} // namespace sfm::gfx
