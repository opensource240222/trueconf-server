#pragma once

#include "FrameFilterLib/Base/Node.h"
#include "FrameFilterLib/Base/FrameMetadata.h"
#include "FrameFilterLib/Base/FilterFormat.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/SharedBuffer.h"

#include <atomic>
#include <vector>

namespace ffl {
	struct FilterCommand;
	class AbstractSink;

	class AbstractSource : virtual public Node
	{
	public:
		AbstractSource();
		virtual ~AbstractSource();

		bool SetChainID(unsigned value = 0, bool replace = false) override;
		void EnableTrace(const std::weak_ptr<TraceLog>& log) override;

		// If source is frozen it won't generate any frames and will ignore commands.
		void Freeze(bool value);

		// remove sink from subscribers list.
		// when no one subscriber is left, source will be stopped
		void UnregisterSink(const std::shared_ptr<AbstractSink>& sink);

		// search for compatible (the same) sink, or append new one.
		// returns found (or appended) sink.
		template<class T>
		std::shared_ptr<T> RegisterSinkOrGetCompatible(const std::shared_ptr<T>& sink)
		{
			return std::dynamic_pointer_cast<T>(RegisterSinkOrGetCompatibleImpl(std::static_pointer_cast<AbstractSink>(sink)));
		}

		// control data source. e.g keyframe.
		// Returns true if command was handled (and "handled" might mean "deliberatly ignored").
		// Overloads must call base version first and stop if command was handled by it.
		virtual bool ProcessCommand(const FilterCommand& cmd);

		// Returns current source format. Only valid to call from AbstractSink::PutFrame of a connected sink.
		const FilterFormat& GetFormat() const;

	protected:
		// Send frame to all subscribers, notifying about format change if necessary.
		void SendFrameToSubscribers(vs::SharedBuffer&& buffer, FrameMetadata md);

		// Set frame format for the next send.
		void SetFormat(FilterFormat&& format);
		void SetFormat(const FilterFormat& format)
		{
			SetFormat(FilterFormat(format));
		}
		// Derived class is responsible for serializing its calls to SendFrameToSubscribers() and SetFormat().

		virtual void NotifySourceUnused();
		virtual void NotifyNewSink(const std::shared_ptr<AbstractSink>& sink);

		std::shared_ptr<AbstractSource> shared_from_this()
		{
			return std::shared_ptr<AbstractSource>(Node::shared_from_this(), this);
		}

		std::shared_ptr<AbstractSource const> shared_from_this() const
		{
			return std::shared_ptr<AbstractSource const>(Node::shared_from_this(), this);
		}

	private:
		std::shared_ptr<AbstractSink> RegisterSinkOrGetCompatibleImpl(std::shared_ptr<AbstractSink>&& sink);

		using subscriber_list_t = std::vector<std::shared_ptr<AbstractSink>>;
		vs::atomic_shared_ptr<const subscriber_list_t> m_subscribers;
		FilterFormat m_current_format;
		bool m_format_changed;
		std::atomic<bool> m_frozen;
	};
}



