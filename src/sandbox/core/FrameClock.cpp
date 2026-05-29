#include "FrameClock.hpp"

#include <limits>

namespace sfm::core
{

FrameClock::FrameClock() noexcept
	: m_last_tick(clock_type::now())
{
}

FrameTiming FrameClock::tick() noexcept
{
	auto const now = clock_type::now();
	std::chrono::duration<float> const delta = now - m_last_tick;
	m_last_tick = now;
	++m_frame_index;

	float const seconds = delta.count();
	float const fps = seconds > std::numeric_limits<float>::epsilon() ? 1.0f / seconds : 0.0f;
	return FrameTiming{ seconds, seconds * 1000.0f, fps, m_frame_index };
}

} // namespace sfm::core
