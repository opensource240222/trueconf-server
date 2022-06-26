#include "FrameFilterLib/Video/FilterVideoTranscoderWithResolutionLimits.h"
#include "tools/SingleGatewayLib/ModeSelection.h"

namespace ffl {
	std::shared_ptr<FilterVideoTranscoderWithResolutionLimits> FilterVideoTranscoderWithResolutionLimits::Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, const VS_ClientCaps& caps)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVideoTranscoderWithResolutionLimits>(src, formats, allow_upscale, default_format, caps));
	}

	FilterVideoTranscoderWithResolutionLimits::FilterVideoTranscoderWithResolutionLimits(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, const VS_ClientCaps& caps)
		: FilterVideoTranscoder(src, formats, allow_upscale, default_format, 0, false)
		, m_caps(caps)
	{
		SetName("video transcoder+");
	}

	bool FilterVideoTranscoderWithResolutionLimits::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoTranscoderWithResolutionLimits*>(sink);
		if (!p)
			return false;
		if (m_caps != p->m_caps)
			return false;
		return FilterVideoTranscoder::IsCompatibleWith(p);
	}

	bool FilterVideoTranscoderWithResolutionLimits::SelectOutputFormat(VS_MediaFormat& new_mf_out)
	{
		if (!FilterVideoTranscoder::SelectOutputFormat(new_mf_out))
			return false;
		if (!LimitRTP2VSResolution(m_caps, new_mf_out, new_mf_out))
			return false;
		return true;
	}
}
