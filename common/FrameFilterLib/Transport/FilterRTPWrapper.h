#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

class VS_RTP_OutputBuffer;

namespace ffl {
	class FilterRTPWrapper : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		typedef std::map<uint32_t, uint8_t> codec_pt_map_t;
		typedef std::map<uint8_t, bool> swap_bytes_map_t;

		static std::shared_ptr<FilterRTPWrapper> Create(const std::shared_ptr<AbstractSource>& src, const codec_pt_map_t& formats, const swap_bytes_map_t& swap_bytes = {});

		FilterRTPWrapper(const codec_pt_map_t& formats, const swap_bytes_map_t& swap_bytes);

		bool IsCompatibleWith(const AbstractSink* sink) override;

		void set_ssrc_override(unsigned long ssrc) { m_ssrc_override = ssrc; }
	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateFormat();

		std::unique_ptr<VS_RTP_OutputBuffer> m_outBuff;
		codec_pt_map_t m_possibleFormats; // codec id -> payload type
		swap_bytes_map_t m_swap_bytes; // payload type -> should use non-standard byte order?
		bool m_haveNewFormat;
		VS_MediaFormat m_mf;
		unsigned long m_ssrc_override;
	};
}