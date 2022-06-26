#include "VS_IppAudiCodec.h"
#include "IppLib2/libinit.h"
#include "ipps.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../Transcoder/ipp_speech_core/usc/include/usc.h"

#define _USC_CODECS

#include "../Transcoder/ipp_speech_core/usc/include/usc_objects_decl.h"

#ifdef __cplusplus
}
#endif

#define LOCAL_ALLOC(size) ippsMalloc_8u(size)
#define LOCAL_FREE(ptr) ippsFree(ptr)

uint32_t GranOfCodec(uint32_t tag, bool codec)
{
	if (codec)
		switch(tag)
	{
		case VS_ACODEC_PCM:		 return 160;  // 10 ms
		case VS_ACODEC_G711a:	 return 160;  // 10 ms
		case VS_ACODEC_G711mu:	 return 160;  // 10 ms
		case VS_ACODEC_G723:	 return 480;  // 30 ms
		case VS_ACODEC_G729A:	 return 160;  // 10 ms
		case VS_ACODEC_G728:	 return 160;  // 10 ms
		case VS_ACODEC_G7221_24: return 640;  // 40 ms
		case VS_ACODEC_G7221_32: return 640;  // 40 ms
		case VS_ACODEC_G7221C_24:return 1280; // 40 ms
		case VS_ACODEC_G7221C_32:return 1280; // 40 ms
		case VS_ACODEC_G7221C_48:return 1280; // 40 ms
		case VS_ACODEC_G722:	 return 320;  // 20 ms
	}
	else
		switch(tag)
	{
		case VS_ACODEC_PCM:		 return 160;
		case VS_ACODEC_G711a:	 return 80;
		case VS_ACODEC_G711mu:	 return 80;
		case VS_ACODEC_G723:	 return 24;
		case VS_ACODEC_G728:	 return 20;
		case VS_ACODEC_G729A:	 return 10;
		case VS_ACODEC_G7221_24: return 60;  // at 24 kbit/s
		case VS_ACODEC_G7221_32: return 80;  // at 32 kbit/s
		case VS_ACODEC_G7221C_24:return 60;  // at 24 kbit/s
		case VS_ACODEC_G7221C_32:return 80;  // at 32 kbit/s
		case VS_ACODEC_G7221C_48:return 120; // at 48 kbit/s
		case VS_ACODEC_G722:	 return 80;
	}
	return 0;
}

static uint32_t McSecPerFrame(uint32_t tag)
{
	/// at 8000 kHz
	switch(tag)
	{
		case VS_ACODEC_PCM:		 return 10000;
		case VS_ACODEC_G711a:	 return 10000;
		case VS_ACODEC_G711mu:	 return 10000;
		case VS_ACODEC_G723:	 return 30000;
		case VS_ACODEC_G729A:	 return 10000;
		case VS_ACODEC_G728:	 return 10000;
		case VS_ACODEC_G7221_24: return 40000;
		case VS_ACODEC_G7221_32: return	40000;
		case VS_ACODEC_G7221C_24:return 80000;
		case VS_ACODEC_G7221C_32:return 80000;
		case VS_ACODEC_G7221C_48:return 80000;
		case VS_ACODEC_G722:	 return 20000;
	}
	return 0;
}


VS_IppAudiCodec::VS_IppAudiCodec(uint32_t tag, bool coder):AudioCodec(tag, coder, GranOfCodec(tag, coder))
{
	m_pCodecInfo = 0;
	m_hCodecHandle = 0;
	m_pBanks = 0;
	m_iNumBanks = 0;
	switch (tag) {
		case VS_ACODEC_G711a:		m_pUSC_Fxns = &USC_G711A_Fxns; m_iFrameType = 3; break;
		case VS_ACODEC_G711mu:		m_pUSC_Fxns = &USC_G711U_Fxns; m_iFrameType = 3; break;
		case VS_ACODEC_G722:		m_pUSC_Fxns = &USC_G722SB_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G7221_24:	m_pUSC_Fxns = &USC_G722_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G7221_32:	m_pUSC_Fxns = &USC_G722_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G7221C_24:
		case VS_ACODEC_G7221C_32:
		case VS_ACODEC_G7221C_48:	m_pUSC_Fxns = &USC_G722_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G723:		m_pUSC_Fxns = &USC_G723_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G728:		m_pUSC_Fxns = &USC_G728_Fxns; m_iFrameType = 0; break;
		case VS_ACODEC_G729A:		m_pUSC_Fxns = &USC_G729A_Fxns; m_iFrameType = 3; break;
		default : m_pUSC_Fxns = 0; break;
	}
	IppLibInit();
}

