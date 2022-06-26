#pragma once

#include "std/cpplib/VS_MediaFormat.h"
#include "Transcoder/VS_FfmpegResampler.h"
#include "std/VS_FifoBuffer.h"

#include <map>
#include <memory>
#include <vector>

class VS_AudioRay
{
public:
	VS_AudioRay(const VS_MediaFormat& out);
	virtual ~VS_AudioRay();

	virtual bool AddAudio(uint8_t* audio, int size);
	uint8_t* GetAudio(size_t requestSize);
	int16_t* GetStorageBuffer();
	virtual void DropAudio(int size);

	bool SetOutputFormat(VS_MediaFormat *out);
	bool SetInputFormat(const VS_MediaFormat& mf);

protected:
	VS_MediaFormat m_infmt;
	VS_MediaFormat m_outfmt;

	VS_FifoBuffer m_Audio;
	VS_FfmpegResampler m_resmp;

	std::unique_ptr<int16_t[]> m_storageBuffer;
	std::vector<uint8_t> m_resampleBuffer;
};
