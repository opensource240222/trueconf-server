#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "FrameFilterLib/Base/AbstractSource.h"
#include "FrameFilterLib/Base/TraceLog.h"

namespace ffl {
	AbstractSingleSourceSink::AbstractSingleSourceSink() = default;

	void AbstractSingleSourceSink::NotifyNewSource(const std::shared_ptr<AbstractSource>& src)
	{
		auto source = m_source.exchange(src, std::memory_order_relaxed);
		if (source && source != src)
			source->UnregisterSink(shared_from_this());
	}

	void AbstractSingleSourceSink::NotifyRemoveSource(const std::shared_ptr<AbstractSource>& src)
	{
		auto source = src;
		if (!m_source.compare_exchange_strong(source, nullptr, std::memory_order_relaxed))
			return;
		if (source)
			source->UnregisterSink(shared_from_this());
	}

	void AbstractSingleSourceSink::Detach()
	{
		auto source = m_source.exchange(nullptr, std::memory_order_relaxed);
		if (source)
			source->UnregisterSink(shared_from_this());
	}

	bool AbstractSingleSourceSink::SendCommandToSources(const FilterCommand& cmd, bool log)
	{
		if (log)
			if (const auto log = m_log.lock())
				log->TraceMessage(this, "sending cmd: ", cmd);
		auto source = m_source.load(std::memory_order_relaxed);
		if (source)
			return source->ProcessCommand(cmd);
		return false;
	}
};