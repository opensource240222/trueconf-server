#include "FilterSlideEncoder.h"

#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "tools/Server/CommonTypes.h"
#include "std-generic/cpplib/scope_exit.h"
#include "JPEGUtils/jpeg_utils.h"
#include "../../IppLib2/VSVideoProcessingIpp.h"

namespace ffl {
	std::shared_ptr<FilterSlideEncoder> FilterSlideEncoder::Create(const std::shared_ptr<AbstractSource>& src,
																	 const std::function<void(const std::vector<unsigned char> &data, const SlideInfo &info)> &on_slide_update,
																	 std::chrono::steady_clock::duration slide_interval,
																	 unsigned max_side, unsigned quality) {
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterSlideEncoder>(on_slide_update, slide_interval, max_side, quality));
	}

	FilterSlideEncoder::FilterSlideEncoder(const std::function<void(const std::vector<unsigned char> &data, const SlideInfo &info)> &on_slide_update, std::chrono::steady_clock::duration slide_interval,
										   unsigned max_side, unsigned quality)
		: AbstractThreadedFilter(std::chrono::milliseconds(500)), m_slide_interval(slide_interval), m_slide_counter(0), m_slide_updating(false),
		  m_max_side(max_side), m_quality(quality), m_on_slide_update(on_slide_update) {
		SetName("slide encoder");
		m_mf_in.SetZero();
		m_next_slide = std::chrono::steady_clock::now();

		auto min_slide_interval = std::chrono::milliseconds(500);
		if (m_slide_interval < min_slide_interval) {
			m_slide_interval = min_slide_interval;
		}
		if (m_max_side > 1920) {
			m_max_side = 1920;
		}
		if (m_quality > 100) {
			m_quality = 100;
		}
	}

	auto FilterSlideEncoder::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult {
		if (md.track != stream::Track::video)
			return e_noResult; // drop non-video data

		auto now = std::chrono::steady_clock::now();
		if (now > m_next_slide && !m_slide_updating.exchange(true)) {
			VS_Singleton<VS_ThreadPool>::Instance().Post(std::bind(&FilterSlideEncoder::PrepareSlide, shared_from_this(), buffer, md));
			m_next_slide = now + m_slide_interval;
		}
		return e_lastBuffer;
	}

	bool FilterSlideEncoder::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format) {
		m_mf_in.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC);
		return true;
	}

	bool FilterSlideEncoder::IsCompatibleWith(const AbstractSink* sink) {
		auto p = dynamic_cast<const FilterSlideEncoder*>(sink);
		if (!p)
			return false;
		return true;
	}

	void FilterSlideEncoder::PrepareSlide(vs::SharedBuffer& buffer, FrameMetadata& /*md*/) {
		VS_SCOPE_EXIT{ m_slide_updating = false; };

		std::vector<unsigned char> jpg_out_buf;
		SlideInfo info;
		info.slide_count = ++m_slide_counter;
		info.slide_n = info.slide_count - 1;
		info.about = "DesktopSharing";
		info.img_type = "image/jpeg";

		{
			unsigned _w = m_mf_in.dwVideoWidht, _h = m_mf_in.dwVideoHeight;
			auto *buf_in = buffer.data<const uint8_t>();

			if (buffer.size() < (unsigned)_w * _h * 3 / 2) return;

			VSVideoProcessingIpp prc;

			std::vector<uint8_t> resampled_buf;

			unsigned w = _w, h = _h;
			if (w > m_max_side || h > m_max_side) {
				auto aspect = double(w) / h;
				if (w > h) {
					w = m_max_side;
					h = w / aspect;
				} else {
					h = m_max_side;
					w = aspect * h;
				}
				resampled_buf.resize((w * h * 3) / 2);
				prc.ResampleI420(const_cast<uint8_t *>(&buf_in[0]), _w, _h, &resampled_buf[0], w, h);
				buf_in = &resampled_buf[0];
			}

			/*std::vector<uint8_t> rgb_out_buf(w * h * 3);

			uint8_t *Y = const_cast<uint8_t *>(&buf_in[0]);
			uint8_t *U = Y + w * h;
			uint8_t *V = Y + w * h * 5 / 4;
			prc.ConvertI420ToBMF24(Y, U, V, &rgb_out_buf[0], w, h, w);

			JPEGEncodeRGB24ToMem(&rgb_out_buf[0], w, h, jpg_out_buf);*/
			jpeg::write_I420_mem(buf_in, jpg_out_buf, w, h, 0);
			info.w = w;
			info.h = h;
		}

		m_on_slide_update(jpg_out_buf, info);
	}
}
