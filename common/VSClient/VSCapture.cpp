/**
 **************************************************************************
 * \file VSCapture.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Classes used to capture videodata from videocapture device
 *
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2004
 *
 * $Revision: 45 $
 *
 * $History: VSCapture.cpp $
 *
 * *****************  Version 45  *****************
 * User: Sanufriev    Date: 31.07.12   Time: 12:14
 * Updated in $/VSNA/VSClient
 * - fix h.264 dec
 * - fix AR resampling
 *
 * *****************  Version 44  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 43  *****************
 * User: Sanufriev    Date: 13.04.12   Time: 12:38
 * Updated in $/VSNA/VSClient
 * - fix stereo capture
 *
 * *****************  Version 42  *****************
 * User: Sanufriev    Date: 13.04.12   Time: 12:04
 * Updated in $/VSNA/VSClient
 * - fix stereo capture
 *
 * *****************  Version 41  *****************
 * User: Sanufriev    Date: 13.04.12   Time: 11:41
 * Updated in $/VSNA/VSClient
 * - fix stereo capture
 *
 * *****************  Version 40  *****************
 * User: Sanufriev    Date: 13.04.12   Time: 11:39
 * Updated in $/VSNA/VSClient
 * - fix stereo capture
 *
 * *****************  Version 39  *****************
 * User: Sanufriev    Date: 19.03.12   Time: 18:47
 * Updated in $/VSNA/VSClient
 * - improve csc input format (ipp wrap)
 *
 * *****************  Version 38  *****************
 * User: Sanufriev    Date: 15.03.12   Time: 11:33
 * Updated in $/VSNA/VSClient
 * - fix AR resample for HD Source
 *
 * *****************  Version 37  *****************
 * User: Sanufriev    Date: 2.11.11    Time: 14:12
 * Updated in $/VSNA/VSClient
 * - were added capability for capture devices : framerate list &
 * interlaced flag
 *
 * *****************  Version 36  *****************
 * User: Sanufriev    Date: 28.10.11   Time: 13:38
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 35  *****************
 * User: Sanufriev    Date: 24.10.11   Time: 12:33
 * Updated in $/VSNA/VSClient
 * - fix csc directshow
 *
 * *****************  Version 34  *****************
 * User: Sanufriev    Date: 24.10.11   Time: 11:15
 * Updated in $/VSNA/VSClient
 * - ench csc module for directshow
 *
 * *****************  Version 33  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/VSClient
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 32  *****************
 * User: Sanufriev    Date: 23.09.11   Time: 11:13
 * Updated in $/VSNA/VSClient
 * - fix camera "None"
 *
 * *****************  Version 31  *****************
 * User: Sanufriev    Date: 7.09.11    Time: 15:19
 * Updated in $/VSNA/VSClient
 * - fix capture caps for WebcamXP
 *
 * *****************  Version 30  *****************
 * User: Sanufriev    Date: 28.07.11   Time: 15:48
 * Updated in $/VSNA/VSClient
 * - fix capture : CloseHandle
 *
 * *****************  Version 29  *****************
 * User: Sanufriev    Date: 25.07.11   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - fix auto mode
 *
 * *****************  Version 28  *****************
 * User: Sanufriev    Date: 25.07.11   Time: 12:47
 * Updated in $/VSNA/VSClient
 * - were adde support HDYC color format
 *
 * *****************  Version 27  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 26  *****************
 * User: Sanufriev    Date: 23.06.11   Time: 14:23
 * Updated in $/VSNA/VSClient
 * - fix enumerate bug in directshow
 * - fix bug: av null name in capture list
 * - self view fps to 60
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 19.06.11   Time: 16:20
 * Updated in $/VSNA/VSClient
 * - fix freeze video after call camera property page
 *
 * *****************  Version 24  *****************
 * User: Sanufriev    Date: 27.04.11   Time: 19:14
 * Updated in $/VSNA/VSClient
 * - were added auto change media format
 * - were added info media format command
 * - wait time reduced to 1000 ms in EventManager
 * - were added new capability : dynamic change media format
 * - capture : unblock SetFps and GetFrame
 * - receivers can dynamic change media format
 * - were added auto check media format from bitstream in receivers
 * - change scheme BtrVsFPS for vpx
 * - change AviWriter for auto change media format
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 12.04.11   Time: 17:09
 * Updated in $/VSNA/VSClient
 * - av in video fixed
 *
 * *****************  Version 22  *****************
 * User: Sanufriev    Date: 29.03.11   Time: 13:08
 * Updated in $/VSNA/VSClient
 * - update Capture module (STA implementation)
 *
 * *****************  Version 21  *****************
 * User: Sanufriev    Date: 14.03.11   Time: 14:27
 * Updated in $/VSNA/vsclient
 * - change VS_MediaFormat - were added dwFps
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 11.03.11   Time: 12:52
 * Updated in $/VSNA/VSClient
 * - change method retrive camera propety, pins pages
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 16:56
 * Updated in $/VSNA/VSClient
 * - directshow fix : camera propety, pins pages
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 9.02.11    Time: 17:21
 * Updated in $/VSNA/VSClient
 * - were added case of change fps in camera
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 13.01.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - ench #8342 (refactoring camera states)
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 20.12.10   Time: 14:41
 * Updated in $/VSNA/VSClient
 * - were add Blend deinterlace init
 *
 * *****************  Version 14  *****************
 * User: Melechko     Date: 17.12.10   Time: 20:19
 * Updated in $/VSNA/VSClient
 * Add support virtual cameras
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 2.12.10    Time: 16:14
 * Updated in $/VSNA/VSClient
 * - increase accuracy video capture timestamp
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 1.12.10    Time: 20:47
 * Updated in $/VSNA/VSClient
 * - possible fix for NULL variant error
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 3.11.10    Time: 12:58
 * Updated in $/VSNA/VSClient
 * - enable SSE2 deinterlace
 *
 * *****************  Version 10  *****************
 * User: Dront78      Date: 11.01.10   Time: 14:45
 * Updated in $/VSNA/VSClient
 * - solution merge vzo7 to add unicode devices support
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 20:58
 * Updated in $/VSNA/VSClient
 * - change set caps for video in the capture
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 21.12.09   Time: 13:44
 * Updated in $/VSNA/VSClient
 * - unicode capability ReadParam
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 23.11.09   Time: 13:05
 * Updated in $/VSNA/VSClient
 * - fixed crash on conference stop VZOchat7
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 28.10.09   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - directx dshow headers changed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 16.06.09   Time: 14:02
 * Updated in $/VSNA/VSClient
 * - add lock in device status query
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 12.03.09   Time: 15:28
 * Updated in $/VSNA/VSClient
 * - camera swtched of in tray
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 20.03.08   Time: 17:03
 * Updated in $/VSNA/VSClient
 * - Video for Linux I420 support added via memory mapped files.
 * Compilation controls via #define VS_LINUX_DEVICE in VSCapture.h
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/VSClient
 * - device statuses added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:42
 * Updated in $/VS2005/VSClient
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 66  *****************
 * User: Smirnov      Date: 24.07.06   Time: 16:56
 * Updated in $/VS/VSClient
 * - added deintrlacing algorithm
 *
 * *****************  Version 65  *****************
 * User: Melechko     Date: 16.05.06   Time: 12:00
 * Updated in $/VS/VSClient
 * Add support BJPG video format
 *
 * *****************  Version 64  *****************
 * User: Smirnov      Date: 4.05.06    Time: 12:40
 * Updated in $/VS/VSClient
 * - hardware test repaired
 *
 * *****************  Version 63  *****************
 * User: Smirnov      Date: 3.05.06    Time: 18:09
 * Updated in $/VS/VSClient
 * - removed 1 frame latency in video capture device
 * - removed null (first) frame rendering after capture deinit
 * - InvertUv registry param moved
 *
 * *****************  Version 62  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/VSClient
 * - sender reinitialisation
 *
 * *****************  Version 61  *****************
 * User: Smirnov      Date: 12.04.06   Time: 16:47
 * Updated in $/VS/VSClient
 * - hardware endpoint properties repaired
 *
 * *****************  Version 60  *****************
 * User: Smirnov      Date: 21.02.06   Time: 17:02
 * Updated in $/VS/VSClient
 * - added preferred video capture dimension flag
 *
 * *****************  Version 59  *****************
 * User: Smirnov      Date: 28.11.05   Time: 18:17
 * Updated in $/VS/VSClient
 * - scale captured image instead clip it
 *
 * *****************  Version 58  *****************
 * User: Smirnov      Date: 25.11.05   Time: 18:22
 * Updated in $/VS/VSClient
 *  - DVC90 352x288 bad picture Fix
 *
 * *****************  Version 57  *****************
 * User: Smirnov      Date: 25.11.05   Time: 17:13
 * Updated in $/VS/VSClient
 * - frame rate will not rise more then rgistry frame rate
 *
 * *****************  Version 56  *****************
 * User: Smirnov      Date: 7.06.05    Time: 18:35
 * Updated in $/VS/VSClient
 * added support for larger then captured dimensions
 *
 * *****************  Version 55  *****************
 * User: Melechko     Date: 25.02.05   Time: 15:00
 * Updated in $/VS/VSClient
 * Add DV support
 *
 * *****************  Version 54  *****************
 * User: Melechko     Date: 16.02.05   Time: 14:21
 * Updated in $/VS/VSClient
 * Add MJPEG support
 *
 * *****************  Version 53  *****************
 * User: Melechko     Date: 25.01.05   Time: 13:38
 * Updated in $/VS/VSClient
 * split input video buffer
 *
 * *****************  Version 52  *****************
 * User: Smirnov      Date: 19.01.05   Time: 20:25
 * Updated in $/VS/VSClient
 * set sps in calc bitrate
 *
 * *****************  Version 51  *****************
 * User: Melechko     Date: 19.01.05   Time: 18:01
 * Updated in $/VS/VSClient
 * Add re-init audio format
 *
 * *****************  Version 50  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 49  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSCapture.h"
#include "Video/VSVideoProc.h"
#include "Video/Deinterlacing.h"
#include "../std/VS_ProfileTools.h"
#include "VSVideoCaptureList.h"
#include "VS_Dmodule.h"
/// videocodec include
#include "../Transcoder/VideoCodec.h"
#include "../MediaParserLib/VS_H264Parser.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#ifndef HAVE_JPEG
#define HAVE_JPEG
#endif
#include "libyuv/include/libyuv.h"


HINSTANCE	g_hMf = NULL;
HINSTANCE	g_hMfplat = NULL;
HINSTANCE	g_hMfreadwrite = NULL;
HINSTANCE	g_hMfuuid = NULL;

LPMFStartup								g_MFStartup = NULL;
LPMFShutdown							g_MFShutdown = NULL;
LPMFEnumDeviceSources					g_MFEnumDeviceSources = NULL;
LPMFCreateAttributes					g_MFCreateAttributes = NULL;
LPMFCreateDeviceSource					g_MFCreateDeviceSource = NULL;
LPMFPutWorkItem							g_MFPutWorkItem = NULL;
LPMFCreateAsyncResult					g_MFCreateAsyncResult = NULL;
LPMFCreateSourceReaderFromMediaSource	g_MFCreateSourceReaderFromMediaSource = NULL;
LPMFCreateMemoryBuffer					g_MFCreateMemoryBuffer = NULL;
LPMFCreateMediaType						g_MFCreateMediaType = NULL;
LPMFInvokeCallback						g_MFInvokeCallback = NULL;
LPMFAllocateSerialWorkQueue				g_MFAllocateSerialWorkQueue = NULL;
LPMFUnlockWorkQueue						g_MFUnlockWorkQueue = NULL;

/******************************************************************************
 * CCaptureCConv
 ******************************************************************************/
