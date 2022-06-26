#include "FrameFilterLib/Base/AbstractSource.h"
#include "FrameFilterLib/Base/AbstractSink.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"

namespace ffl {
	AbstractSource::AbstractSource()
		: m_subscribers(std::make_shared<subscriber_list_t>())
		, m_format_changed(false)
		, m_frozen(false)
	{
	};

	AbstractSource::~AbstractSource()
	{
	};

	bool AbstractSource::SetChainID(unsigned value, bool replace)
	{
		if (!Node::SetChainID(value, replace))
			return false;

		auto subscribers = m_subscribers.load(std::memory_order_relaxed);
		for (auto& x: *subscribers)
			x->SetChainID(m_chain_id, replace);
		return true;
	}

	void AbstractSource::EnableTrace(const std::weak_ptr<TraceLog>& log)
	{
		Node::EnableTrace(log);

		auto subscribers = m_subscribers.load(std::memory_order_relaxed);
		for (auto& x: *subscribers)
			x->EnableTrace(log);
	}

	void AbstractSource::Freeze(bool value)
	{
		m_frozen = value;
	}

	void AbstractSource::UnregisterSink(const std::shared_ptr<AbstractSink>& sink)
	{
		auto subscribers = m_subscribers.load(std::memory_order_relaxed);
		auto new_subscribers = std::make_shared<subscriber_list_t>();
		bool no_subscribers_left;
		do
		{
			new_subscribers->clear();
			new_subscribers->reserve(subscribers->size());
			for (auto& x: *subscribers)
				if (x != sink)
					new_subscribers->push_back(x);
			no_subscribers_left = new_subscribers->empty();
		} while (!m_subscribers.compare_exchange_strong(subscribers, new_subscribers, std::memory_order_relaxed));

		sink->NotifyRemoveSource(shared_from_this());
		if (no_subscribers_left)
			NotifySourceUnused();
	}

	std::shared_ptr<AbstractSink> AbstractSource::RegisterSinkOrGetCompatibleImpl(std::shared_ptr<AbstractSink>&& sink)
	{
		auto subscribers = m_subscribers.load(std::memory_order_relaxed);
		auto new_subscribers = std::make_shared<subscriber_list_t>();

		for (auto& x: *subscribers)
			if (sink->IsCompatibleWith(x.get()))
				return x;

		do
		{
			*new_subscribers = *subscribers;
			new_subscribers->push_back(sink);

		} while (!m_subscribers.compare_exchange_strong(subscribers, new_subscribers, std::memory_order_relaxed));

		sink->NotifyNewSource(shared_from_this());
		if (m_chain_id != 0)
			sink->SetChainID(m_chain_id);
		sink->EnableTrace(m_log);
		if (m_current_format.type != FilterFormat::e_invalid)
			sink->NotifyNewFormat(shared_from_this(), m_current_format);
		NotifyNewSink(sink);
		return std::move(sink);
	}

	bool AbstractSource::ProcessCommand(const FilterCommand& cmd)
	{
		if (auto log = m_log.lock())
			log->TraceCommand(this, cmd);

		return m_frozen;
	}

	void AbstractSource::SendFrameToSubscribers(vs::SharedBuffer&& buffer, FrameMetadata md)
	{
		if (m_frozen)
			return;

		auto subscribers = m_subscribers.load(std::memory_order_relaxed);

		if (m_format_changed)
		{
			for (auto& x: *subscribers)
				x->NotifyNewFormat(shared_from_this(), m_current_format);
			m_format_changed = false;
			// Need to reload subscriber list because it may have changed as a result of a format change
			subscribers = m_subscribers.load(std::memory_order_relaxed);
		}

		if (subscribers->size() == 1)
			subscribers->front()->PutFrame(shared_from_this(), std::move(buffer), md);
		else
			for (auto& x: *subscribers)
				x->PutFrame(shared_from_this(), buffer, md);
	}

	void AbstractSource::SetFormat(FilterFormat&& format)
	{
		if (m_current_format == format)
			return;
		m_current_format = std::move(format);
		m_format_changed = true;
	}

	const FilterFormat& AbstractSource::GetFormat() const
	{
		return m_current_format;
	}

	void AbstractSource::NotifySourceUnused()
	{
	}

	void AbstractSource::NotifyNewSink(const std::shared_ptr<AbstractSink>& /*sink*/)
	{
	}
}