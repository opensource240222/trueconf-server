#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "FrameFilterLib/Base/TraceLog.h"

namespace ffl {
	template <class Sink>
	AbstractFilter<Sink>::AbstractFilter()
		: m_persistent(false)
	{
	}

	template <class Sink>
	void AbstractFilter<Sink>::PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md)
	{
		if (auto log = m_log.lock())
			log->TraceBuffer(this, buffer, md);

		e_processingResult res;
		{
			auto out_md(md);
			res = ProcessFrame(src, buffer, out_md);
			if (res != e_noResult)
				SendFrameToSubscribers(std::move(buffer), out_md);
		}

		while (res == e_moreBuffers)
		{
			auto out_md(md);
			buffer = vs::SharedBuffer();
			res = GetNextBuffer(buffer, out_md);
			if (res != e_noResult)
				SendFrameToSubscribers(std::move(buffer), out_md);
		}
	}

	template <class Sink>
	void AbstractFilter<Sink>::NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (auto log = m_log.lock())
			log->TraceFormat(this, format);

		ProcessFormat(src, format);
	}

	template <class Sink>
	bool AbstractFilter<Sink>::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;
		return this->SendCommandToSources(cmd, false);
	}

	template <class Sink>
	void AbstractFilter<Sink>::Persistent(bool value)
	{
		m_persistent = value;
	}

	template <class Sink>
	auto AbstractFilter<Sink>::GetNextBuffer(vs::SharedBuffer& /*buffer*/, FrameMetadata& /*md*/) -> e_processingResult
	{
		return e_noResult;
	}

	template <class Sink>
	bool AbstractFilter<Sink>::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		SetFormat(format);
		return true;
	}

	template <class Sink>
	void AbstractFilter<Sink>::NotifySourceUnused()
	{
		if (!m_persistent)
			this->Detach();
	}

	template class AbstractFilter<AbstractMultiSourceSink>;
	template class AbstractFilter<AbstractSingleSourceSink>;
}