/**
 ****************************************************************************
 * Constructor
 * \date    13-02-2004
 ******************************************************************************/
CCaptureCConv::CCaptureCConv() {
	memset(this, 0, sizeof(CCaptureCConv));
	m_proc = new VS_VideoProc;
	m_DeInt = new VS_Deinterlacing((m_proc->Cpu() & VS_CPU_SSE2) ? VS_Deinterlacing::SSE2 :
								   ((m_proc->Cpu() & VS_CPU_MMX) ? VS_Deinterlacing::MMX : VS_Deinterlacing::C));
}

/**
 ****************************************************************************
 * Destructor
 * \date    13-02-2004
 ******************************************************************************/
CCaptureCConv::~CCaptureCConv() {
	delete m_DeInt;
	delete m_proc;
	if (m_dCaptureFlags & (RESAMPLE_OUTPUT | CROP_OUTPUT | AR_OUTPUT | AR_IN_OUTPUT)) free(m_pClip);
	if (m_pClipRsmpl) free(m_pClipRsmpl);
	if (m_dCaptureFlags & CSC_INPUT) free(m_pConv);
	if (m_dCaptureFlags & DEINTERLACE) free(m_pDeint);
	if (m_dCaptureFlags & NOT_SRC_INPUT) free(m_pCaptureFrame);
	if (m_dCaptureFlags & COMPRESS_INPUT) {
		free(m_pOutCompress);
		delete m_pVideoDecoder;
	}
}

