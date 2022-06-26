#pragma once

#include "VSVideoFile.h"

class VSVideoFileWriter : public VSVideoFile
{
public:
	VSVideoFileWriter();
	~VSVideoFileWriter();

	bool Init(std::string fileName);

	bool SetVideoFormat(SVideoInfo videoInfo);
	bool SetAudioFormat(SAudioInfo audioInfo);

	bool WriteHeader();

	bool WriteVideo(char* data, int size, bool isKey, int videoInterval = -1);
	bool WriteVideoTimeAbs(char* data, int size, bool isKey, int absoluteTime);

	bool WriteAudioSamples(char* data, int size, int samples = -1);
	bool WriteAudioTime(char* data, int size, int audioInterval);
	bool WriteAudioTimeAbs(char* data, int size, int absoluteTime);

	uint64_t Release();

private:
	bool m_HeaderWrited = false;
};
