#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "tools/Server/CommonTypes.h"

#include <cstdint>
#include <map>

class VS_RTP_InputBuffer;

namespace ffl {
	class FilterRTPUnwrapper : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		typedef std::map<uint8_t, VS_MediaFormat> format_map_t;
		typedef std::map<uint8_t, VS_MPEG4ESConfiguration> mpeg4es_config_map_t;
		typedef std::map<uint8_t, bool> swap_bytes_map_t;

		static std::shared_ptr<FilterRTPUnwrapper> Create(const std::shared_ptr<AbstractSource>& src, const format_map_t& formats, const mpeg4es_config_map_t& mpeg4es_conf = {}, const swap_bytes_map_t& swap_bytes = {}, bool use_h264_uc = false);

		FilterRTPUnwrapper(const format_map_t& formats, const mpeg4es_config_map_t& mpeg4es_conf, const swap_bytes_map_t& swap_bytes, bool use_h264_uc);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		void UpdateFormat(uint8_t pt);

		std::unique_ptr<VS_RTP_InputBuffer> m_inBuff;
		format_map_t m_possibleFormats; // payload type -> media format
		mpeg4es_config_map_t m_mpeg4es_conf; // payload type -> MPEG4ES config
		swap_bytes_map_t m_swap_bytes; // payload type -> should use non-standard byte order?
		bool m_use_h264_uc;
		uint8_t m_currPT;
	};
}