#define IPP_RESIZE

/**
 ****************************************************************************
 * Init Input Image Format recieved from capture device
 * \date    13-02-2004
 ******************************************************************************/
bool CCaptureCConv::SetInFormat(LPBITMAPINFOHEADER pbih, unsigned char* pCaptureFrame, int deint) {
	if (m_Inited == SET_INIT) m_Inited = 0;
	if (m_dCaptureFlags & NOT_SRC_INPUT) free(m_pCaptureFrame); m_pCaptureFrame = 0;
	if (m_dCaptureFlags & CSC_INPUT) free(m_pConv); m_pConv = 0;
	if (m_dCaptureFlags & COMPRESS_INPUT) {
		free(m_pOutCompress); m_pOutCompress = 0;
		delete m_pVideoDecoder; m_pVideoDecoder = 0;
	}
	if (m_dCaptureFlags & DEINTERLACE) free(m_pDeint); m_pDeint = 0;
	m_pCaptureFrame = pCaptureFrame;
	memcpy(&m_bihIn,pbih,sizeof(BITMAPINFOHEADER));
	m_DeInt->Release();
	if (m_bihIn.biCompression != CColorSpace::FCC_I420 && m_bihIn.biCompression != CColorSpace::FCC_IYUV) {
		m_pConv = (unsigned char*)malloc(m_bihIn.biHeight*m_bihIn.biWidth*4);
		memset(m_pConv, 0, m_bihIn.biHeight * m_bihIn.biWidth);
		memset(m_pConv + m_bihIn.biHeight  *m_bihIn.biWidth, 0x80, 3 * m_bihIn.biHeight * m_bihIn.biWidth);
	}
	if (deint > 0) {
		m_pDeint = (unsigned char*)malloc(m_bihIn.biHeight*m_bihIn.biWidth*3/2);
		m_DeInt->Init(m_bihIn.biWidth, m_bihIn.biHeight, m_bihIn.biWidth, m_bihIn.biWidth/2, (deint == 1) ? VS_Deinterlacing::QUALITY : VS_Deinterlacing::BLEND);
	}
	m_Inited |= SET_INPUT;
	return true;
}

/**
 ****************************************************************************
 * Reset Input Image Format recieved from capture device
 * \date    13-02-2004
 ******************************************************************************/
bool CCaptureCConv::ResetInFormat(LPBITMAPINFOHEADER pbih, unsigned char* pCaptureFrame, int deint) {
	m_Inited &= ~SET_CHECK;
	if (m_dCaptureFlags & NOT_SRC_INPUT) free(m_pCaptureFrame); m_pCaptureFrame = 0;
	if (m_dCaptureFlags & CSC_INPUT) free(m_pConv); m_pConv = 0;
	if (m_dCaptureFlags & COMPRESS_INPUT) {
		free(m_pOutCompress); m_pOutCompress = 0;
		delete m_pVideoDecoder; m_pVideoDecoder = 0;
	}
	if (m_dCaptureFlags & DEINTERLACE) free(m_pDeint); m_pDeint = 0;
	m_pCaptureFrame = pCaptureFrame;
	memcpy(&m_bihIn,pbih,sizeof(BITMAPINFOHEADER));
	m_DeInt->Release();
	if (m_bihIn.biCompression != CColorSpace::FCC_I420 && m_bihIn.biCompression != CColorSpace::FCC_IYUV) {
		m_pConv = (unsigned char*)malloc(m_bihIn.biHeight*m_bihIn.biWidth*4);
		memset(m_pConv, 0, m_bihIn.biHeight * m_bihIn.biWidth);
		memset(m_pConv + m_bihIn.biHeight  *m_bihIn.biWidth, 0x80, 3 * m_bihIn.biHeight * m_bihIn.biWidth);
	}
	if (deint > 0) {
		m_pDeint = (unsigned char*)malloc(m_bihIn.biHeight*m_bihIn.biWidth*3/2);
		m_DeInt->Init(m_bihIn.biWidth, m_bihIn.biHeight, m_bihIn.biWidth, m_bihIn.biWidth/2, (deint == 1) ? VS_Deinterlacing::QUALITY : VS_Deinterlacing::BLEND);
	}
	m_Inited |= SET_INPUT;
	return CheckFormats();
}

/**
 ****************************************************************************
 * Init Otput Image Format
 * \date    13-02-2004
 ******************************************************************************/
bool CCaptureCConv::SetOutFormat(CColorMode &cm, int CrpType) {
	if (m_Inited == SET_INIT) m_Inited = 0;
	cm.ColorModeToBitmapInfoHeader(&m_bihOut);
	m_Inited |= SET_OUTPUT;
	m_CrpType = CrpType;
	return true;
}

/**
 ****************************************************************************
 * Reset Otput Image Format
 * \date    13-02-2004
 ******************************************************************************/
bool CCaptureCConv::ResetOutFormat(CColorMode &cm, int CrpType) {
	m_Inited &= ~SET_CHECK;
	if ((m_dCaptureFlags & DEINTERLACE) == 0) m_pDeint = 0;
	if ((m_dCaptureFlags & CSC_INPUT) == 0) m_pConv = 0;
	cm.ColorModeToBitmapInfoHeader(&m_bihOut);
	m_Inited |= SET_OUTPUT;
	m_CrpType = CrpType;
	m_Inited |= SET_OUTPUT;
	return CheckFormats();
}

