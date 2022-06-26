#include "FrameFilterLib/Video/FilterDumpVideo.h"

#if FFL_DUMP_VIDEO

#include "std/cpplib/VS_MediaFormat.h"

#include <sstream>

namespace ffl {

std::atomic<unsigned> FilterDumpVideo::s_cnt { 0 };

std::shared_ptr<AbstractSource> FilterDumpVideo::Create(const std::shared_ptr<AbstractSource>& src, string_view name)
{
	if (!src)
		return nullptr;
	return src->RegisterSinkOrGetCompatible(std::make_shared<FilterDumpVideo>(name));
}

FilterDumpVideo::FilterDumpVideo(string_view name)
	: m_name(name)
	, m_id(s_cnt.fetch_add(1, std::memory_order_relaxed))
{
	SetName("dump video");
}

auto FilterDumpVideo::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
{
	if (GetFormat().type == FilterFormat::e_mf)
	{
		switch (GetFormat().mf.dwVideoCodecFCC)
		{
		case FOURCC_I420:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << '.' << GetFormat().mf.dwVideoWidht << 'x' << GetFormat().mf.dwVideoHeight << ".yuv";
				m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);
			}
			if (m_file)
				m_file.write(buffer.data<const char>(), buffer.size());
			break;
		case VS_VCODEC_H264:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << ".264";
				m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);
			}
			if (m_file)
				m_file.write(buffer.data<const char>(), buffer.size());
			break;
		default:
			if (!m_file.is_open())
			{
				std::ostringstream fn;
				char fourcc[5];
				fourcc[0] = static_cast<char>(GetFormat().mf.dwVideoCodecFCC);
				fourcc[1] = static_cast<char>(GetFormat().mf.dwVideoCodecFCC >> 8);
				fourcc[2] = static_cast<char>(GetFormat().mf.dwVideoCodecFCC >> 16);
				fourcc[3] = static_cast<char>(GetFormat().mf.dwVideoCodecFCC >> 24);
				fourcc[4] = '\0';
				fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << '.' << fourcc << ".sz-data";
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

bool FilterDumpVideo::IsCompatibleWith(const AbstractSink* sink)
{
	auto p = dynamic_cast<const FilterDumpVideo*>(sink);
	if (!p)
		return false;
	return true;
}

}

#endif
