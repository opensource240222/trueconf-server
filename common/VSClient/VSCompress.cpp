/**
**************************************************************************
* \file VSCompress.cpp
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief Implementation of Video coding classes
*
* \b Project Client
* \author Melechko Ivan
* \date 07.10.2002
*
* $Revision: 7 $
*
* $History: VSCompress.cpp $
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 15.03.12   Time: 18:37
 * Updated in $/VSNA/VSClient
 * - upgrade system benchmark (lower thresholds for 720p)
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 17:00
 * Updated in $/VSNA/VSClient
 * - add SVC capability
 * - change MediaFormat structure
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 25.06.09   Time: 13:26
 * Updated in $/VSNA/VSClient
 * - bugfix#6163
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 25.03.09   Time: 15:03
 * Updated in $/VSNA/VSClient
 * 5.5 PVC enhancments:
 * - added "adaptive data decode" capability
 * - new bitrate control for data
 *
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 10.10.07   Time: 17:54
 * Updated in $/VS2005/VSClient
 * - fixed XCCodec, VideoCodec (BITMAPINFO)
 * - add XCCodec wrap
 * - change VS_RetriveVideoCodec function
 * - add support h.264 codecs
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Updated in $/VS/VSClient
 * - NHP headers added
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 26.05.06   Time: 14:26
 * Updated in $/VS/VSClient
 * - Nhp new format
 * - send stat in Nhp
 *
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 25.04.06   Time: 18:27
 * Updated in $/VS/VSClient
 * - removed answer-to-die event in Loop() functions
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 16.11.05   Time: 19:04
 * Updated in $/VS/VSClient
 * - calc bitrate for 10 fps now
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 15.11.05   Time: 12:32
 * Updated in $/VS/VSClient
 * - multi video codecs support
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 10.11.05   Time: 19:02
 * Updated in $/VS/VSClient
 * - bug in videocompress disconnect process
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 28.10.05   Time: 18:52
 * Updated in $/VS/VSClient
 * - system (VFW) video codecs support
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 1.07.05    Time: 18:42
 * Updated in $/VS/VSClient
 * in multi conf key frame repeat period set to 10 sec
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VSClientBase.h"
#include "VSCompress.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "Transcoder/VS_VfwVideoCodec.h"
#include "Transcoder/VS_VideoCodecManager.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#define VS_VIDEOHEADER_SIZE		(6)

/****************************************************************************
 * Classes
 ****************************************************************************/

 /**************************************************************************
 * CVideoCompressor
 ****************************************************************************/

void CVideoCompressor::CompressingFrame()
{
	int len = m_pManager->Convert(m_pInpBuffer, m_pBuffer + VS_VIDEOHEADER_SIZE, &m_bKey);
	m_pManager->GetResolutionBaseLayer(&m_baseWidth, &m_baseHeight);
	m_bSuccess = len >= 0;
	if (len > 0) {
		m_pBuffer[0] = m_FrameCounter++;
		m_pBuffer[1] = !m_bKey;
		*(DWORD*)(m_pBuffer + 2) = m_Marker;
	}
	m_ComprSize = len + VS_VIDEOHEADER_SIZE;
}

/**
 **************************************************************************
 * \param	lpParameter	[in] not used
 * \return  zero
 ****************************************************************************/