bool CCaptureCConv::CheckFormats()
{
	if (m_Inited == 3) {
		m_fFactorW = 1.0;
		m_fFactorH = 1.0;
		if (m_dCaptureFlags & (RESAMPLE_OUTPUT | CROP_OUTPUT | AR_OUTPUT | AR_IN_OUTPUT)) free(m_pClip); m_pClip = 0;
		if (m_pClipRsmpl) free(m_pClipRsmpl); m_pClipRsmpl = 0;
		m_dCaptureFlags = 0x0;

		if (m_bihOut.biCompression == CColorSpace::FCC_I42S) m_dCaptureFlags |= STEREO_UD_OUTPUT;
		if (m_bihIn.biCompression == CColorSpace::FCC_STR0) m_dCaptureFlags |= STEREO_UD_INPUT;
		if (!m_pCaptureFrame) {
			m_dCaptureFlags |= NOT_SRC_INPUT;
			m_pCaptureFrame = (unsigned char*)malloc(m_bihIn.biSizeImage); // size to max (rgb32)
		}
		if (m_bihIn.biCompression == CColorSpace::FCC_H264) {
			m_pVideoDecoder = VS_RetriveVideoCodec(VS_VCODEC_H264, false);
			if (m_pVideoDecoder) {
				m_pVideoDecoder->Init(m_bihIn.biWidth, m_bihIn.biHeight, CColorSpace::FCC_I420);
				m_pOutCompress = (unsigned char*)malloc(m_bihIn.biSizeImage);
				m_dCaptureFlags |= COMPRESS_INPUT;
			}
		}
		if (m_pConv) m_dCaptureFlags |= CSC_INPUT;
		else m_pConv = m_pCaptureFrame;
		if (m_pDeint) m_dCaptureFlags |= DEINTERLACE;
		else m_pDeint = m_pConv;

		m_inHeight = m_bihIn.biHeight;
		m_outHeight = m_bihOut.biHeight;
		if (m_dCaptureFlags & STEREO_UD_INPUT) {
			m_inHeight /= 2;
		}
		if (m_dCaptureFlags & STEREO_UD_OUTPUT) {
			m_outHeight /= 2;
		}

		if (m_bihIn.biWidth == m_bihOut.biWidth && m_inHeight == m_outHeight) {
			m_dCaptureFlags |= IDLE_OUTPUT;
			m_pClip = m_pDeint;
		} else {
			if (m_CrpType == 0) m_dCaptureFlags |= RESAMPLE_OUTPUT;
			else if (m_CrpType == 1) m_dCaptureFlags |= AR_IN_OUTPUT;
			else if (m_CrpType == 5) m_dCaptureFlags |= CROP_OUTPUT;
			else if (m_CrpType == 6) m_dCaptureFlags |= AR_OUTPUT;
			m_pClip = (unsigned char*)malloc(m_bihOut.biHeight*m_bihOut.biWidth*3/2);
		}

		m_pOut = m_pClip;

		if (m_dCaptureFlags & (AR_OUTPUT | AR_IN_OUTPUT)) {
			double xk = (double)m_bihIn.biWidth / (double)m_bihOut.biWidth,
				   yk = (double)m_inHeight / (double)m_outHeight;

#ifdef IPP_RESIZE
			if (m_dCaptureFlags & AR_IN_OUTPUT) {
				if (xk <= 1 && yk <= 1) {
					m_outWidthAR = m_bihIn.biWidth;
					m_outHeightAR = m_inHeight;
					m_iOffsetW = (m_bihOut.biWidth - m_bihIn.biWidth) / 2;
					m_iOffsetH = (m_outHeight - m_inHeight) / 2;
					m_fFactorW = 1.0;
					m_fFactorH = 1.0;
				} else if (xk >= yk) {
					m_outWidthAR = m_bihOut.biWidth;
					m_outHeightAR = ((int)((double)m_inHeight / xk + 1.5)) &~ 1;
					m_iOffsetW = 0;
					m_iOffsetH = (m_outHeight - m_outHeightAR) / 2;
					m_fFactorW = 1 / xk;
					m_fFactorH = 1 / xk;
				} else if (yk >= xk) {
					m_outWidthAR = ((int)((double)m_bihIn.biWidth / yk + 1.5)) &~ 1;
					m_outHeightAR = m_outHeight;
					m_iOffsetW = (m_bihOut.biWidth - m_outWidthAR) / 2;
					m_iOffsetH = 0;
					m_fFactorW = 1 / yk;
					m_fFactorH = 1 / yk;
				}
			} else {
				m_outWidthAR = m_bihIn.biWidth;
				m_outHeightAR = m_inHeight;
				m_iOffsetW = 0;
				m_iOffsetH = 0;
				if (yk < xk) {
					m_outWidthAR = ((int)(m_bihOut.biWidth * yk + 1.5)) &~ 1;
					m_iOffsetW = (m_bihIn.biWidth - m_outWidthAR) / 2;
					m_fFactorH = 1.0 / yk;
					m_fFactorW = (double)m_bihOut.biWidth / (double)m_outWidthAR;
				} else if (xk <= yk) {
					m_outHeightAR = ((int)(m_outHeight * xk + 1.5)) &~ 1;
					m_iOffsetH = (m_inHeight - m_outHeightAR) / 2;
					m_fFactorW = 1.0 / xk;
					m_fFactorH = (double)m_outHeight / (double)m_outHeightAR;
				}
			}
			if (xk > 1.0 && yk > 1.0) {
				m_rMode = IPPI_INTER_LINEAR;
			} else {
				m_rMode = IPPI_INTER_CUBIC;
			}
#else
			m_outWidthAR = m_bihOut.biWidth;
			m_outHeightAR = m_outHeight;
			if (yk < xk) {
				m_outWidthAR = ((int)(((m_bihOut.biWidth * xk) / yk + 0.5))) &~ 15;
			} else if (xk <= yk) {
				m_outHeightAR = ((int)(((m_outHeight * yk) / xk + 0.5))) &~ 15;
			}
			m_pClipRsmpl = (unsigned char*)malloc(m_outWidthAR*m_outHeightAR*3/2);
#endif

		}

		m_pSrcPlane[0] = m_pConv;
		if ((m_dCaptureFlags & STEREO_UD_INPUT) && (!(m_dCaptureFlags & STEREO_UD_OUTPUT))) {
			m_pSrcPlane[1] = m_pConv + m_bihIn.biWidth * m_bihIn.biHeight / 2;
			m_pSrcPlane[2] = m_pSrcPlane[1] + m_bihIn.biWidth * m_bihIn.biHeight / 8;
		} else {
			m_pSrcPlane[1] = m_pConv + m_bihIn.biWidth * m_bihIn.biHeight;
			m_pSrcPlane[2] = m_pConv + 5 * m_bihIn.biWidth * m_bihIn.biHeight / 4;
		}
		m_pDstPlaneD[0] = m_pDstPlaneD[1] = m_pDstPlaneD[2] = 0;
		m_pDstPlaneU[0] = m_pDeint;
		m_pDstPlaneU[1] = m_pDeint + m_bihIn.biWidth * m_inHeight;
		m_pDstPlaneU[2] = m_pDeint + 5 * m_bihIn.biWidth * m_inHeight / 4;
		if (m_dCaptureFlags & STEREO_UD_INPUT) {
			m_pDstPlaneD[0] = m_pDstPlaneU[0] + 3 * m_bihIn.biWidth * m_inHeight / 2;
			m_pDstPlaneD[1] = m_pDstPlaneD[0] + m_bihIn.biWidth * m_inHeight;
			m_pDstPlaneD[2] = m_pDstPlaneD[0] + 5 * m_bihIn.biWidth * m_inHeight / 4;
		}
		m_pOutPlaneD[0] = m_pOutPlaneD[1] = m_pOutPlaneD[2] = 0;
		m_pOutPlaneU[0] = m_pOut;
		m_pOutPlaneU[1] = m_pOut + m_bihOut.biWidth * m_outHeight;
		m_pOutPlaneU[2] = m_pOut + 5 * m_bihOut.biWidth * m_outHeight / 4;
		if (m_dCaptureFlags & STEREO_UD_OUTPUT) {
			m_pOutPlaneD[0] = m_pOutPlaneU[0] + 3 * m_bihOut.biWidth * m_outHeight / 2;
			m_pOutPlaneD[1] = m_pOutPlaneD[0] + m_bihOut.biWidth * m_outHeight;
			m_pOutPlaneD[2] = m_pOutPlaneD[0] + 5 * m_bihOut.biWidth * m_outHeight / 4;
		}

#ifndef IPP_RESIZE
		m_pClipRsmplUD[1] = 0;
		m_pClipRsmplUD[0] = m_pClipRsmpl;
		if (m_dCaptureFlags & STEREO_UD_OUTPUT) {
			m_pClipRsmplUD[1] = m_pClipRsmpl + 3 * m_bihOut.biWidth * m_outHeight / 2;
		}
#endif

		m_Inited |= SET_CHECK;
	}
	return true;
}

