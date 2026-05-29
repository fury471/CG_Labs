#pragma once

#include <chrono>
#include <cstdint>

namespace sfm::core
{

/// Per-frame timing values consumed by development UI and future profiling.
///
/// This is deliberately CPU-side wall-clock timing. GPU pass timings will be
/// introduced separately once renderer passes exist, so the two measurements do
/// not become mixed or misleading.
struct FrameTiming final
{
	float delta_seconds{ 0.0f };
	float delta_milliseconds{ 0.0f };
	float frames_per_second{ 0.0f };
	std::uint64_t frame_index{ 0u };
};

/// Small monotonic frame timer for the sandbox application loop.
///
/// `FrameClock` belongs in `sandbox/core` because it has no OpenGL, scene, or UI
/// dependency. Keeping it independent makes it reusable for later profiling and
/// tests without dragging graphics headers into core utilities.
class FrameClock final
{
public:
	FrameClock() noexcept;

	[[nodiscard]] FrameTiming tick() noexcept;

private:
	using clock_type = std::chrono::steady_clock;

	clock_type::time_point m_last_tick;
	std::uint64_t m_frame_index{ 0u };
};

} // namespace sfm::core