DWORD CVideoCompressor::Loop(LPVOID hEvDie)
{
	while(1){
		HANDLE handles[2] = { hEvDie, m_hEvRequest };
		DWORD dwRet = WaitForMultipleObjects(2, handles, FALSE, 500);
		if (dwRet == WAIT_OBJECT_0) {
			return NOERROR;
		}
		else if (dwRet == WAIT_OBJECT_0 + 1) {
			CompressingFrame();
			SetEvent(m_hEvAnswer);
		}
		else if (dwRet == WAIT_FAILED) {
			Sleep(50);	/// if this case is possible, wait and try again
		}
	}
	return NOERROR;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCompressor::CVideoCompressor()
{
	m_hEvRequest = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hEvAnswer = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_pBuffer = 0;
	m_pManager = new VS_VideoCodecManager();
	m_Bitrate = 0;
	m_bKey = true;
	m_bSuccess = false;
	m_Marker = 0;
	m_ComprSize = 0;
	m_pInpBuffer = 0;
	m_FrameCounter = 0;
	m_baseWidth = 0;
	m_baseHeight = 0;
	m_eHardwareType = ENCODER_SOFTWARE;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCompressor::~CVideoCompressor()
{
	CloseHandle(m_hEvAnswer);
	CloseHandle(m_hEvRequest);
	if (m_pManager) delete m_pManager;
}

/**
 **************************************************************************
 * \param	Bitrate	[in] set desired bitrate
 * \return  set birate
 ****************************************************************************/
int CVideoCompressor::SetBitrate(int baseBitrate, int maxBitrate, int framerate)
{
	m_pManager->SetBitrate(baseBitrate, maxBitrate, framerate);
	m_Bitrate = baseBitrate;
	return m_Bitrate;
}

/**
 **************************************************************************
 * \param	pvmf	[in] input video data description
 * \param	codecID	[in] video coder subtype(mode)
 * \return  0 if no error
 *			-1 in case of coder initialisztion error
 ****************************************************************************/
int CVideoCompressor::ConnectToVideoCompressor(VS_MediaFormat *pvmf)
{
	DisconnectToVideoCompressor();
	if (m_pManager->Init(pvmf)) {
		m_pManager->GetResolutionBaseLayer(&m_baseWidth, &m_baseHeight);
		m_pBuffer = new unsigned char [pvmf->dwVideoWidht*pvmf->dwVideoHeight*3+VS_VIDEOHEADER_SIZE];
		m_eHardwareType = (eHardwareEncoder)pvmf->dwHWCodec;
		if (m_eHardwareType != ENCODER_H264_LOGITECH) {
			if (!ActivateThread((LPVOID)this)) {
				m_pManager->Release();
				delete [] m_pBuffer; m_pBuffer = 0;
				return -1;
			}
		}
		return 0;
	}
	return -1;
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoCompressor::DisconnectToVideoCompressor()
{
	DesactivateThread();
	ResetEvent(m_hEvAnswer);
	ResetEvent(m_hEvRequest);
	m_pManager->Release();
	delete [] m_pBuffer; m_pBuffer = 0;
	m_Bitrate = 0;
	m_bKey = true;
	m_bSuccess = false;
	m_Marker = 0;
	m_ComprSize = 0;
	m_pInpBuffer = 0;
	m_FrameCounter = 0;
	m_eHardwareType = ENCODER_SOFTWARE;
	return 0;
}

/**
 **************************************************************************
 * \param	pBuffer	[in] pointer to input video data
 * \param	bKey	[in] request key frame from vodeocoder
 * \param	Marker	[in] reserved data
 * \return  zero
 ****************************************************************************/
int CVideoCompressor::CompressFrame(unsigned char *pBuffer, bool &bKey, DWORD Marker)
{
	m_pInpBuffer = pBuffer;
	m_bKey = bKey;
	m_bSuccess = TRUE;
	m_Marker = Marker;
	if (m_eHardwareType == ENCODER_H264_LOGITECH) {
		CompressingFrame();
		bKey = m_bKey;
	} else {
		m_ComprSize = 0;
		SetEvent(m_hEvRequest);
	}
	return 0;
}

/**
 **************************************************************************
 * \param	pSize	[out] pointer to compressed frame size
 * \param	pbKey	[out] pointer to compressed frame type
 * \return  pointer to compressed data buffer
 ****************************************************************************/
unsigned char *CVideoCompressor::GetCompressedData(int *pSize, bool *pbKey, int *pw, int *ph)
{
	*pbKey = m_bKey;
	*pSize = m_ComprSize;
	*pw = m_baseWidth;
	*ph = m_baseHeight;
	return m_pBuffer;
}


 /**************************************************************************
 * CVideoCompressor
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CVideoDecompressor::CVideoDecompressor()
{
	m_pCodec = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoDecompressor::~CVideoDecompressor()
{
	delete m_pCodec; m_pCodec = 0;
}

/**
 **************************************************************************
 * \param	pvmf	[in] input video data description
 * \return  0 if no error
 *			-1 in case of decoder initialisztion error
 ****************************************************************************/
int CVideoDecompressor::ConnectToVideoDecompressor(VS_MediaFormat *pvmf)
{
	int ret = -1;
	m_pCodec = VS_RetriveVideoCodec(pvmf->dwVideoCodecFCC, false, pvmf->dwHWCodec);
	if (m_pCodec) {
		int mulWidth = (pvmf->dwVideoCodecFCC == VS_VCODEC_VPXSTEREO) ? 2 : 1;
		if (m_pCodec->Init(pvmf->dwVideoWidht, pvmf->dwVideoHeight * mulWidth, FOURCC_I420, 0) == 0) {
			ret = 0;
		}
		if (ret != 0) {
			delete m_pCodec; m_pCodec = 0;
		}
	}
	return ret;
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoDecompressor::DisconnectToVideoDecompressor()
{
	delete m_pCodec; m_pCodec = 0;
	return 0;
}

/**
 **************************************************************************
 * \param	pBuffer	[out] pointer to decompressed video frame
 * \param	bKey	[in] say if compressed frame was Key Frame
 * \param	size	[in] compressed frame size
 * \return  0 if no error
 *			-1 in case of decoder processing error
 ****************************************************************************/
int CVideoDecompressor::DecompressFrame(unsigned char *in, int size, bool bKey, unsigned char *out)
{
	VS_VideoCodecParam prm;
	prm.dec.Flags = bKey ? 0 : ICDECOMPRESS_NOTKEYFRAME;
	prm.dec.FrameSize = size;
	if (m_pCodec && m_pCodec->Convert(in, out, &prm) >= 0)
		return 0;
	else
		return -1;
}