/**
 ****************************************************************************
 * Call to convert to Output format, use then GetOutput()
 * \date    13-02-2004
 ******************************************************************************/
bool CCaptureCConv::ConvertInput(unsigned char *src, int size){
	if (m_Inited != SET_INIT) return false;
	bool ret = true;

	if (m_dCaptureFlags & NOT_SRC_INPUT) memcpy(m_pCaptureFrame, src, m_bihIn.biSizeImage);

	if (m_dCaptureFlags & CSC_INPUT) {
		ret = false;
		switch(m_bihIn.biCompression)
		{
		case CColorSpace::FCC_YV12:
			ret = m_proc->ConvertYV12ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case CColorSpace::FCC_NV12:
			ret = m_proc->ConvertNV12ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case CColorSpace::FCC_YUY2:
			ret = m_proc->ConvertYUY2ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth*2, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case CColorSpace::FCC_UYVY:
		case CColorSpace::FCC_HDYC:
			ret = m_proc->ConvertUYVYToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth*2, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case CColorSpace::FCC_STR0:
			ret = m_proc->ConvertBMF24ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth * 3, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case BI_RGB:
			if (m_bihIn.biBitCount==24)
				ret = m_proc->ConvertBMF24ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth*3, m_bihIn.biHeight, m_bihIn.biWidth);
			else
				ret = m_proc->ConvertBMF32ToI420(m_pCaptureFrame, m_pSrcPlane[0], m_pSrcPlane[1], m_pSrcPlane[2], m_bihIn.biWidth*4, m_bihIn.biHeight, m_bihIn.biWidth);
			break;
		case CColorSpace::FCC_H264:
			if (m_dCaptureFlags & COMPRESS_INPUT) {
				VS_VideoCodecParam param;
				param.dec.FrameSize = size;
				m_pVideoDecoder->Convert(m_pCaptureFrame, m_pSrcPlane[0], &param);
				*(DWORD*)m_pOutCompress = size;
				memcpy(m_pOutCompress + sizeof(DWORD), m_pCaptureFrame, size);
				ret = true;
			}
		case CColorSpace::FCC_MJPG:
			{
				int res = libyuv::MJPGToI420(m_pCaptureFrame, size,
											m_pSrcPlane[0], m_bihIn.biWidth,
											m_pSrcPlane[1], m_bihIn.biWidth / 2,
											m_pSrcPlane[2], m_bihIn.biWidth / 2,
											m_bihIn.biWidth, m_bihIn.biHeight, m_bihIn.biWidth, m_bihIn.biHeight);
				ret = (res == 0);
			}
			break;
		default:
			break;
		}
	}

	if (!ret) {
		return false;
	}

	if (m_dCaptureFlags & DEINTERLACE) m_DeInt->Process(m_pConv, m_pDeint);

	if (m_dCaptureFlags & IDLE_OUTPUT) return ret;
	if (ret) {
		if (m_dCaptureFlags & RESAMPLE_OUTPUT) {
			ret = m_proc->ResampleI420(m_pDstPlaneU[0], m_pOutPlaneU[0], m_bihIn.biWidth, m_inHeight, m_bihOut.biWidth, m_outHeight);
			if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
				ret = m_proc->ResampleI420(m_pDstPlaneD[0], m_pOutPlaneD[0], m_bihIn.biWidth, m_inHeight, m_bihOut.biWidth, m_outHeight);
			}
		}
		else if (m_dCaptureFlags & CROP_OUTPUT) {
			ret = m_proc->ClipI420(m_pDstPlaneU[0], m_pOutPlaneU[0], m_bihIn.biWidth, m_inHeight, m_bihOut.biWidth, m_outHeight, m_CrpType);
			if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
				ret = m_proc->ClipI420(m_pDstPlaneD[0], m_pOutPlaneD[0], m_bihIn.biWidth, m_inHeight, m_bihOut.biWidth, m_outHeight, m_CrpType);
			}
		} else if (m_dCaptureFlags & (AR_OUTPUT | AR_IN_OUTPUT)) {

#ifdef IPP_RESIZE
			if (m_dCaptureFlags & AR_IN_OUTPUT) {
				ret = m_proc->ResampleInscribedI420(m_pDstPlaneU, m_pOutPlaneU, m_bihIn.biWidth, m_inHeight, m_bihIn.biWidth,
													m_bihOut.biWidth, m_outHeight, m_bihOut.biWidth,
													m_iOffsetW, m_iOffsetH, m_fFactorW, m_fFactorH, m_rMode);
				if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
					ret = m_proc->ResampleInscribedI420(m_pDstPlaneD, m_pOutPlaneD, m_bihIn.biWidth, m_inHeight, m_bihIn.biWidth,
														m_bihOut.biWidth, m_outHeight, m_bihOut.biWidth,
														m_iOffsetW, m_iOffsetH, m_fFactorW, m_fFactorH, m_rMode);
				}
			} else {
				ret = m_proc->ResampleCropI420(m_pDstPlaneU, m_pOutPlaneU, m_bihIn.biWidth, m_inHeight, m_bihIn.biWidth,
											   m_bihOut.biWidth, m_outHeight, m_bihOut.biWidth,
											   m_outWidthAR, m_outHeightAR, m_iOffsetW, m_iOffsetH, m_fFactorW, m_fFactorH, m_rMode);
				if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
					ret = m_proc->ResampleCropI420(m_pDstPlaneD, m_pOutPlaneD, m_bihIn.biWidth, m_inHeight, m_bihIn.biWidth,
												   m_bihOut.biWidth, m_outHeight, m_bihOut.biWidth,
												   m_outWidthAR, m_outHeightAR, m_iOffsetW, m_iOffsetH, m_fFactorW, m_fFactorH, m_rMode);
				}
			}
