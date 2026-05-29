#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace sfm::gfx
{

/// Result of a lightweight runtime ownership check.
///
/// The probe is not a renderer test and does not validate visual output. Its
/// purpose is narrower: verify that the initial RAII owners can be constructed,
/// moved, and left in safe empty states under a live OpenGL context.
struct OwnershipProbeResult final
{
	bool passed{ false };
	std::vector<std::string> messages{};
};

/// Creates every Milestone 2 GPU owner, transfers ownership by move, and checks
/// the C++-side ownership invariants. Call this only after an OpenGL context is
/// current and before the context is destroyed.
[[nodiscard]] OwnershipProbeResult run_ownership_probe(std::string_view label_prefix) noexcept;

} // namespace sfm::gfx
