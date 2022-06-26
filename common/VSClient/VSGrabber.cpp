/**
 **************************************************************************
 * \file VSGrabber.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Классы фильтра VSGrabber
 *
 * \b Project Client
 * \author Grabber Sample
 * \author Melechko Ivan
 * \date 07.10.02
 *
 * $Revision: 8 $
 *
 * $History: VSGrabber.cpp $
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 23.08.12   Time: 18:57
 * Updated in $/VSNA/VSClient
 * - grabber fix
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 2.03.12    Time: 14:52
 * Updated in $/VSNA/VSClient
 * - fix mutex wait for grabber
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 23.06.11   Time: 14:23
 * Updated in $/VSNA/VSClient
 * - fix enumerate bug in directshow
 * - fix bug: av null name in capture list
 * - self view fps to 60
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 17.12.10   Time: 20:19
 * Updated in $/VSNA/VSClient
 * Add support virtual cameras
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 2.12.10    Time: 16:14
 * Updated in $/VSNA/VSClient
 * - increase accuracy video capture timestamp
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 18.02.09   Time: 22:19
 * Updated in $/VSNA/VSClient
 * - skip first frames
 * - bugfix with possible Null pointer
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 21.04.08   Time: 18:56
 * Updated in $/VSNA/VSClient
 * - possible bugfix: bad memory size in some video drivers corrected
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 6.02.07    Time: 20:29
 * Updated in $/VS2005/VSClient
 * - Added DirectShow project
 * - soluton configuration corrected
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 19.07.06   Time: 17:41
 * Updated in $/VS/VSClient
 * - fix first reported RealFps from grabber
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 3.05.06    Time: 18:09
 * Updated in $/VS/VSClient
 * - removed 1 frame latency in video capture device
 * - removed null (first) frame rendering after capture deinit
 * - InvertUv registry param moved
 *
 * *****************  Version 27  *****************
 * User: Melechko     Date: 16.02.05   Time: 14:21
 * Updated in $/VS/VSClient
 * Add MJPEG support
 *
 * *****************  Version 26  *****************
 * User: Melechko     Date: 2/08/05    Time: 4:51p
 * Updated in $/VS/VSClient
 *
 * *****************  Version 25  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "extlibs/DirectShowBaseClasses/streams.h"
#include <dvdmedia.h>
#include <initguid.h>
#include <string.h>

#include "VSDirecShow.h"
#include "VSGrabber.h"
#include "VSShareMem.h"
#include "VSCapture.h"

/****************************************************************************
 * Classes
 ****************************************************************************/
HRESULT CVSGrabber::GetFPS(int *pFPS){
	*pFPS = m_iFPS;
	return NOERROR;
};

HRESULT CVSGrabber::SetFPS(int iFPS){
	if		(iFPS<25)
		m_iFPS = 25;
	else if (iFPS>6000)
		m_iFPS = 6000;
	else
		m_iFPS = iFPS;
	return NOERROR;
};

CVSGrabber::CVSGrabber( IUnknown * pOuter, HRESULT * phr, VS_VideoCaptureSlotObserver *observer )
: CBaseRenderer(CLSID_VSGrabber,TEXT("VSGrabber"), (IUnknown*) pOuter,phr)
{
	m_observer = observer;
	m_pInputPin = (CRendererInputPin*) new CVSGrabberInPin( this, phr );
	if( !m_pInputPin )
	{
		*phr = E_OUTOFMEMORY;
	}
	m_iFPS=1000;
	m_Time=0;
	int i;
	m_TimeCount=0;
	m_FPSCounter=0;
	m_PrevTime=0;
	for(i=0;i<10;i++){
		m_aFPS[i]=0;
		m_aTime[i]=0;
	}
	m_bHardwareEncoder = false;
}

