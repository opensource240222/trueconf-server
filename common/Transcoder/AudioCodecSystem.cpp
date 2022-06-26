#ifdef _WIN32
#include "AudioCodecSystem.h"

/******************************************************************************
* Constructor.
* \param	outTag			- codec to use (defined in mmreg.h)
* \param	coder			- coder or decoder functionality
* \date    31-07-2009
******************************************************************************/
AudioCodecSystem::AudioCodecSystem(DWORD outTag, bool coder, DWORD GRAN) :AudioCodec(outTag, coder, GRAN)
{
	m_had = 0;
	m_has = 0;
}

/******************************************************************************
* Destructor. Release resources
* \date    31-07-2009
******************************************************************************/
AudioCodecSystem::~AudioCodecSystem()
{
	Release();
}

/******************************************************************************
* Init codec and test compatibility
* \param	in				- input wavw format to convert
* \date    31-07-2009
******************************************************************************/
int AudioCodecSystem::Init(WAVEFORMATEX* in)
{
	Release();

	m_pcmfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	memset(m_pcmfmt, 0, sizeof(WAVEFORMATEX));
	m_pcmfmt->cbSize = 0;
	m_pcmfmt->wFormatTag = WAVE_FORMAT_PCM;
	m_pcmfmt->nSamplesPerSec = in->nSamplesPerSec;
	m_pcmfmt->nChannels = in->nChannels;
	m_pcmfmt->wBitsPerSample = 16;
	m_pcmfmt->nBlockAlign = 2 * m_pcmfmt->nChannels;
	m_pcmfmt->nAvgBytesPerSec = m_pcmfmt->nBlockAlign*m_pcmfmt->nSamplesPerSec;

	WORD tag = (WORD)GetTag();
	int size = in->cbSize;
	switch (tag) // presets for system decoders in client
	{
	case VS_ACODEC_GSM610:
		size = 2;
		m_cdcfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX) + size);
		m_cdcfmt->wBitsPerSample = 0;
		m_cdcfmt->nChannels = in->nChannels;
		m_cdcfmt->nSamplesPerSec = in->nSamplesPerSec;
		m_cdcfmt->nBlockAlign = 65;
		m_cdcfmt->nAvgBytesPerSec = (in->nSamplesPerSec * m_cdcfmt->nBlockAlign) / 320;
		*(unsigned short*)((unsigned char*)m_cdcfmt + sizeof(WAVEFORMATEX)) = 320;
		break;
	case 0xcd04:
		size = 0;
		m_cdcfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX) + size);
		m_cdcfmt->wBitsPerSample = 0;
		m_cdcfmt->nChannels = 1;
		m_cdcfmt->nSamplesPerSec = in->nSamplesPerSec;
		m_cdcfmt->nBlockAlign = (unsigned short)(in->nSamplesPerSec / 16000 * 60);
		m_cdcfmt->nAvgBytesPerSec = 3000;
		break;
	default:
		m_cdcfmt = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX) + size);
		memcpy(m_cdcfmt, in, sizeof(WAVEFORMATEX) + size);
		break;
	}
	m_cdcfmt->cbSize = size;
	m_cdcfmt->wFormatTag = tag;

	acmDriverEnum(acmDriverEnumCallback, (DWORD_PTR)this, 0);
	if (!m_valid) return -1;
	m_DataPointer = m_inBuff;
	return 0;
}

/******************************************************************************
* Release all resources
* \date    31-07-2009
******************************************************************************/
void AudioCodecSystem::Release()
{
	if (m_pcmfmt) free(m_pcmfmt); m_pcmfmt = 0;
	if (m_cdcfmt) free(m_cdcfmt); m_cdcfmt = 0;
	if (m_inBuff) free(m_inBuff); m_inBuff = 0;
	if (m_outBuff) free(m_outBuff); m_outBuff = 0; m_DataPointer = 0;
	if (m_valid) acmStreamUnprepareHeader(m_has, &m_ash, 0); memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
	if (m_has) acmStreamClose(m_has, 0); m_has = 0;
	if (m_had) acmDriverClose(m_had, 0); m_had = 0;
	m_valid = false;
}