#else
			ret = m_proc->ResampleI420(m_pDstPlaneU[0], m_pClipRsmplUD[0], m_bihIn.biWidth, m_inHeight, m_outWidthAR, m_outHeightAR);
			if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
				ret = m_proc->ResampleI420(m_pDstPlaneD[0], m_pClipRsmplUD[1], m_bihIn.biWidth, m_inHeight, m_outWidthAR, m_outHeightAR);
			}
			if (ret) {
				ret = m_proc->ClipI420(m_pClipRsmplUD[0], m_pOutPlaneU[0], m_outWidthAR, m_outHeightAR, m_bihOut.biWidth, m_outHeight, 5);
				if (ret && (m_dCaptureFlags & (STEREO_UD_INPUT | STEREO_UD_OUTPUT))) {
					ret = m_proc->ClipI420(m_pClipRsmplUD[1], m_pOutPlaneD[0], m_outWidthAR, m_outHeightAR, m_bihOut.biWidth, m_outHeight, 5);
				}
			}
#endif

		}
	}

	return ret;
}

/**
 ****************************************************************************
 * Retrive pointer to I420 image
 * \return full size of image
 * \date    13-02-2004
 ******************************************************************************/
unsigned int CCaptureCConv::GetOutput(unsigned char* &frame, unsigned char* &compress) {
	if (m_Inited == SET_INIT) {
		frame = m_pOut;
		compress = m_pOutCompress;
		return m_bihOut.biWidth*m_bihOut.biHeight*3/2;
	}
	else {
		frame = 0;
		compress = 0;
		return 0;
	}
}

#ifdef VS_LINUX_DEVICE
CCaptureVideoSourceV4L::CCaptureVideoSourceV4L(CVSInterface *pParentInterface,char *szName,CVideoCaptureList *pCaptureList)
 :CVSInterface(szName, pParentInterface)
{
}

int CCaptureVideoSourceV4L::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	VS_AutoLock lock(this);
	_variant_t* var=(_variant_t*)pVar;
	*var = 0;
	return VS_INTERFACE_OK;
}
#endif //VS_LINUX_DEVICE

/////////////// Base Capture Device

const char VS_CaptureDevice::_funcConnect[] = "Connect";
const char VS_CaptureDevice::_funcDisconnect[] = "Disconnect";
const char VS_CaptureDevice::_funcControl[] = "CameraControl";
const char VS_CaptureDevice::_funcRealFramerate[] = "CaptureFPS";
const char VS_CaptureDevice::_funcCaptureFramerate[] = "CameraFPS";
const char VS_CaptureDevice::_funcPropertyPage[] = "CameraPropertyPage";
const char VS_CaptureDevice::_funcPins[] = "CameraPins";
const char VS_CaptureDevice::_funcVideoMode[] = "CameraMode";
const char VS_CaptureDevice::_funcCurrentName[] = "CurrentCamera";

VS_HardwareEncoderObserver* VS_CaptureDevice::m_pHardwareObserver = 0;

bool IsWindows8OrGreater();

VS_CaptureDevice* VS_CaptureDevice::Create(VS_CaptureDevice::eCaptureType type, VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList)
{
	switch (type)
	{
	case VS_CaptureDevice::SCREEN_CAPTURE:
		{
			return new VS_CaptureDeviceScreen(observerSlot);
		}
	case VS_CaptureDevice::MEDIA_FOUNDATION:
		{
			if (IsWindows8OrGreater() && IsMediaFoundationSupported())
				return new VS_CaptureDeviceMediaFoundation(observerSlot, pCaptureList);
			else
				return new VS_CaptureDeviceDirectShow(observerSlot, pCaptureList);
		}
	case VS_CaptureDevice::DIRECT_SHOW:
		{
			return new VS_CaptureDeviceDirectShow(observerSlot, pCaptureList);
		}
	}
	return 0;
}

VS_HardwareEncoderObserver* VS_CaptureDevice::GetHardwareObserver()
{
	return m_pHardwareObserver;
}