#define DROP_WATERMARK "VisicronII"
void CVSGrabber::PauseGpaph()
{
	m_PrevTime^=m_PrevTime;
};
HRESULT CVSGrabber::DoRenderSample( IMediaSample *pms)
{
	CAutoLock lock( &m_Lock );

	if (m_observer) {
		BYTE *p = 0;
		REFERENCE_TIME StartTime, StopTime;
		int i;
		int size = std::min(pms->GetSize(), pms->GetActualDataLength());
		size = std::min(size, m_iSampleSize);
		pms->GetPointer(&p);
		pms->GetTime(&StartTime, &StopTime);
		if (p && size > 20) { // we have data and it length
			if (strncmp((char*)(p+size/2-10), DROP_WATERMARK, 10)!=0 || m_bHardwareEncoder) {
				DWORD dtWait = 10;
				DWORD RealFPS = 0;
				if (StartTime-m_PrevTime>0) {
					RealFPS = (DWORD)(10000000*100/(StartTime-m_PrevTime));
					m_aFPS[m_FPSCounter]=RealFPS;
					RealFPS = 0;
					int cnt = 0;
					for (i=0; i<10; i++) {
						RealFPS+=m_aFPS[i];
						cnt+=m_aFPS[i]!=0;
					}
					if (cnt)
						RealFPS/=cnt;
					else
						RealFPS = 3000;
					m_FPSCounter++;
					if (m_FPSCounter>9)
						m_FPSCounter=0;
				}
				else {
					if (StartTime-m_PrevTime<0) {
						m_PrevTime=StartTime;
					}
					return NOERROR;
				}
				m_PrevTime=StartTime;
				StopTime=m_Time;
				m_Time=StartTime;

				if (m_aTime[1]==0)
					m_aTime[1] = StartTime;
				if (StartTime - m_aTime[1] < 2000000 && !m_bHardwareEncoder) //skip first frame + 0.2 sec
					return NOERROR;

				m_aTime[0]-=StartTime-StopTime;

				if(m_aTime[0]<0)
					m_aTime[0]=0;
				if(m_aTime[0] > (1000000000/m_iFPS)*2)
					m_aTime[0] = (1000000000/m_iFPS)*2;
				if(m_aTime[0] > (1000000000/m_iFPS) && !m_bHardwareEncoder) {
					return NOERROR;
				}
				m_aTime[0]+=(1000000000/m_iFPS);

				m_Time=StartTime;

				m_observer->PushFrame(p, size, RealFPS, (unsigned int)((double)StartTime/10000.0), m_iWidth, m_iHeight, m_iColor, m_bHardwareEncoder);

				strcpy((char*)(p+size/2-10), DROP_WATERMARK);
			}
		}
	}
	return NOERROR;
}

HRESULT CVSGrabber::CheckMediaFormat(unsigned int &w, unsigned int &h, unsigned int &fourcc)
{
	CAutoLock lock( &m_Lock );

	CMediaType MediaType;

	if(!m_pInputPin->IsConnected())
		return E_FAIL;

	m_pInputPin->ConnectionMediaType(&MediaType);
	BITMAPINFOHEADER *pBmh = NULL;
	if (MediaType.formattype == FORMAT_VideoInfo) {
		pBmh = &(((VIDEOINFOHEADER*)MediaType.pbFormat)->bmiHeader);
	} else {
		pBmh = &(((VIDEOINFOHEADER2*)MediaType.pbFormat)->bmiHeader);
	}
	int rgbSize = DIBSIZE(*pBmh);
	m_iSampleSize = std::max(MediaType.GetSampleSize(), pBmh->biSizeImage);
	m_iSampleSize = std::max(m_iSampleSize, rgbSize);
	if (m_iSampleSize < 2) {
		return E_FAIL;
	}
	w = pBmh->biWidth;
	h = pBmh->biHeight;
	fourcc = pBmh->biCompression;

	return S_OK;
}

HRESULT CVSGrabber::SetAcceptedMediaType( const CMediaType * pmt )
{
	CAutoLock lock( &m_Lock );

	if( !pmt )
	{
		m_mtAccept = CMediaType( );
		return NOERROR;
	}

	HRESULT hr;

	hr = CopyMediaType( &m_mtAccept, pmt );

	return hr;
}

/**
 **************************************************************************
 * \brief used to help speed input pin connection times. We return a partially
 * specified media type - only the main type is specified. If we return
 * anything BUT a major type, some codecs written improperly will crash
 ****************************************************************************/
HRESULT CVSGrabberInPin::GetMediaType( int iPosition, CMediaType * pMediaType )
{
	if (iPosition < 0) {
		return E_INVALIDARG;
	}
	if (iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	*pMediaType = CMediaType( );
	pMediaType->SetType( ((CVSGrabber*)m_pFilter)->m_mtAccept.Type( ) );
	return S_OK;
}

