#include "GpuProfiler.hpp"

#include "DebugLabel.hpp"

#include <algorithm>
#include <utility>

namespace sfm::gfx
{
namespace
{

constexpr std::size_t kMaxPendingGpuFrames = 4u;

[[nodiscard]] double nanoseconds_to_milliseconds(std::uint64_t nanoseconds) noexcept
{
	return static_cast<double>(nanoseconds) / 1'000'000.0;
}

} // namespace

GpuTimerQuery::GpuTimerQuery(std::string_view debug_label) noexcept
{
	if (glCreateQueries != nullptr) {
		glCreateQueries(GL_TIME_ELAPSED, 1, &m_id);
		detail::label_object(GL_QUERY, m_id, debug_label);
		return;
	}
	glGenQueries(1, &m_id);
}

GpuTimerQuery::~GpuTimerQuery() noexcept
{
	reset();
}

GpuTimerQuery::GpuTimerQuery(GpuTimerQuery&& other) noexcept
	: m_id(std::exchange(other.m_id, 0u))
	, m_active(std::exchange(other.m_active, false))
	, m_ended(std::exchange(other.m_ended, false))
{
}

GpuTimerQuery& GpuTimerQuery::operator=(GpuTimerQuery&& other) noexcept
{
	if (this != &other) {
		reset();
		m_id = std::exchange(other.m_id, 0u);
		m_active = std::exchange(other.m_active, false);
		m_ended = std::exchange(other.m_ended, false);
	}
	return *this;
}

bool GpuTimerQuery::begin() noexcept
{
	if (m_id == 0u || m_active)
		return false;
	glBeginQuery(GL_TIME_ELAPSED, m_id);
	GLenum const error = glGetError();
	if (error != GL_NO_ERROR) {
		m_active = false;
		m_ended = false;
		return false;
	}
	m_active = true;
	m_ended = false;
	return true;
}

void GpuTimerQuery::end() noexcept
{
	if (!m_active)
		return;
	glEndQuery(GL_TIME_ELAPSED);
	m_active = false;
	m_ended = true;
}

bool GpuTimerQuery::result_available() const noexcept
{
	if (m_id == 0u || !m_ended)
		return false;
	GLint available = GL_FALSE;
	glGetQueryObjectiv(m_id, GL_QUERY_RESULT_AVAILABLE, &available);
	return available == GL_TRUE;
}

std::uint64_t GpuTimerQuery::elapsed_nanoseconds() const noexcept
{
	if (m_id == 0u || !m_ended)
		return 0u;
	GLuint64 elapsed = 0u;
	glGetQueryObjectui64v(m_id, GL_QUERY_RESULT, &elapsed);
	return static_cast<std::uint64_t>(elapsed);
}

void GpuTimerQuery::reset() noexcept
{
	if (m_active)
		end();
	if (m_id == 0u)
		return;
	glDeleteQueries(1, &m_id);
	m_id = 0u;
	m_ended = false;
}

GpuFrameProfiler::Scope::Scope(GpuFrameProfiler& profiler, std::string_view name)
	: m_profiler(&profiler)
	, m_name(name)
	, m_query(name)
{
	if (!m_query.begin()) {
		m_profiler->m_scope_active = false;
		m_profiler->m_message = "GPU timer query skipped: glBeginQuery failed";
		m_profiler = nullptr;
	}
}

GpuFrameProfiler::Scope::~Scope() noexcept
{
	if (m_profiler == nullptr)
		return;
	m_profiler->finish_scope(std::move(m_name), std::move(m_query));
}

GpuFrameProfiler::Scope::Scope(Scope&& other) noexcept
	: m_profiler(std::exchange(other.m_profiler, nullptr))
	, m_name(std::move(other.m_name))
	, m_query(std::move(other.m_query))
{
}

void GpuFrameProfiler::begin_frame()
{
	initialise_if_needed();
	collect_completed_frames();
	m_recording.clear();
	m_recording.reserve(8u);
	m_scope_active = false;
	++m_frame_index;
}

void GpuFrameProfiler::end_frame()
{
	if (!m_supported)
		return;
	if (m_recording.empty()) {
		m_message = "GPU timer queries are available, but no GPU scopes were recorded this frame";
		return;
	}

	PendingFrame frame{};
	frame.frame_index = m_frame_index;
	frame.passes = std::move(m_recording);
	m_pending_frames.push_back(std::move(frame));
	if (m_pending_frames.size() > kMaxPendingGpuFrames) {
		m_pending_frames.pop_front();
		m_message = "Dropped stale GPU timing frame because query results did not become available quickly enough";
	}
}

GpuFrameProfiler::Scope GpuFrameProfiler::scope(std::string_view name)
{
	if (!m_supported)
		return {};
	if (m_scope_active) {
		m_message = "GPU timer query skipped: nested timer scopes are not supported";
		return {};
	}
	m_scope_active = true;
	return Scope{ *this, name };
}

GpuProfilerSnapshot GpuFrameProfiler::snapshot() const
{
	return GpuProfilerSnapshot{ m_supported, m_message, m_last_completed_passes, m_pending_frames.size() };
}

void GpuFrameProfiler::initialise_if_needed() noexcept
{
	if (m_initialised)
		return;
	m_initialised = true;

	if (glGenQueries == nullptr || glBeginQuery == nullptr || glEndQuery == nullptr ||
	    glGetQueryiv == nullptr || glGetQueryObjectiv == nullptr || glGetQueryObjectui64v == nullptr) {
		m_supported = false;
		m_message = "GPU timer queries are unavailable: required OpenGL query functions are missing";
		return;
	}

	GLint counter_bits = 0;
	glGetQueryiv(GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS, &counter_bits);
	if (counter_bits <= 0) {
		m_supported = false;
		m_message = "GPU timer queries are unavailable: GL_TIME_ELAPSED reports zero counter bits";
		return;
	}

	m_supported = true;
	m_message = "GPU timer queries are available with " + std::to_string(counter_bits) + " counter bits";
}

void GpuFrameProfiler::collect_completed_frames()
{
	if (!m_supported || m_pending_frames.empty())
		return;

	bool completed_any = false;
	while (!m_pending_frames.empty()) {
		PendingFrame& frame = m_pending_frames.front();
		bool const complete = std::all_of(frame.passes.begin(), frame.passes.end(), [](PendingPass const& pass) {
			return pass.query.result_available();
		});
		if (!complete)
			break;

		std::vector<GpuTimedPass> completed_passes;
		completed_passes.reserve(frame.passes.size());
		for (PendingPass const& pass : frame.passes) {
			completed_passes.push_back(GpuTimedPass{
				pass.name,
				nanoseconds_to_milliseconds(pass.query.elapsed_nanoseconds())
			});
		}
		m_last_completed_passes = std::move(completed_passes);
		m_message = "GPU timing frame " + std::to_string(frame.frame_index) + " completed";
		m_pending_frames.pop_front();
		completed_any = true;
	}

	if (!completed_any && !m_pending_frames.empty())
		m_message = "Waiting for " + std::to_string(m_pending_frames.size()) + " GPU timing frame(s) to complete";
}

void GpuFrameProfiler::finish_scope(std::string name, GpuTimerQuery query) noexcept
{
	query.end();
	m_scope_active = false;
	try {
		m_recording.push_back(PendingPass{ std::move(name), std::move(query) });
	} catch (...) {
		m_message = "GPU timer query skipped: could not store pass result";
	}
}

} // namespace sfm::gfx
