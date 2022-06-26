#pragma once

#include "std/cpplib/VS_MediaFormat.h"
#include "std-generic/cpplib/VS_Container.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "Transcoder/VSVideoUtils.h"
#include "Transcoder/VS_TextDrawer.h"
#include "Transcoder/VideoBuffer.h"

#include <map>
#include <memory>
#include <vector>

class VS_VideoRay
{
public:
	VS_VideoRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor);
	virtual ~VS_VideoRay();

	bool SetOutputRect(VSRect outputRect);
	bool SetInputFormat(const VS_MediaFormat& mf);
	void SetDisplayName(const std::string& displayName);

	VSRect GetOutputRect();

	bool IsNeedDraw();
	std::pair<int32_t, int32_t> GetFrameSize();

	void SetVideoDummy(uint8_t* video, size_t width, size_t height);
	void SetDisplayNamePosition(EDisplayNamePosition position);

	void DrawDisplayName(uint8_t *video, VSSize size);
	virtual bool AddVideo(const std::shared_ptr<media_synch::VideoBuffer>& video);
	void TranscDrawVideo(uint8_t* dstFrame, bool forceDraw);
	void SetVideoReceive(bool enable);

	virtual bool GetVideo(uint8_t* /*video*/, int32_t& /*size*/, unsigned long& /*vi*/, uint32_t& /*mb*/, uint8_t& /*tl*/, bool& /*key*/, uint32_t& /*fourcc*/) { return false; }

protected:
	VSSize m_outputImageSize;

	VSSize m_inputSize;
	VSRect m_outputRect;
	VSRect m_resampledRect;
	std::shared_ptr<media_synch::VideoBuffer> m_videoBuffer;
	int m_backgndColor;
	std::string m_displayName;
	EDisplayNamePosition m_displayNamePosition = DNP_TOP;
	bool m_NeedEmptyFrame = true;
	VSVideoProcessingIpp m_vp;
	bool m_ReceiveVideo = true;
	bool m_needDraw = true;

	struct SizeLess
	{
		bool operator()(const VSSize& lhs, const VSSize& rhs) const
		{
			return (lhs.width < rhs.width) || ((lhs.width == rhs.width) && (lhs.height < rhs.height));
		}
	};

	std::map<VSSize, std::unique_ptr<VS_TextDrawer>, SizeLess> m_textDrawers;

	VSRect AspectRatioStream(VSRect rect, VSSize size);
	std::unique_ptr<VS_TextDrawer> CreateTextDrawer(VSSize size);
	void ResizeCurrentVideo();
};
