#pragma once

#include "FrameFilterLib/Video/FilterVideoTranscoder.h"
#include "std/cpplib/VS_ClientCaps.h"

namespace ffl {
	class FilterVideoTranscoderWithResolutionLimits : public FilterVideoTranscoder
	{
	public:
		static std::shared_ptr<FilterVideoTranscoderWithResolutionLimits> Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, const VS_ClientCaps& caps);

		FilterVideoTranscoderWithResolutionLimits(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, const VS_ClientCaps& caps);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		bool SelectOutputFormat(VS_MediaFormat& new_mf_out) override;

	private:
		VS_ClientCaps m_caps;
	};
}
