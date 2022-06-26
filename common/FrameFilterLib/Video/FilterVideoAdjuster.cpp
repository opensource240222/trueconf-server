#include "FrameFilterLib/Video/FilterVideoAdjuster.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "std-generic/cpplib/AtomicCache.h"

#include "std-generic/compat/memory.h"

namespace ffl {

	extern vs::AtomicCache<unsigned char*, std::default_delete<unsigned char[]>, 128> s_video_dec_memory_cache;

	std::shared_ptr<FilterVideoAdjuster> FilterVideoAdjuster::Create(const std::shared_ptr<AbstractSource>& src, unsigned int width, unsigned int height)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVideoAdjuster>(width, height));
	}

	FilterVideoAdjuster::FilterVideoAdjuster(unsigned int width, unsigned int height)
		: m_OutWidth(width)
		, m_OutHeight(height)
		, m_rsmpH(0)
		, m_rsmpW(0)
		, m_rsmpOffsetH(0)
		, m_rsmpOffsetW(0)
	{
		SetName("video adjuster");
		m_mf_in.SetZero();

		s_video_dec_memory_cache.IncreaseSize(1);
	}

	FilterVideoAdjuster::~FilterVideoAdjuster()
	{
		s_video_dec_memory_cache.DecreaseSize(1);
	}

	auto FilterVideoAdjuster::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::video)
			return e_noResult; // drop non-video data

		if (m_OutWidth == m_mf_in.dwVideoWidht && m_OutHeight == m_mf_in.dwVideoHeight)
			return e_lastBuffer;

		AdjustVideo(buffer);
		return e_lastBuffer;
	}

	bool FilterVideoAdjuster::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoAdjuster*>(sink);
		if (!p)
			return false;
		if (m_OutWidth != p->m_OutWidth)
			return false;
		if (m_OutHeight != p->m_OutHeight)
			return false;
		return true;
	}

	bool FilterVideoAdjuster::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (format.mf.dwVideoCodecFCC != FOURCC_I420)
			return false; // can only work with raw video

		if (m_mf_in.VideoEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC);
		UpdateAdjuster();
		return true;
	}

	void FilterVideoAdjuster::UpdateAdjuster()
	{
		m_rsmpH = m_rsmpW = m_rsmpOffsetH = m_rsmpOffsetW = 0;
		m_kw = m_kh = 0.0;
		if (m_mf_in.dwVideoWidht == 0 || m_mf_in.dwVideoHeight == 0 || m_OutWidth == 0 || m_OutHeight == 0) {
			return;
		}
		if (m_mf_in.dwVideoWidht != m_OutWidth || m_mf_in.dwVideoHeight != m_OutHeight) {
			int drawW = m_OutWidth;
			int drawH = m_OutHeight;

			m_rsmpW = drawW;
			m_rsmpH = drawH;
			m_kw = (double) m_mf_in.dwVideoWidht / (double) drawW;
			m_kh = (double) m_mf_in.dwVideoHeight / (double) drawH;

			if (m_kw > m_kh) {
				int nh = (int) (((drawH * m_kh) / m_kw + 0.5));
				int d0 = drawH - nh;
				int d1 = nh - (nh &~1);
				if (d1 < d0) {
					m_rsmpH = nh - d1;
				}
				m_rsmpOffsetH = ((drawH - m_rsmpH)&~3) / 2;
			}
			else if (m_kh > m_kw) {
				int nw = (int) (((drawW * m_kw) / m_kh + 0.5));
				int d0 = drawW - nw;
				int d1 = nw - (nw &~1);
				if (d1 < d0) {
					m_rsmpW = nw - d1;
				}
				m_rsmpOffsetW = ((drawW - m_rsmpW)&~3) / 2;
			}
			else {
				m_rsmpOffsetW = m_rsmpOffsetH = 0;
			}
		}

		SetFormat(FilterFormat::MakeVideo(FOURCC_I420, m_OutWidth, m_OutHeight));
	}

	void FilterVideoAdjuster::AdjustVideo(vs::SharedBuffer& buffer)
	{
		if (m_rsmpW || m_rsmpH || m_rsmpOffsetH || m_rsmpOffsetW)
		{
			struct return_to_cache { void operator()(unsigned char* x) const { s_video_dec_memory_cache.Put(x); }};
			std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_dec_memory_cache.Get());
			if (!memory_block)
				memory_block.reset(new unsigned char[1920 * 1080 * 3 / 2]);
			auto out_buffer = vs::SharedBuffer(std::move(memory_block), m_OutWidth * m_OutHeight * 3 / 2);

			uint8_t* srcPlanes[3];
			uint8_t* dstPlanes[3];

			srcPlanes[0] = const_cast<uint8_t*>(buffer.data<const uint8_t>());
			srcPlanes[1] = srcPlanes[0] + m_mf_in.dwVideoWidht * m_mf_in.dwVideoHeight;
			srcPlanes[2] = srcPlanes[0] + m_mf_in.dwVideoWidht * m_mf_in.dwVideoHeight * 5 / 4;

			dstPlanes[0] = out_buffer.data<uint8_t>();
			dstPlanes[1] = dstPlanes[0] + m_OutWidth * m_OutHeight;
			dstPlanes[2] = dstPlanes[0] + m_OutWidth * m_OutHeight * 5 / 4;

			if (m_kw == m_kh) {
				m_vproc.ResampleCropI420(srcPlanes, dstPlanes,
										 m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight, m_mf_in.dwVideoWidht,
										 m_OutWidth, m_OutHeight, m_OutWidth,
										 m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight,
										 0, 0, 1.0 / m_kw, 1.0 / m_kh, IPPI_INTER_LINEAR); // linear
			}
			else {
				m_vproc.ResampleInscribedI420(srcPlanes, dstPlanes,
											 m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight, m_mf_in.dwVideoWidht,
											 m_OutWidth, m_OutHeight, m_OutWidth,
											 m_rsmpOffsetW, m_rsmpOffsetH, 1.0 / m_kw, 1.0 / m_kh, IPPI_INTER_LINEAR); // linear
			}

			buffer = std::move(out_buffer);
		}
	}
}