void VS_CaptureDevice::Open()
{
	g_hMf = LoadLibrary("Mf.dll");
	g_hMfplat = LoadLibrary("Mfplat.dll");
	g_hMfreadwrite = LoadLibrary("mfreadwrite.dll");
	g_hMfuuid = LoadLibrary("mfuuid.dll");
	if (g_hMf && g_hMfplat && g_hMfreadwrite) {
		(FARPROC &)g_MFStartup = GetProcAddress(g_hMfplat, "MFStartup");
		(FARPROC &)g_MFShutdown = GetProcAddress(g_hMfplat, "MFShutdown");
		(FARPROC &)g_MFEnumDeviceSources = GetProcAddress(g_hMf, "MFEnumDeviceSources");
		(FARPROC &)g_MFCreateAttributes = GetProcAddress(g_hMfplat, "MFCreateAttributes");
		(FARPROC &)g_MFCreateDeviceSource = GetProcAddress(g_hMf, "MFCreateDeviceSource");
		(FARPROC &)g_MFPutWorkItem = GetProcAddress(g_hMfplat, "MFPutWorkItem");
		(FARPROC &)g_MFCreateAsyncResult = GetProcAddress(g_hMfplat, "MFCreateAsyncResult");
		(FARPROC &)g_MFCreateSourceReaderFromMediaSource = GetProcAddress(g_hMfreadwrite, "MFCreateSourceReaderFromMediaSource");
		(FARPROC &)g_MFCreateMemoryBuffer = GetProcAddress(g_hMfplat, "MFCreateMemoryBuffer");
		(FARPROC &)g_MFCreateMediaType = GetProcAddress(g_hMfplat, "MFCreateMediaType");
		(FARPROC &)g_MFInvokeCallback = GetProcAddress(g_hMfplat, "MFInvokeCallback");
		(FARPROC &)g_MFAllocateSerialWorkQueue = GetProcAddress(g_hMfplat, "MFAllocateSerialWorkQueue");
		(FARPROC &)g_MFUnlockWorkQueue = GetProcAddress(g_hMfplat, "MFUnlockWorkQueue");
	}
}

void VS_CaptureDevice::Close()
{
	if (g_hMf) FreeLibrary(g_hMf); g_hMf = NULL;
	if (g_hMfplat) FreeLibrary(g_hMfplat); g_hMfplat = NULL;
	if (g_hMfreadwrite) FreeLibrary(g_hMfreadwrite); g_hMfreadwrite = NULL;
	if (g_hMfuuid) FreeLibrary(g_hMfuuid); g_hMfuuid = NULL;
}

bool VS_CaptureDevice::IsMediaFoundationSupported()
{
	bool ret = (g_MFStartup && g_MFShutdown &&
				g_MFEnumDeviceSources && g_MFCreateAttributes && g_MFCreateDeviceSource &&
				g_MFPutWorkItem && g_MFCreateAsyncResult && g_MFCreateSourceReaderFromMediaSource &&
				g_MFCreateMemoryBuffer && g_MFCreateMediaType && g_MFInvokeCallback &&
				g_MFAllocateSerialWorkQueue && g_MFUnlockWorkQueue);
	return ret;
}

VS_CaptureDevice::VS_CaptureDevice(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList)
{
	m_CurrentDeviceName[0] = 0;
	m_pCaptureList = pCaptureList;
	m_renderFmt.SetZero();
	m_captureFmt.SetZero();
	m_startFramerate = 3;
	m_setFramerate = m_last_setFramerate = 300;
	m_realFramerate = 300;
	CleanFramerate();
	m_hUpdateState = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hGetPropertyPage = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_pVideoSlotObserver = reinterpret_cast <VS_VideoCaptureSlotObserver*> (observerSlot);
}

VS_CaptureDevice::~VS_CaptureDevice()
{
	if (m_hUpdateState) CloseHandle(m_hUpdateState);
	if (m_hGetPropertyPage) CloseHandle(m_hGetPropertyPage);
}

int VS_CaptureDevice::Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings)
{
	VS_AutoLock lock(this);
	m_devState.FillState(szName, DEVICE_CONNECT, mf, deviceMode);
	if (devSettings) m_hwndPropPage = devSettings->hwndProp;
	m_qDevState.push(m_devState);
	SetEvent(m_hUpdateState);
	return 0;
}

int VS_CaptureDevice::Disconnect()
{
	VS_AutoLock lock(this);
	m_devState.FillState(NULL, DEVICE_DISCONNECT, &m_renderFmt, -1);
	m_qDevState.push(m_devState);
	SetEvent(m_hUpdateState);
	return 0;
}

bool VS_CaptureDevice::RunPropertyPage(HWND hwndPage)
{
	m_hwndPropPage = hwndPage;
	return GetPropertyPage();
}

void VS_CaptureDevice::StartCaptureFramerate(int framerate)
{
	m_lockFramerate.Lock();
	m_startFramerate = framerate;
	m_setFramerate = m_startFramerate * 100;
	m_lockFramerate.UnLock();
}

void VS_CaptureDevice::SetCaptureFramerate(int framerate)
{
	m_lockFramerate.Lock();
	m_setFramerate = framerate;
	m_lockFramerate.UnLock();
}

int VS_CaptureDevice::GetCaptureFramerate()
{
	m_lockFramerate.Lock();
	int framerate = m_startFramerate;
	m_lockFramerate.UnLock();
	return 0;
}

int VS_CaptureDevice::GetRealFramerate()
{
	m_lockFramerate.Lock();
	int framerate = m_realFramerate;
	m_lockFramerate.UnLock();
	return framerate;
}

void VS_CaptureDevice::CleanFramerate()
{
	while (!m_qFrameTimestamp.empty()) m_qFrameTimestamp.pop();
	m_lastTimestamp = 0;
	m_accTimestamp = 0;
}

bool VS_CaptureDevice::SnapFramerate(unsigned int ctime, __int64 timestamp, int baseFramerate)
{
	bool bSkipFrame = false;
	__int64 frameIntervalBase = 1000000000 / baseFramerate;
	__int64 frameInterval = frameIntervalBase;
	if (m_lastTimestamp != 0) {
		frameInterval = timestamp - m_lastTimestamp;
	}
	m_accTimestamp -= frameInterval;
	if (m_accTimestamp < 0) m_accTimestamp = 0;
	if (m_accTimestamp > frameIntervalBase * 2) m_accTimestamp = frameIntervalBase * 2;
	if (m_accTimestamp > frameIntervalBase) {
		bSkipFrame = true;
	} else {
		m_accTimestamp += frameIntervalBase;
	}
	m_lastTimestamp = timestamp;
	m_qFrameTimestamp.push(ctime);
	return bSkipFrame;
}

