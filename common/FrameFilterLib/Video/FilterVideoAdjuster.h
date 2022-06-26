#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "IppLib2/VSVideoProcessingIpp.h"

#include <memory>

namespace ffl {
	class FilterVideoAdjuster : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVideoAdjuster> Create(const std::shared_ptr<AbstractSource>& src, unsigned int width, unsigned int height);

		FilterVideoAdjuster(unsigned int width, unsigned int height);
		~FilterVideoAdjuster();
		unsigned int GetWidth() const  { return m_OutWidth;  }
		unsigned int GetHeight() const { return m_OutHeight; }

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateAdjuster();
		void InitAdjustVideo(unsigned int in_w, unsigned int in_h, unsigned int out_w, unsigned int out_h);
		void AdjustVideo(vs::SharedBuffer& buffer);

		const unsigned int m_OutWidth;
		const unsigned int m_OutHeight;
		VS_MediaFormat m_mf_in;

		// video resampler settings
		int					m_rsmpH, m_rsmpW;
		int					m_rsmpOffsetH, m_rsmpOffsetW;
		double				m_kw = 0.0, m_kh = 0.0;
		VSVideoProcessingIpp m_vproc;
	};
}