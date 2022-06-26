#include "FrameFilterLib/Utility/FilterChangeFormatCommandFixer.h"
#include "streams/Command.h"
#include "std/cpplib/VS_MediaFormat.h"

namespace ffl {
	std::shared_ptr<FilterChangeFormatCommandFixer> FilterChangeFormatCommandFixer::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterChangeFormatCommandFixer>());
	}

	FilterChangeFormatCommandFixer::FilterChangeFormatCommandFixer()
	{
		SetName("VS cmd fixer");
	}

	auto FilterChangeFormatCommandFixer::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track != stream::Track::command)
			return e_lastBuffer; // forward non-commands
		if (GetFormat().type != FilterFormat::e_mf)
			return e_lastBuffer; // format not known yet
		if (buffer.size() < stream::Command::header_size)
			return e_lastBuffer; // unknown command

		auto& cmd = *buffer.make_exclusive().data<stream::Command>();
		if (buffer.size() != cmd.Size())
			return e_lastBuffer; // malformed command
		if (!(cmd.type == stream::Command::Type::ChangeRcvMFormat && (cmd.sub_type == stream::Command::Request || cmd.sub_type == stream::Command::Info)))
			return e_lastBuffer; // non interesting command

		auto& mf = *reinterpret_cast<VS_MediaFormat*>(cmd.data);
		const VS_MediaFormat& our_mf = GetFormat().mf;
		if (mf.dwAudioCodecTag == 0)
			mf.SetAudio(our_mf.dwAudioSampleRate, our_mf.dwAudioCodecTag);
		if (mf.dwVideoCodecFCC == 0)
			mf.SetVideo(our_mf.dwVideoWidht, our_mf.dwVideoHeight, our_mf.dwVideoCodecFCC);
		return e_lastBuffer;
	}

	bool FilterChangeFormatCommandFixer::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterChangeFormatCommandFixer*>(sink);
		if (!p)
			return false;
		return true;
	}
}