bool VS_CaptureDevice::GetFramerate(unsigned int ctime, int &realFramerate)
{
	bool ret = false;
	if (!m_qFrameTimestamp.empty()) {
		unsigned int dt = ctime - m_qFrameTimestamp.front();
		if (dt >= 1000) {
			realFramerate = (int)((m_qFrameTimestamp.size() * 1000.0) / (double)dt) * 100;
			//DTRACE(VSTM_VCAPTURE, "Real Framerate capture: fps = %d", realFramerate);
			while (!m_qFrameTimestamp.empty() && (ctime - m_qFrameTimestamp.front() >= 1000)) m_qFrameTimestamp.pop();
			ret = true;
		}
	}
	return ret;
}

int VS_CaptureDevice::FindOptimalVideoMode(VS_MediaFormat renderFmt, int iCheckHDInput)
{
	int nVideoMode = -1, nHDMode = -1;
	int setWidth = 0, setHeight = 0;

	if ((renderFmt.dwVideoWidht & 0x7) && (renderFmt.dwVideoHeight & 0x7)) return nVideoMode;

	m_captureFmt = renderFmt;
	if (iCheckHDInput > 0) nVideoMode = m_pCaptureList->GetAutoModeHD(m_CurrentDeviceName);
	if (nVideoMode < 0) nVideoMode = m_nOptimalVideoMode;
	if (nVideoMode < 0) {
		// auto serch of favorable mode
		int ColorOrder[] = {
				CColorSpace::STR0,
				CColorSpace::H264,
				(m_typeDevice == VS_CaptureDevice::MEDIA_FOUNDATION) ? CColorSpace::MJPEG : CColorSpace::I420,
				CColorSpace::FCC_IYUV,
				CColorSpace::YV12,
				CColorSpace::UYVY,
				CColorSpace::HDYC,
				CColorSpace::YUY2,
				CColorSpace::RGB24,
				CColorSpace::RGB32,
				(m_typeDevice == VS_CaptureDevice::MEDIA_FOUNDATION) ? CColorSpace::I420 : CColorSpace::MJPEG,
				CColorSpace::DVSD,
				CColorSpace::DVHD,
				CColorSpace::DVSL,
				CColorSpace::BJPG
		};
		int ModeMAX = sizeof(ColorOrder)/sizeof(ColorOrder[0]);
		int startMode = 2;

		if (m_renderFmt.dwStereo > 0) {
			ModeMAX = 1;
			startMode = 0;
		} else if (m_renderFmt.dwHWCodec == ENCODER_H264_LOGITECH) {
			ModeMAX = 2;
			startMode = 1;
		}

		int prefh = renderFmt.dwVideoHeight, prefw = renderFmt.dwVideoWidht;

		int i = 0, it = 0;
		int NumOfMode = m_pModeList->iGetMaxMode();
		unsigned int MinDim = -1;
		for (i = 0; i < NumOfMode; i++) {
			bool bNextMode = false;
			CColorModeDescription cmd;
			m_pModeList->iGetModeDescription(i, &cmd);
			for (it = startMode; it < ModeMAX; it++) {
				if (cmd.Color == ColorOrder[it]) {
					int w, h, dw, dh;
					dw = (cmd.dW == 0 && cmd.WidthMin <= cmd.WidthMax) ? 4 : cmd.dW;
					dh = (cmd.dH == 0 && cmd.HeightMin <= cmd.HeightMax) ? 4 : cmd.dH;
					for (w = cmd.WidthMin; w <= cmd.WidthMax; w += dw) {
						if (w % 8 == 0) {
							for (h = cmd.HeightMin; h <= cmd.HeightMax; h += dh) {
								if (h % 8 == 0) {
									unsigned int CurrDim = abs(h - prefh) + abs(w - prefw) + it;
									if (MinDim >= CurrDim) {
										MinDim = CurrDim;
										nVideoMode = i;
										setWidth = w;
										setHeight = h;
									}
								}
							}
						}
						if (bNextMode) break;
					}
				}
				if (bNextMode) break;
			}
		}
	}

	CColorModeDescription cmd;
	if (m_pModeList->iGetModeDescription(nVideoMode, &cmd) == 0) {
		m_captureFmt.dwVideoCodecFCC = cmd.Color;
		m_captureFmt.dwVideoWidht = (setWidth == 0) ? cmd.WidthBase : setWidth;
		m_captureFmt.dwVideoHeight = (setHeight == 0) ? cmd.HeightBase : setHeight;
		if (!cmd.FrameIntList.empty()) {
			int i = 0;
			__int64 dmin_up = ((__int64)1 << 63);
			__int64 dmin_lo = ((__int64)1 << 62);
			int idx_up = -1, idx_lo = -1;
			unsigned int rndFrameInt = (unsigned int)(10000000.0 / (double)(m_renderFmt.dwFps) + 0.5);
			for (i = 0; i < (int)cmd.FrameIntList.size(); i++) {
				if (cmd.FrameIntList[i] == 0) continue;
				__int64 d = cmd.FrameIntList[i] - rndFrameInt;
				if (d <= 0 && d > dmin_up) {
					dmin_up = d;
					idx_up = i;
				} else if (d > 0 && d < dmin_lo) {
					dmin_lo = d;
					idx_lo = i;
				}
			}
			int idx = (idx_up == -1) ? idx_lo : idx_up;
			if (cmd.FrameIntList[idx] == 0) return -1;
			m_captureFmt.dwFps = (DWORD)(10000000.0 / (double)(cmd.FrameIntList[idx]) * 100);
		} else {
			if (cmd.FrameIntMin == 0 && cmd.FrameIntMax == 0) return -1;
			if (cmd.FrameIntMin == 0) cmd.FrameIntMin = cmd.FrameIntMax;
			if (cmd.FrameIntMax == 0) cmd.FrameIntMax = cmd.FrameIntMin;
			uint32_t maxFps = 10000000.0 / cmd.FrameIntMin * 100;
			uint32_t minFps = 10000000.0 / cmd.FrameIntMax * 100;
			if (m_pCaptureList->IsAverMediaDevice(m_CurrentDeviceName)) {
				m_captureFmt.dwFps = std::min(std::max((uint32_t)3000, m_renderFmt.dwFps * 100), maxFps);
			} else {
				m_captureFmt.dwFps = std::min(std::max(minFps, m_renderFmt.dwFps * 100), maxFps);
			}
		}
	}

	return nVideoMode;
}