VS_IppAudiCodec::~VS_IppAudiCodec()
{
	Release();
}

int VS_IppAudiCodec::Init(WAVEFORMATEX* in)
{
	Release();

	m_cdcfmt = (WAVEFORMATEX*)LOCAL_ALLOC(sizeof(WAVEFORMATEX));
	m_pcmfmt = (WAVEFORMATEX*)LOCAL_ALLOC(sizeof(WAVEFORMATEX));
	m_inBuff = (uint8_t*)LOCAL_ALLOC(m_SIZE);
	m_outBuff = (uint8_t*)LOCAL_ALLOC(m_SIZE);
	m_DataPointer = m_inBuff;
	m_ash.cbStruct = sizeof(ACMSTREAMHEADER);
	m_ash.pbSrc = m_inBuff;
	m_ash.pbDst = m_outBuff;

	m_pcmfmt->wFormatTag = WAVE_FORMAT_PCM;
	m_pcmfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_pcmfmt->wBitsPerSample = 16;
	m_pcmfmt->nChannels = 1;
	m_pcmfmt->nBlockAlign = 2;
	m_pcmfmt->nAvgBytesPerSec = m_pcmfmt->nSamplesPerSec*m_pcmfmt->nBlockAlign;
	m_pcmfmt->cbSize = 0;

	m_cdcfmt->wFormatTag = (uint16_t)GetTag();
	m_cdcfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_cdcfmt->wBitsPerSample = 0;
	m_cdcfmt->nChannels = 1;
	m_cdcfmt->nBlockAlign = (uint16_t)GranOfCodec(GetTag(), false);
	m_cdcfmt->cbSize = 0;
	m_cdcfmt->nAvgBytesPerSec = m_cdcfmt->nBlockAlign*m_cdcfmt->nSamplesPerSec*1000/(McSecPerFrame(GetTag())*8);

	USC_Fxns *pFnxs = (USC_Fxns*)m_pUSC_Fxns;
	int infoSize = 0;

	USC_Status status = (GetTag() == VS_ACODEC_PCM) ? USC_NoError : USC_NotInitialized;
	if (pFnxs != 0) {
		status = pFnxs->std.GetInfoSize(&infoSize);
		if (status == USC_NoError) {
			m_pCodecInfo = LOCAL_ALLOC(infoSize);
			status = pFnxs->std.GetInfo(NULL, m_pCodecInfo);
			USC_CodecInfo *pInfo = (USC_CodecInfo*)m_pCodecInfo;
			if (status == USC_NoError) {
				pInfo->params.direction = (IsCoder()) ? USC_ENCODE : USC_DECODE;
				pInfo->params.pcmType.sample_frequency = in->nSamplesPerSec;
				pInfo->params.pcmType.bitPerSample = in->wBitsPerSample;
				pInfo->params.pcmType.nChannels = in->nChannels;
				pInfo->params.modes.vad = 0;

				if (GetTag() == VS_ACODEC_G7221_24 || GetTag() == VS_ACODEC_G7221C_24)
					pInfo->params.modes.bitrate = 24000;
				if (GetTag() == VS_ACODEC_G7221_32 || GetTag() == VS_ACODEC_G7221C_32)
					pInfo->params.modes.bitrate = 32000;
				if (GetTag() == VS_ACODEC_G7221C_48)
					pInfo->params.modes.bitrate = 48000;
				status = pFnxs->std.NumAlloc(&pInfo->params, &m_iNumBanks);
				if (status == USC_NoError) {
					m_pBanks = LOCAL_ALLOC(sizeof(USC_MemBank) * m_iNumBanks);
					USC_MemBank *pBanks = (USC_MemBank*)m_pBanks;
					status = pFnxs->std.MemAlloc(&pInfo->params, pBanks);
					if (status == USC_NoError) {
						for (int i = 0; i < m_iNumBanks; i++) {
							pBanks[i].pMem = (char*)LOCAL_ALLOC(pBanks[i].nbytes);
						}
						status = pFnxs->std.Init(&pInfo->params, pBanks, &m_hCodecHandle);
						if (status == USC_NoError) {
							status = pFnxs->std.GetInfo(m_hCodecHandle, pInfo);
						}
					}
				}
			}
		}
	}

	m_valid = (status == 0);

	if (!m_valid) Release();

	return status;
}

