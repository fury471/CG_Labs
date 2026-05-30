#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sfm::core
{

struct TimedPass final
{
	std::string name{};
	double milliseconds{ 0.0 };
};

class FrameProfiler final
{
public:
	using Clock = std::chrono::steady_clock;

	void begin_frame()
	{
		m_passes.clear();
		m_frame_start = Clock::now();
	}

	void end_frame()
	{
		m_total_milliseconds = elapsed_since(m_frame_start);
	}

	class Scope final
	{
	public:
		Scope(FrameProfiler& profiler, std::string_view name)
			: m_profiler(&profiler)
			, m_name(name)
			, m_start(Clock::now())
		{
		}

		~Scope()
		{
			if (m_profiler == nullptr)
				return;
			m_profiler->record(m_name, elapsed_since(m_start));
		}

		Scope(Scope const&) = delete;
		Scope& operator=(Scope const&) = delete;
		Scope(Scope&& other) noexcept
			: m_profiler(std::exchange(other.m_profiler, nullptr))
			, m_name(std::move(other.m_name))
			, m_start(other.m_start)
		{
		}
		Scope& operator=(Scope&&) = delete;

	private:
		FrameProfiler* m_profiler{ nullptr };
		std::string m_name{};
		Clock::time_point m_start{};
	};

	[[nodiscard]] Scope scope(std::string_view name) { return Scope{ *this, name }; }
	[[nodiscard]] std::vector<TimedPass> const& passes() const noexcept { return m_passes; }
	[[nodiscard]] double total_milliseconds() const noexcept { return m_total_milliseconds; }

private:
	static double elapsed_since(Clock::time_point start)
	{
		return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
	}

	void record(std::string name, double milliseconds)
	{
		m_passes.push_back(TimedPass{ std::move(name), milliseconds });
	}

	Clock::time_point m_frame_start{};
	std::vector<TimedPass> m_passes{};
	double m_total_milliseconds{ 0.0 };
};

} // namespace sfm::core
