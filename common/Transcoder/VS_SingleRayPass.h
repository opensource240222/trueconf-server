#pragma once

#include "mixer/VS_VideoRay.h"
#include "mixer/VS_AudioRay.h"

#include <boost/shared_ptr.hpp>

#include <map>
#include <queue>

class VS_VS_InputBuffer;

class VS_SingleRayPass : public VS_VideoRay, public VS_AudioRay
{

private:

	typedef std::queue<std::pair<boost::shared_ptr<uint8_t> /* packet */, int32_t /* size */>> tPacketQueue;
	typedef std::map <uint8_t /* sl */, std::pair<uint32_t /* mb */, VS_VS_InputBuffer>> tInputVideo;
	tPacketQueue m_videoQueue;
	tPacketQueue m_audioQueue;
	tInputVideo  m_videoInput;

public:

	VS_SingleRayPass(const std::string& dname);
	virtual ~VS_SingleRayPass();
	//bool AddVideo(uint8_t *video, int32_t procWidth, int32_t procHeight, int32_t baseWidth, int32_t baseHeight) override;
	bool AddAudio(uint8_t *audio, int32_t size) override;
	bool GetVideo(uint8_t *video, int32_t &size, unsigned long &vi, uint32_t &mb, uint8_t &tl, bool &key, uint32_t &fourcc) override;
	uint8_t* GetAudio(int32_t &size);
	void DropAudio(int32_t size) override;
};