/******************************************************************************
* A callback function used with the acmDriverEnum function.
* \param	hadid		- Handle to an ACM driver identifier.
* \param	dwInstance	- Application-defined value specified in acmDriverEnum.
* \param	fdwSupport	- Driver-support flags specific to the driver specified by hadid
* \return	TRUE to continue enumeration or FALSE to stop enumeration.
* \date    16-04-2004
******************************************************************************/
BOOL CALLBACK AudioCodecSystem::acmDriverEnumCallback(HACMDRIVERID hadid, DWORD_PTR dwInstance, DWORD fdwSupport)
{
	MMRESULT mRes = 0;
	AudioCodecSystem* pCodec = (AudioCodecSystem*)dwInstance;

	if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC) {
		int maxSize;
		if (acmDriverOpen(&pCodec->m_had, hadid, 0) != MMSYSERR_NOERROR) return TRUE;
		mRes = acmMetrics((HACMOBJ)pCodec->m_had, ACM_METRIC_MAX_SIZE_FORMAT, &maxSize);
		if (mRes == MMSYSERR_NOERROR) {
			if (maxSize < sizeof(WAVEFORMATEX)) maxSize = sizeof(WAVEFORMATEX);
			WAVEFORMATEX *pSrcFmt, *pDstFmt;
			pDstFmt = (LPWAVEFORMATEX)malloc(maxSize);
			if (pCodec->IsCoder()) {
				pSrcFmt = pCodec->m_pcmfmt;
				if (maxSize > (int)(sizeof(WAVEFORMATEX) + pCodec->m_cdcfmt->cbSize)) maxSize = sizeof(WAVEFORMATEX) + pCodec->m_cdcfmt->cbSize;
				memcpy(pDstFmt, pCodec->m_cdcfmt, maxSize);
			}
			else {
				pSrcFmt = pCodec->m_cdcfmt;
				memcpy(pDstFmt, pCodec->m_pcmfmt, sizeof(WAVEFORMATEX));
			}
			DWORD fdwSuggest = ACM_FORMATSUGGESTF_WFORMATTAG | ACM_FORMATSUGGESTF_NSAMPLESPERSEC | ACM_FORMATSUGGESTF_NCHANNELS;
			mRes = (pDstFmt->wFormatTag == 0xcd04) ? 0 : acmFormatSuggest(pCodec->m_had, pSrcFmt, pDstFmt, maxSize, fdwSuggest);
			if (mRes == MMSYSERR_NOERROR) {
				mRes = acmStreamOpen(&pCodec->m_has, pCodec->m_had, pSrcFmt, pDstFmt, NULL, 0, 0, 0);
				if (mRes == MMSYSERR_NOERROR) {
					pCodec->m_inBuff = (BYTE*)malloc(m_SIZE);
					pCodec->m_outBuff = (BYTE*)malloc(m_SIZE);
					pCodec->m_ash.cbStruct = sizeof(ACMSTREAMHEADER);
					pCodec->m_ash.fdwStatus = 0;
					pCodec->m_ash.dwUser = 0;
					pCodec->m_ash.pbSrc = pCodec->m_inBuff;
					pCodec->m_ash.cbSrcLength = m_SIZE;
					pCodec->m_ash.dwSrcUser = 0;
					pCodec->m_ash.pbDst = pCodec->m_outBuff;
					pCodec->m_ash.cbDstLength = m_SIZE;
					pCodec->m_ash.dwSrcUser = 0;
					mRes = acmStreamPrepareHeader(pCodec->m_has, &pCodec->m_ash, 0);
					if (mRes == MMSYSERR_NOERROR) {
						if (pCodec->IsCoder()) {
							if (pCodec->m_cdcfmt) free(pCodec->m_cdcfmt);
							pCodec->m_cdcfmt = pDstFmt;
						}
						else {
							if (pCodec->m_pcmfmt) free(pCodec->m_pcmfmt);
							pCodec->m_pcmfmt = pDstFmt;
						}
						pCodec->m_valid = true;
						return FALSE;
					}
					free(pCodec->m_inBuff); pCodec->m_inBuff = 0;
					free(pCodec->m_outBuff); pCodec->m_outBuff = 0;
					acmStreamClose(pCodec->m_has, 0); pCodec->m_has = 0;
				}
			}
			free(pDstFmt);
		}
		acmDriverClose(pCodec->m_had, 0); pCodec->m_had = 0;
	}

	return TRUE;
}

/******************************************************************************
* internal conversion function
* \date    20-04-2004
******************************************************************************/
int	AudioCodecSystem::ConvertFunction()
{
	return acmStreamConvert(m_has, &m_ash, 0);
}

#endif
