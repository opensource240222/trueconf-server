#include "Transcoder/VideoCodec.h"
#include <Windows.h>

class VS_HardwareEncoderObserver;

class VS_HardwareH264VideoCodec : public VideoCodec
{
	VS_HardwareEncoderObserver	*m_pHWControl;
protected:
	bool			UpdateBitrate();
public:
	VS_HardwareH264VideoCodec(int CodecId, bool IsCoder);
	~VS_HardwareH264VideoCodec();
	int				Init(int w, int h, uint32_t ColorMode = BI_RGB, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void			Release();
	int				Convert(BYTE *in, BYTE* out, VS_VideoCodecParam* param);
};
