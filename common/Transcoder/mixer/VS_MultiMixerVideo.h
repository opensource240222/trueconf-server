#pragma once

#include <map>
#include <memory>
#include <boost/functional/hash.hpp>
#include "VS_VideoRay.h"
#include "Transcoder/VS_VideoWindow.h"
#include "Transcoder/VS_AudioMixer.h"
#include "Transcoder/VideoBuffer.h"

#define MAX_NUM_STREAMS		(64)
#define LIMIT_CUR_STREAMS	(64)

struct VS_MixerGrid
{
	unsigned int	W;
	unsigned int	H;
	int				arW;
	int				arH;
	int				wMultp;
	int				hMultp;
};

typedef std::map<std::string /* ray id */, std::pair<int32_t /* width */, int32_t /* height */>> mixerVideoRaySize;

struct VSRayInfo
{
	enum ERayType : uint8_t
	{
		RT_EMPTY,
		RT_USER,
		RT_CONTENT
	};

	VSRect rect;
	std::string displayname;
	ERayType type = RT_USER;
	bool priority = false;
};

struct VSLayoutDesc
{
	std::map<std::string, VSRayInfo> layout;
	uint32_t width = 0;
	uint32_t height = 0;
	EDisplayNamePosition displayNamePosition = DNP_TOP;

	std::size_t Hash() const
	{
		std::size_t hash(0);
		boost::hash_combine(hash, width);
		boost::hash_combine(hash, height);
		for (const auto & it : layout) {
			boost::hash_combine(hash, it.first);
			boost::hash_combine(hash, it.second.displayname);
			boost::hash_combine(hash, it.second.priority);
			boost::hash_combine(hash, static_cast<uint8_t>(it.second.type));
			boost::hash_combine(hash, it.second.rect.offset.x);
			boost::hash_combine(hash, it.second.rect.offset.y);
			boost::hash_combine(hash, it.second.rect.size.width);
			boost::hash_combine(hash, it.second.rect.size.height);
		}
		return hash;
	}
};

class VS_MultiMixerVideo
{
public:
	VS_MultiMixerVideo(const VS_MixerGrid& CustomMixerGrid);
	~VS_MultiMixerVideo();

	bool Add(const std::string& handle, const std::shared_ptr<media_synch::VideoBuffer>& video);

	uint32_t GetVideo(uint8_t* buff, uint32_t width, uint32_t height);
	int GetMuxerOutWidth();
	int GetMuxerOutHeight();
	mixerVideoRaySize GetMuxerRaysSize();
	std::vector<VS_WindowGrid::sOneWindow> GetRaysGrid();

	void SetVideoDummy(const std::string& handle, uint8_t* video, size_t width, size_t height);
	void SetRayRect(const std::string& handle, int x, int y, int w, int h);
	void SetLayoutFixed(bool fixed);
	void SetLayout(const VSLayoutDesc& layout);
	void SetDisplayNamePosition(EDisplayNamePosition position);


	void Refresh();
	VS_WindowGrid* GetLayoutControl() { return &m_WindowGrigManager; }

	virtual bool AddRay(const std::string& handle, const std::string& name, const VS_MediaFormat& in);
	bool RenameRay(const std::string& handle, const std::string& name);
	bool DeleteRay(const std::string& handle);

	virtual bool Get(uint8_t* /*buf*/, int32_t& /*size*/, uint32_t& /*mb*/, uint8_t& /*tl*/, bool& /*key*/, uint32_t& /*tm*/, uint32_t& /*fourcc*/) { return false; }

protected:
	std::map<std::string, std::shared_ptr<VS_VideoRay>> m_mr;

	std::weak_ptr<VS_VideoRay> m_PriorityRay;
	VS_MixerGrid	m_stMixerGrid;
	VS_WindowGrid	m_WindowGrigManager;
	VS_MediaFormat	m_outFormat;
	int				m_fillBackGndColor;
	bool			m_NeedEmptyFrame = true;
	bool			m_LayoutFixed = false;
	EDisplayNamePosition m_displayNamePosition = DNP_TOP;
	std::vector<uint8_t> m_internalFrame;
	VSVideoProcessingIpp m_videoProcessing;
	bool m_InternalFrameActual = false;

	void CalculateLayout();
	void ApplyLayout(const std::map<std::string, VSRayInfo>& layout);

	virtual VS_VideoRay* CreateRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor);
};
