#include "FrameFilterLib/Audio/FilterDumpAudio.h"

#if FFL_DUMP_AUDIO

#include <iomanip>
#include <sstream>

namespace ffl {

std::atomic<unsigned> FilterDumpAudio::s_cnt { 0 };

std::shared_ptr<AbstractSource> FilterDumpAudio::Create(const std::shared_ptr<AbstractSource>& src, string_view name)
{
	if (!src)
		return nullptr;
	return src->RegisterSinkOrGetCompatible(std::make_shared<FilterDumpAudio>(name));
}

FilterDumpAudio::FilterDumpAudio(string_view name)
	: m_name(name)
	, m_id(s_cnt.fetch_add(1, std::memory_order_relaxed))
{
	SetName("dump audio");
}

auto FilterDumpAudio::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
{
	if (GetFormat().type == FilterFormat::e_mf)
	{
		switch (GetFormat().mf.dwAudioCodecTag)
		{
		case VS_ACODEC_PCM:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << '.' << GetFormat().mf.dwAudioSampleRate << "Hz.s16le";
				m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);
			}
			if (m_file)
				m_file.write(buffer.data<const char>(), buffer.size());
			break;
		case VS_ACODEC_MP3:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << ".mp3";
				m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);
			}
			if (m_file)
				m_file.write(buffer.data<const char>(), buffer.size());
			break;
		default:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << ".0x" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int16_t>(GetFormat().mf.dwAudioCodecTag) << ".sz-data";
				m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);
			}
			if (m_file)
			{
				auto sz = static_cast<uint32_t>(buffer.size());
				m_file.write(reinterpret_cast<char*>(&sz), sizeof(sz));
				m_file.write(buffer.data<const char>(), buffer.size());
			}
		}
	}

	return e_lastBuffer;
}

bool FilterDumpAudio::IsCompatibleWith(const AbstractSink* sink)
{
	auto p = dynamic_cast<const FilterDumpAudio*>(sink);
	if (!p)
		return false;
	return true;
}

}

#endif
