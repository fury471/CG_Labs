#pragma once

#include <glad/gl.h>

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace sfm::gfx
{

struct GpuTimedPass final
{
	std::string name{};
	double milliseconds{ 0.0 };
};

struct GpuProfilerSnapshot final
{
	bool supported{ false };
	std::string message{};
	std::vector<GpuTimedPass> passes{};
	std::size_t pending_frame_count{ 0u };
};

class GpuTimerQuery final
{
public:
	GpuTimerQuery() noexcept = default;
	explicit GpuTimerQuery(std::string_view debug_label) noexcept;
	~GpuTimerQuery() noexcept;

	GpuTimerQuery(GpuTimerQuery const&) = delete;
	GpuTimerQuery& operator=(GpuTimerQuery const&) = delete;
	GpuTimerQuery(GpuTimerQuery&& other) noexcept;
	GpuTimerQuery& operator=(GpuTimerQuery&& other) noexcept;

	[[nodiscard]] GLuint id() const noexcept { return m_id; }
	[[nodiscard]] explicit operator bool() const noexcept { return m_id != 0u; }

	[[nodiscard]] bool begin() noexcept;
	void end() noexcept;
	[[nodiscard]] bool result_available() const noexcept;
	[[nodiscard]] std::uint64_t elapsed_nanoseconds() const noexcept;
	void reset() noexcept;

private:
	GLuint m_id{ 0u };
	bool m_active{ false };
	bool m_ended{ false };
};

class GpuFrameProfiler final
{
public:
	class Scope final
	{
	public:
		Scope() noexcept = default;
		Scope(GpuFrameProfiler& profiler, std::string_view name);
		~Scope() noexcept;

		Scope(Scope const&) = delete;
		Scope& operator=(Scope const&) = delete;
		Scope(Scope&& other) noexcept;
		Scope& operator=(Scope&&) = delete;

	private:
		GpuFrameProfiler* m_profiler{ nullptr };
		std::string m_name{};
		GpuTimerQuery m_query{};
	};

	void begin_frame();
	void end_frame();
	[[nodiscard]] Scope scope(std::string_view name);
	[[nodiscard]] GpuProfilerSnapshot snapshot() const;

private:
	struct PendingPass final
	{
		std::string name{};
		GpuTimerQuery query{};
	};

	struct PendingFrame final
	{
		unsigned long long frame_index{ 0u };
		std::vector<PendingPass> passes{};
	};

	void initialise_if_needed() noexcept;
	void collect_completed_frames();
	void finish_scope(std::string name, GpuTimerQuery query) noexcept;

	bool m_initialised{ false };
	bool m_supported{ false };
	bool m_scope_active{ false };
	unsigned long long m_frame_index{ 0u };
	std::string m_message{ "GPU timing has not been initialised yet" };
	std::vector<PendingPass> m_recording{};
	std::deque<PendingFrame> m_pending_frames{};
	std::vector<GpuTimedPass> m_last_completed_passes{};
};

} // namespace sfm::gfx
