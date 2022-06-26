#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"
#include "FrameFilterLib/Base/AbstractSource.h"
#include "FrameFilterLib/Base/TraceLog.h"

namespace ffl {
	AbstractMultiSourceSink::AbstractMultiSourceSink()
		: m_sources(std::make_shared<source_set_t>())
	{
	}

	void AbstractMultiSourceSink::NotifyNewSource(const std::shared_ptr<AbstractSource>& src)
	{
		auto sources = m_sources.load(std::memory_order_relaxed);
		auto new_sources = std::make_shared<source_set_t>();
		do
		{
			*new_sources = *sources;
			new_sources->insert(src);
		} while (!m_sources.compare_exchange_strong(sources, new_sources, std::memory_order_relaxed));
	}

	void AbstractMultiSourceSink::NotifyRemoveSource(const std::shared_ptr<AbstractSource>& src)
	{
		auto sources = m_sources.load(std::memory_order_relaxed);
		auto new_sources = std::make_shared<source_set_t>();
		do
		{
			*new_sources = *sources;
			new_sources->erase(src);
		} while (!m_sources.compare_exchange_strong(sources, new_sources, std::memory_order_relaxed));
	}

	void AbstractMultiSourceSink::Detach()
	{
		auto sources = m_sources.exchange(std::make_shared<source_set_t>(), std::memory_order_relaxed);
		for (auto& x: *sources)
			x->UnregisterSink(shared_from_this());
	}

	bool AbstractMultiSourceSink::SendCommandToSources(const FilterCommand& cmd, bool log)
	{
		if (log)
			if (const auto log = m_log.lock())
				log->TraceMessage(this, "sending cmd: ", cmd);
		auto sources = m_sources.load(std::memory_order_relaxed);
		bool handled(false);
		for (auto& x: *sources)
			if (x->ProcessCommand(cmd))
				handled = true;
		return handled;
	}
};