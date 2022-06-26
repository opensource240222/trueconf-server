/**
 **************************************************************************
 * \file VSGrabber.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Объявления классов фильтра VSGrabber
 *
 * we override the input pin class so we can provide a media type
* to speed up connection times. When you try to connect a filesourceasync
* to a transform filter, DirectShow will insert a splitter and then
* start trying codecs, both audio and video, video codecs first. If
* your sample grabber's set to connect to audio, unless we do this, it
* will try all the video codecs first. Connection times are sped up x10
* for audio with just this minor modification!
 *
 * \b Project Client
 * \author Grabber Sample
 * \author Melechko Ivan
 * \date 07.10.02
 *
 * $Revision: 1 $
 *
 * $History: VSGrabber.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 11  *****************
 * User: Melechko     Date: 16.02.05   Time: 14:21
 * Updated in $/VS/VSClient
 * Add MJPEG support
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef _VSGRABBER_
#define _VSGRABBER_

/// CLSID для фильтра VSGrabber
DEFINE_GUID(CLSID_VSGrabber,0x2ca0a2d5, 0x5918, 0x4bd8, 0x80, 0x4a, 0x97, 0x45, 0xc6, 0x3c, 0x1e, 0x0a);

class CSharedMemoryMaster;
class VS_VideoCaptureSlotObserver;

/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Grabber In Pin Class
 ****************************************************************************/
class CVSGrabberInPin : public CRendererInputPin
{
    friend class CVSGrabber;
protected:
    CVSGrabber * VSGrabber( ) { return (CVSGrabber*) m_pFilter; }
public:

    CVSGrabberInPin(CBaseRenderer * pFilter, HRESULT * pHr)
        : CRendererInputPin(pFilter, pHr, L"Input"){};

    HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );

};

/**
 **************************************************************************
* \brief Класс фильтра VSGrabber. Позволяет перехватывать видефреймы DirectShow и получать их
* через разделяемую память.
 ****************************************************************************/
class CVSGrabber : public CBaseRenderer
{
    friend class CVSGrabberInPin;
    friend class CVSGrabberAllocator;
    friend class CVSGrabberI;
private :
	bool m_bHardwareEncoder;
	VS_VideoCaptureSlotObserver *m_observer;
public:

    DECLARE_IUNKNOWN;
    HRESULT DoRenderSample( IMediaSample *pms);
    HRESULT CheckMediaType(    const CMediaType *pmt) {return SetAcceptedMediaType(pmt);};
    CVSGrabber( IUnknown * pOuter, HRESULT * pHr, VS_VideoCaptureSlotObserver *observer);

    /// IGrabberSample
    HRESULT SetAcceptedMediaType( const CMediaType * pmt );
    HRESULT GetConnectedMediaType( CMediaType * pmt );
    HRESULT GetFPS(int *pFPS);
    HRESULT SetFPS(int iFPS);
	HRESULT CheckMediaFormat(unsigned int &w, unsigned int &h, unsigned int &fourcc);
	void SetCaps(int width, int height, int color, bool bHardware) { m_iWidth = width; m_iHeight = height; m_iColor = color; m_bHardwareEncoder = bHardware; }
	void PauseGpaph();

protected:
    CMediaType m_mtAccept;
	int m_iSampleSize;
	int m_iWidth;
	int m_iHeight;
	int m_iColor;

    CCritSec m_Lock; ///< serialize access to our data

    int m_iFPS;
    int m_TimeCount,m_FPSCounter;
    int m_aFPS[10];
    REFERENCE_TIME m_PrevTime;
    REFERENCE_TIME m_Time;
    REFERENCE_TIME m_aTime[10];
};

#endif