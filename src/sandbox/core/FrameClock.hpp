#pragma once

#include <chrono>
#include <cstdint>

namespace sfm::core
{

struct FrameTiming final
{
	float delta_seconds{ 0.0f };
	float delta_milliseconds{ 0.0f };
	float frames_per_second{ 0.0f };
	std::uint64_t frame_index{ 0u };
};

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
