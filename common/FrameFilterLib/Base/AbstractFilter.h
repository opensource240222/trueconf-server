#pragma once

#include "FrameFilterLib/Base/AbstractSink.h"
#include "FrameFilterLib/Base/AbstractSource.h"

#include <atomic>
#include <type_traits>

namespace ffl {
	template <class Sink>
	class AbstractFilter : public std::enable_if<std::is_base_of<AbstractSink, Sink>::value, Sink>::type, public AbstractSource
	{
	public:
		AbstractFilter();

		void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
		void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
		bool ProcessCommand(const FilterCommand& cmd) override;

		// Persistent filter won't automatically Detach() when last subscriber disconnects
		void Persistent(bool value);

	protected:
		enum e_processingResult
		{
			e_noResult,
			e_moreBuffers,
			e_lastBuffer
		};

		// call base->derived
		// new frame from src. process it, and place result to buffer
		virtual e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) = 0;

		// call base->derived
		// if e_processingResult::e_moreBuffers is returned, GetNextBuffer will be called before next ProcessFrame call
		virtual e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md);

		// call base->derived
		// new format from src.
		virtual bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format);

		void NotifySourceUnused() override;

		std::shared_ptr<AbstractFilter> shared_from_this()
		{
			return std::shared_ptr<AbstractFilter>(Node::shared_from_this(), this);
		}

		std::shared_ptr<AbstractFilter const> shared_from_this() const
		{
			return std::shared_ptr<AbstractFilter const>(Node::shared_from_this(), this);
		}

		std::atomic<bool> m_persistent;
	};
}

