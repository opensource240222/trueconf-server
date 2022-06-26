#include "AudioCodec.h"

class AudioCodecSystem : public AudioCodec
{
	HACMDRIVER				m_had;
	HACMSTREAM				m_has;
	static BOOL CALLBACK acmDriverEnumCallback(HACMDRIVERID hadid, DWORD_PTR dwInstance, DWORD fdwSupport);
public:
	AudioCodecSystem(DWORD outTag, bool coder, DWORD gran);
	virtual ~AudioCodecSystem();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int  ConvertFunction();
};

/****************************************************************************
 * Coder and decoder declaration macros
 ****************************************************************************/
#define DECLARE_AUDIOCODER(name, tag, gran) \
class VS_AudioCoder##name: public AudioCodecSystem { \
public: \
	VS_AudioCoder##name() : AudioCodecSystem(tag, true, gran){} \
	~VS_AudioCoder##name(){}; \
};

#define DECLARE_AUDIODECODER(name, tag, gran) \
class VS_AudioDecoder##name: public AudioCodecSystem { \
public: \
	VS_AudioDecoder##name() : AudioCodecSystem(tag, false, gran){} \
	~VS_AudioDecoder##name(){}; \
};

 /****************************************************************************
  * GSM 6.10 by Microsoft classes
  ****************************************************************************/
DECLARE_AUDIOCODER(GSM610, VS_ACODEC_GSM610, 640)
DECLARE_AUDIODECODER(GSM610, VS_ACODEC_GSM610, 65)
