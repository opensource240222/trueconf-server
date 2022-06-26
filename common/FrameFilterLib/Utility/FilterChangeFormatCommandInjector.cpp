#include "FrameFilterLib/Utility/FilterChangeFormatCommandInjector.h"
#include "streams/Command.h"
#include "std/cpplib/VS_MediaFormat.h"

namespace ffl {
	std::shared_ptr<FilterChangeFormatCommandInjector> FilterChangeFormatCommandInjector::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterChangeFormatCommandInjector>());
	}

	FilterChangeFormatCommandInjector::FilterChangeFormatCommandInjector()
		: m_emit_command(false)
	{
		SetName("VS cmd injector");
	}

	auto FilterChangeFormatCommandInjector::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (!m_emit_command)
			return e_lastBuffer;
		if (GetFormat().type != FilterFormat::e_mf)
			return e_lastBuffer; // media format not known yet

		m_tmp = std::move(buffer);
		auto cmd = std::make_shared<stream::Command>();
		cmd->InfoRcvMFormat(GetFormat().mf);
		buffer = vs::SharedBuffer(cmd, cmd->Size());
		md = FrameMetadata::MakeCommand();
		m_emit_command = false;
		return e_moreBuffers;
	}

	auto FilterChangeFormatCommandInjector::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
	{
		if (!m_tmp.empty())
		{
			buffer = std::move(m_tmp);
			return e_lastBuffer;
		}
		return e_noResult;
	}

	bool FilterChangeFormatCommandInjector::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (format.type == FilterFormat::e_mf)
			m_emit_command = GetFormat().type != FilterFormat::e_mf || !(GetFormat().mf == format.mf);
		return AbstractFilter::ProcessFormat(src, format);
	}

	bool FilterChangeFormatCommandInjector::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterChangeFormatCommandInjector*>(sink);
		if (!p)
			return false;
		return true;
	}
}