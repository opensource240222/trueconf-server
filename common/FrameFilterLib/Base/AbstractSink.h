#pragma once

#include "FrameFilterLib/Base/Node.h"
#include "FrameFilterLib/Base/FrameMetadata.h"
#include "std-generic/cpplib/SharedBuffer.h"

namespace ffl {
	struct FilterFormat;
	struct FilterCommand;
	class AbstractSource;

	class AbstractSink : virtual public Node
	{
	public:
		virtual ~AbstractSink();

		// Should return true if this sink can be replaced with argument sink.
		virtual bool IsCompatibleWith(const AbstractSink* sink) = 0;

		virtual void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) = 0;
		virtual void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) = 0;
		// Each source is responsible for serializing its calls to PutFrame() and NotifyNewFormat().
		// E.g. only one call to either of those functions (with the same src argument) should be running at any point in time.

		virtual void NotifyNewSource(const std::shared_ptr<AbstractSource>& src) = 0;
		virtual void NotifyRemoveSource(const std::shared_ptr<AbstractSource>& src) = 0;

		// Unregisters this sink from all sources
		virtual void Detach() = 0;

		// Returns true if at least one source procsessed passed command.
		virtual bool SendCommandToSources(const FilterCommand& cmd, bool log = true) = 0;

	protected:
		std::shared_ptr<AbstractSink> shared_from_this()
		{
			return std::shared_ptr<AbstractSink>(Node::shared_from_this(), this);
		}

		std::shared_ptr<AbstractSink const> shared_from_this() const
		{
			return std::shared_ptr<AbstractSink const>(Node::shared_from_this(), this);
		}
	};
}