int VS_IppAudiCodec::ConvertFunction()
{
	int status = 0;
	m_ash.cbSrcLengthUsed = 0;
	m_ash.cbDstLengthUsed = 0;
	uint32_t len = m_ash.cbSrcLength;
	uint32_t tag = GetTag();
	int convertSize = GranOfCodec(tag, !IsCoder());
	USC_PCMStream pcm;
	if (tag != VS_ACODEC_PCM) {
		pcm.pcmType.bitPerSample = m_pcmfmt->wBitsPerSample;
		pcm.pcmType.nChannels = m_pcmfmt->nChannels;
		pcm.pcmType.sample_frequency = m_pcmfmt->nSamplesPerSec;
		pcm.bitrate = ((USC_CodecInfo*)m_pCodecInfo)->params.modes.bitrate;
	}
	while(len>=m_GRAN) {
		void* src = m_ash.pbSrc + m_ash.cbSrcLengthUsed;
		void* dst = m_ash.pbDst + m_ash.cbDstLengthUsed;
		status = IppConvert(src, dst, &pcm, len);
		m_ash.cbSrcLengthUsed += m_GRAN;
		m_ash.cbDstLengthUsed += convertSize;
		len -= m_GRAN;
	}
	return (int) status;
}

int VS_IppAudiCodec::IppConvert(void* src, void* dst, void *pcmInfo, int len)
{
	int status;
	USC_PCMStream *pcm = (USC_PCMStream*)pcmInfo;
	USC_Bitstream strm;
	if (IsCoder()) {
		pcm->pBuffer = (char*)src;
		pcm->nbytes = len;
		strm.pBuffer = (char*)dst;
		status = ((USC_Fxns*)m_pUSC_Fxns)->Encode(m_hCodecHandle, pcm, &strm);
	} else {
		strm.pBuffer = (char*)src;
		strm.nbytes = len;
		strm.bitrate = pcm->bitrate;
		strm.frametype = m_iFrameType;
		pcm->pBuffer = (char*)dst;
		status = ((USC_Fxns*)m_pUSC_Fxns)->Decode(m_hCodecHandle, &strm, pcm);
	}

	return status;
}

void VS_IppAudiCodec::Release()
{
	if (m_pBanks) {
		for (int i = 0; i < m_iNumBanks; i++) {
			LOCAL_FREE(((USC_MemBank*)m_pBanks)[i].pMem);
		}
		LOCAL_FREE(m_pBanks);
		m_pBanks = 0;
		m_iNumBanks = 0;
	}
	if (m_pCodecInfo) LOCAL_FREE(m_pCodecInfo); m_pCodecInfo = 0;
	if (m_inBuff) LOCAL_FREE(m_inBuff); m_inBuff = 0; m_DataPointer = 0;
	if (m_outBuff) LOCAL_FREE(m_outBuff); m_outBuff = 0;
	if (m_cdcfmt) LOCAL_FREE(m_cdcfmt); m_cdcfmt = 0;
	if (m_pcmfmt) LOCAL_FREE(m_pcmfmt); m_pcmfmt = 0;
	memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
	m_valid = false;
}
