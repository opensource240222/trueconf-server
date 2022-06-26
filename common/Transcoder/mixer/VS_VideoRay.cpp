#include "VS_VideoRay.h"

VS_VideoRay::VS_VideoRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor)
{
	m_inputSize = { 640, 360 };
	m_outputImageSize = { out.dwVideoWidht, out.dwVideoHeight };
	m_displayName = dname;
	m_backgndColor = backgndColor;
}

VS_VideoRay::~VS_VideoRay()
{
}

bool VS_VideoRay::SetOutputRect(VSRect outputRect)
{
	m_outputRect = outputRect;
	m_resampledRect = AspectRatioStream(m_outputRect, m_inputSize);
	m_NeedEmptyFrame = true;
	m_needDraw = true;

	m_textDrawers.clear();
	ResizeCurrentVideo();

	return true;
}

void VS_VideoRay::SetDisplayName(const std::string& displayName)
{
	m_displayName = displayName;

	m_textDrawers.clear();
}

VSRect VS_VideoRay::GetOutputRect()
{
	return m_outputRect;
}

VSRect VS_VideoRay::AspectRatioStream(const VSRect rect, const VSSize size)
{
	VSRect res = { {}, rect.size };

	double xk = (double)size.width / (double)rect.size.width;
	double yk = (double)size.height / (double)rect.size.height;

	if (xk > yk)
	{
		int nh = (int)(((rect.size.height * yk) / xk + 0.5));
		int d0 = rect.size.height - nh;
		int d1 = nh - (nh &~1);

		if (d1 < d0)
			res.size.height = nh - d1;

		res.offset.y = ((rect.size.height - res.size.height)&~3) / 2;
	}
	else if (yk >= xk)
	{
		int nw = (int)(((rect.size.width * xk) / yk + 0.5));
		int d0 = rect.size.width - nw;
		int d1 = nw - (nw &~1);

		if (d1 < d0)
			res.size.width = nw - d1;

		res.offset.x = ((rect.size.width - res.size.width)&~3) / 2;
	}

	return res;
}

std::pair<int32_t, int32_t> VS_VideoRay::GetFrameSize()
{
	return std::make_pair(m_resampledRect.size.width, m_resampledRect.size.height);
}

void VS_VideoRay::SetVideoDummy(uint8_t* video, size_t width, size_t height)
{
	m_vp.ResampleI420(
		video,
		width, height,
		m_videoBuffer->buffer,
		m_videoBuffer->width, m_videoBuffer->height);

	m_needDraw = true;
}

void VS_VideoRay::SetDisplayNamePosition(EDisplayNamePosition position)
{
	m_displayNamePosition = position;

	for (const auto& i : m_textDrawers)
		i.second->SetDisplayNamePosition(m_displayNamePosition);

	m_needDraw = true;
}

void VS_VideoRay::DrawDisplayName(uint8_t* video, VSSize size)
{
	auto it = m_textDrawers.find(size);

	if (it == m_textDrawers.end())
	{
		auto added = m_textDrawers.emplace(size, CreateTextDrawer(size));
		it = added.first;
	}

	it->second->DrawNameText(video, m_backgndColor);
}

bool VS_VideoRay::AddVideo(const std::shared_ptr<media_synch::VideoBuffer>& video)
{
	if (!m_ReceiveVideo) {
		return false;
	}

	if (video->width != m_resampledRect.size.width || video->height != m_resampledRect.size.height
		|| video->input_width != m_inputSize.width || video->input_height != m_inputSize.height)
	{
		m_inputSize.width = video->input_width;
		m_inputSize.height = video->input_height;

		m_resampledRect = AspectRatioStream(m_outputRect, m_inputSize);

		m_NeedEmptyFrame = true;

		m_videoBuffer.reset(new media_synch::VideoBuffer(m_resampledRect.size.Square() * 3 / 2, m_resampledRect.size.width, m_resampledRect.size.height));

		m_vp.ResampleI420(
			video->buffer, video->width, video->height,
			m_videoBuffer->buffer, m_resampledRect.size.width, m_resampledRect.size.height);
	}
	else
	{
		m_videoBuffer = video;
	}

	m_needDraw = true;

	return true;
}

bool VS_VideoRay::SetInputFormat(const VS_MediaFormat& mf)
{
	m_inputSize = { mf.dwVideoWidht, mf.dwVideoHeight };
	m_resampledRect = AspectRatioStream(m_outputRect, m_inputSize);
	m_NeedEmptyFrame = true;
	m_needDraw = true;

	ResizeCurrentVideo();

	return true;
}

void VS_VideoRay::TranscDrawVideo(uint8_t* dstFrame, bool forceDraw)
{
	if (m_needDraw || forceDraw)
	{
		VSI420ImageView dstFrameView(dstFrame, m_outputImageSize);

		if (m_NeedEmptyFrame)
		{
			dstFrameView.GetSubview(m_outputRect).Fill(m_backgndColor);
			m_NeedEmptyFrame = false;
		}

		if (m_videoBuffer)
		{
			VSI420ImageView raySrcData(m_videoBuffer->buffer, { (uint32_t)m_videoBuffer->width, (uint32_t)m_videoBuffer->height });

			VSRect rayDstRect{ m_outputRect.offset + m_resampledRect.offset, m_resampledRect.size };
			VSI420ImageView rayDstRata = dstFrameView.GetSubview(rayDstRect);

			raySrcData.CopyTo(rayDstRata);

			m_needDraw = false;
		}
	}
}

void VS_VideoRay::SetVideoReceive(bool enable)
{
	m_ReceiveVideo = enable;
	m_videoBuffer.reset();
	m_needDraw = true;
	m_NeedEmptyFrame = true;
}

bool VS_VideoRay::IsNeedDraw()
{
	return m_needDraw;
}

std::unique_ptr<VS_TextDrawer> VS_VideoRay::CreateTextDrawer(VSSize size)
{
	VSRect drawed = m_outputRect;

	if (size != m_outputImageSize)
	{
		float factor = float(size.width) / float(m_outputImageSize.width);

		drawed.offset *= factor;
		drawed.size *= factor;
	}

	return std::make_unique<VS_TextDrawer>(
		size,
		drawed,
		m_displayNamePosition,
		m_displayName);
}

void VS_VideoRay::ResizeCurrentVideo()
{
	if (m_videoBuffer)
	{
		if (m_resampledRect.size.width != m_videoBuffer->width
			&& m_resampledRect.size.height != m_videoBuffer->height)
		{
			auto tmp = m_videoBuffer;
			m_videoBuffer.reset(new media_synch::VideoBuffer(m_resampledRect.size.Square() * 3 / 2, m_resampledRect.size.width, m_resampledRect.size.height));

			m_vp.ResampleI420(
				tmp->buffer, tmp->width, tmp->height,
				m_videoBuffer->buffer, m_resampledRect.size.width, m_resampledRect.size.height);
		}
	}
	else if (m_resampledRect.size.Square())
	{
		m_videoBuffer.reset(new media_synch::VideoBuffer(m_resampledRect.size.Square() * 3 / 2, m_resampledRect.size.width, m_resampledRect.size.height));

		VSI420ImageView bufferView(m_videoBuffer->buffer, { (uint32_t)m_videoBuffer->width, (uint32_t)m_videoBuffer->height });
		bufferView.Fill(m_backgndColor);
	}
}
