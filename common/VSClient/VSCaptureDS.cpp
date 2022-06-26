
#include <atlbase.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <wmcodecdsp.h>
#include "VSShareMem.h"
#include "VSDirecShow.h"
#include "VSCapture.h"
#include <KsMedia.h>
#include "VSVideoCaptureList.h"
#include "VS_Dmodule.h"
#include "../std/cpplib/VS_Protocol.h"

/******************************************************************************
 * DirectShow capture device
 ******************************************************************************/
VS_CaptureDeviceDirectShow::VS_CaptureDeviceDirectShow(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList) : VS_CaptureDevice(observerSlot, pCaptureList)
{
	m_pBuilder = NULL;
	m_pGraph = NULL;
	m_pCaptureFilter = NULL;
	m_pGrabberFilter = NULL;
	m_pMixerVideoPortFilter = NULL;
	m_pVideoDecoderFilter = NULL;
	m_pGrabber = NULL;
	m_pAnalogVideoDecoder = NULL;
	m_pCrossbar = NULL;
	m_pTVTuner = NULL;
	m_pPropertyPages = NULL;
	m_pMediaControl = NULL;
	m_pModeList = NULL;

	memset(m_CrossbarPinsReal, 0, sizeof(m_CrossbarPinsReal));
	memset(m_CrossbarPinsGUI, 0, sizeof(m_CrossbarPinsGUI));
	m_stHardwareEncoder.pKsControl = NULL;
	m_stHardwareEncoder.iNode = 0;

	m_eDeviceState = DEVICE_SHUTDOWN;
	m_nOptimalVideoMode = -1;

	/// Events
	m_hMutexGetParams = CreateMutex(NULL, TRUE, 0);
	ReleaseMutex(m_hMutexGetParams);
	m_hWasInitGraph = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hSetBitrateHW = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hNeedIFrameHW = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hResetHW = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_pHardwareObserver = static_cast <VS_HardwareEncoderObserver*> (this);

	m_typeDevice = VS_CaptureDevice::DIRECT_SHOW;

	ActivateThread(this);
}

VS_CaptureDeviceDirectShow::~VS_CaptureDeviceDirectShow()
{
	DesactivateThread();

	m_lockCapture.Lock();
	m_pModeList = NULL;
	/// close events
	if (m_hMutexGetParams) CloseHandle(m_hMutexGetParams);
	if (m_hWasInitGraph) CloseHandle(m_hWasInitGraph);
	if (m_hSetBitrateHW) CloseHandle(m_hSetBitrateHW);
	if (m_hNeedIFrameHW) CloseHandle(m_hNeedIFrameHW);
	if (m_hResetHW) CloseHandle(m_hResetHW);
	m_lockCapture.UnLock();
}

int VS_CaptureDeviceDirectShow::Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings)
{
	VS_AutoLock lock(this);
	memcpy(&m_devSettings, devSettings, sizeof(VS_CaptureDeviceSettings));
	VS_CaptureDevice::Connect(szName, mf, deviceMode, devSettings);
	return 0;
}

int VS_CaptureDeviceDirectShow::Disconnect()
{
	VS_AutoLock lock(this);
	VS_CaptureDevice::Disconnect();
	ResetEvent(m_hWasInitGraph);
	return 0;
}

void VS_CaptureDeviceDirectShow::Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state)
{
	VS_AutoLock lock(this);
	if (!state) {
		Connect(szName, mf, deviceMode, &m_devSettings);
	} else {
		Disconnect();
	}
}

bool VS_CaptureDeviceDirectShow::IsPropertyPage()
{
	bool ret = false;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT) {
				ret = (m_pPropertyPages != NULL);
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

bool VS_CaptureDeviceDirectShow::GetPropertyPage()
{
	bool ret = false;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT) {
				SetEvent(m_hGetPropertyPage);
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

long VS_CaptureDeviceDirectShow::GetPins(_variant_t *var)
{
	long ret = -1;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT) {
				long i;
				long inpin = m_qCrossbarPins.size(), outpin = 0;
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY *psa;
				if (!m_qCrossbarPins.empty()) {
					rgsabound[0].lLbound = 0;
					rgsabound[0].cElements = inpin;
					psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
					if (psa != NULL) {
						var->parray = psa;
						var->vt = VT_ARRAY | VT_VARIANT;
						for (i = 0; i < inpin; i++) {
							_variant_t var_ = m_qCrossbarPins[i].c_str();
							SafeArrayPutElement(psa, &i, &(var_.GetVARIANT()));
						}
						long iPin = 0;
						m_pCrossbar->get_IsRoutedTo(FindOutPin(), &iPin);
						ret = VS_INTERFACE_OK + (m_CrossbarPinsGUI[iPin] << 16);
					}
				}
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

bool VS_CaptureDeviceDirectShow::SetPin(long nPin)
{
	bool ret = false;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT) {
				if (!m_qCrossbarPins.empty()) {
					m_pCrossbar->Route(FindOutPin(), m_CrossbarPinsReal[nPin]);
					m_pCrossbar->get_IsRoutedTo(FindOutPin(), &nPin);
					ret = true;
				}
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

__int64 VS_CaptureDeviceDirectShow::GetVideoModes()
{
	__int64 ret = -1;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT && m_pAnalogVideoDecoder) {
				long formats, format;
				m_pAnalogVideoDecoder->get_AvailableTVFormats(&formats);
				m_pAnalogVideoDecoder->get_TVFormat(&format);
				ret = (__int64)( (__int64)(formats) + ((__int64)(format) << 32) );
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

bool VS_CaptureDeviceDirectShow::SetVideoMode(long mode)
{
	bool ret = false;
	if (WaitForSingleObject(m_hWasInitGraph, 2000) == WAIT_OBJECT_0) {
		if (WaitForSingleObject(m_hMutexGetParams, 3000) == WAIT_OBJECT_0) {
			if (m_eDeviceState >= DEVICE_INIT && m_pAnalogVideoDecoder) {
				long format;
				m_pAnalogVideoDecoder->put_TVFormat(mode);
				m_pAnalogVideoDecoder->get_TVFormat(&format);
				ret = true;
			}
			ReleaseMutex(m_hMutexGetParams);
		}
	}
	return ret;
}

int VS_CaptureDeviceDirectShow::FindOutPin()
{
	long inpin,outpin;
	m_pCrossbar->get_PinCounts(&outpin,&inpin);
	if(outpin==1)
		return 0;
	int i;
	for(i=0;i<outpin;i++){
		long rel,t;
		m_pCrossbar->get_CrossbarPinInfo(FALSE,i,&rel,&t);
		if(t==PhysConn_Video_VideoDecoder)
			return i;

	}
	return 0;
}

void VS_CaptureDeviceDirectShow::CheckCrossbarPins()
{
	long inpin = 0, outpin = 0;
	m_pCrossbar->get_PinCounts(&outpin, &inpin);
	int idx_outpin = FindOutPin();
	if (inpin >= 2) {
		int i = 0, ri = 0;
		for (i = 0; i < inpin; i++) {
			HRESULT hr = m_pCrossbar->CanRoute(idx_outpin, i);
			if (hr != S_OK) continue;
			long rel = 0, t = 0;
			m_pCrossbar->get_CrossbarPinInfo(TRUE, i, &rel, &t);
			switch (t)
			{
			case 1:  m_qCrossbarPins.emplace_back("Tuner");			break;
			case 2:  m_qCrossbarPins.emplace_back("Composite");		break;
			case 3:  m_qCrossbarPins.emplace_back("S-Video");			break;
			case 4:  m_qCrossbarPins.emplace_back("RGB");				break;
			case 5:  m_qCrossbarPins.emplace_back("YRYBY");			break;
			case 6:  m_qCrossbarPins.emplace_back("Serial Digital");	break;
			case 7:  m_qCrossbarPins.emplace_back("Parallel Digital");	break;
			case 8:  m_qCrossbarPins.emplace_back("SCSI video");		break;
			case 9:  m_qCrossbarPins.emplace_back("AUX video");		break;
			case 10: m_qCrossbarPins.emplace_back("1394 video");		break;
			case 11: m_qCrossbarPins.emplace_back("USB video");		break;
			case 12: m_qCrossbarPins.emplace_back("Video decoder");	break;
			case 13: m_qCrossbarPins.emplace_back("Video encoder");	break;
			case 14: m_qCrossbarPins.emplace_back("SCART video");		break;
			default: if (t < 4096) m_qCrossbarPins.emplace_back("Unknown");
			}
			if (t < 4096) {
				m_CrossbarPinsReal[ri] = i;
				m_CrossbarPinsGUI[i] = ri;
			}
			ri++;
		}
	}
}

int VS_CaptureDeviceDirectShow::iGetDeviceModeList(const wchar_t *szName)
{
	if ((m_pModeList = m_pCaptureList->GetModeListByName(szName)) == NULL) return -1;
	return 0;
}

int VS_CaptureDeviceDirectShow::InitGraph()
{
	HRESULT hr = (HRESULT)(-1);

	ShutdownGraph();

	/// create graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&m_pBuilder);
	if ( SUCCEEDED(hr) ) {
		/// create graph
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,IID_IGraphBuilder, (void **)&m_pGraph);
		if ( SUCCEEDED(hr) ) {
			/// set graph to builder
			hr = m_pBuilder->SetFiltergraph(m_pGraph);
			if ( SUCCEEDED(hr) ) {
				/// get Capture Filter by name
				hr = S_FALSE;
				VS_VideoCaptureDirectShowList *pList = reinterpret_cast <VS_VideoCaptureDirectShowList*> (m_pCaptureList);
				if (pList) {
					m_pCaptureFilter = pList->GetCaptureByName(m_CurrentDeviceName);
					if (m_pCaptureFilter != NULL) {
						/// set Capture Filter to Graph
						hr = m_pGraph->AddFilter(m_pCaptureFilter, L"Video Capture");
						if ( SUCCEEDED(hr) ) {
							/// create Grabber
							m_pGrabber = new CVSGrabber(NULL, &hr, m_pVideoSlotObserver);
							if ( SUCCEEDED(hr) ) {
								hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void**)&m_pGrabberFilter);
								if ( SUCCEEDED(hr) ) {
									/// set Grabber Filter to Graph
									hr = m_pGraph->AddFilter(m_pGrabberFilter, L"Grabber filter");
									if ( SUCCEEDED(hr) ) {
										/// find pin
										IPin *pVideoPortPin = NULL;
										hr = m_pBuilder->FindPin(m_pCaptureFilter, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT, NULL, FALSE, 0, &pVideoPortPin);
										if ( SUCCEEDED(hr) ) {
											hr = CoCreateInstance(CLSID_OverlayMixer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&m_pMixerVideoPortFilter);
											if ( SUCCEEDED(hr) ) {
												hr = m_pGraph->AddFilter(m_pMixerVideoPortFilter, L"Mixer");
												if ( SUCCEEDED(hr) ) {
													IPin *pMixPin = GetPin(m_pMixerVideoPortFilter, 0, PINDIR_INPUT);
													hr = m_pGraph->Connect(pVideoPortPin, pMixPin);
													if ( SUCCEEDED(hr) ) {
														IBaseFilter *pRenderer;
														hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pRenderer);
														if ( SUCCEEDED(hr) ) {
															hr = m_pGraph->AddFilter(pRenderer, L"Renderer");
															if ( SUCCEEDED(hr) ) {
																IPin *pRenderPin = GetPin(pRenderer, 0, PINDIR_INPUT);
																IPin *pMixOutPin = GetPin(m_pMixerVideoPortFilter, 0, PINDIR_OUTPUT);
																if (pMixOutPin && pRenderPin) hr = m_pGraph->Connect(pMixOutPin, pRenderPin);
																m_pGraph->RemoveFilter(pRenderer);
															}
															pRenderer->Release();
														}
													}
												}
											}
										}
										/// check for hw h.264 encoder extention
										IKsTopologyInfo *pKsTopologyInfo = 0;
										hr = m_pCaptureFilter->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&pKsTopologyInfo);
										if ( SUCCEEDED(hr) ) {
											m_stHardwareEncoder.pKsControl = 0;
											m_stHardwareEncoder.iNode = 0;
											hr = FindExtensionNode(pKsTopologyInfo, GUID_UVCX_H264_XU, &m_stHardwareEncoder);
											pKsTopologyInfo->Release();
										}
										/// check property interface
										if (WaitForSingleObject(m_hMutexGetParams, INFINITE) == WAIT_OBJECT_0) {
											/// find IID_ISpecifyPropertyPages
											hr = m_pBuilder->FindInterface(NULL, NULL, m_pCaptureFilter, IID_ISpecifyPropertyPages, (void**)&m_pPropertyPages);
											/// find IID_IAMCrossbar
											hr = m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, m_pCaptureFilter, IID_IAMCrossbar, (void**)&m_pCrossbar);
											int checkHD = 0;
											if ( SUCCEEDED(hr) ) {
												CheckCrossbarPins();
												long pin = m_CrossbarPinsReal[m_devSettings.iChannel];
												m_pCrossbar->Route(FindOutPin(), pin);
												pin = 0;
												m_pCrossbar->get_IsRoutedTo(FindOutPin(), &pin);
												m_devSettings.iChannel = m_CrossbarPinsGUI[pin];
												checkHD = 1;
											}
											if (m_devSettings.iCheckHDInput == -1) m_devSettings.iCheckHDInput = checkHD;
											/// find IID_IAMAnalogVideoDecoder
											hr = m_pBuilder->FindInterface(NULL, NULL, m_pCaptureFilter, IID_IAMAnalogVideoDecoder, (void**)&m_pAnalogVideoDecoder);
											if ( SUCCEEDED(hr) ) {
												long format = m_devSettings.iVideoMode;
												m_pAnalogVideoDecoder->put_TVFormat(format);
												m_pAnalogVideoDecoder->get_TVFormat(&format);
												m_devSettings.iVideoMode = format;
											}
											/// find IID_IAMTVTuner
											hr = m_pBuilder->FindInterface(NULL, NULL, m_pCaptureFilter, IID_IAMTVTuner, (void**)&m_pTVTuner);
											ReleaseMutex(m_hMutexGetParams);
										}
#ifdef _SVKS_M_BUILD_
										IAMVideoProcAmp *pProcAmp = NULL;
										hr = m_pCaptureFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
										if ( SUCCEEDED(hr) ) {
											long val = 0;
											long flag = 0;
											hr = pProcAmp->Get(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &val, &flag);
											if ( SUCCEEDED(hr) ) {
												hr = pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, 1, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL);
											}
										}
#endif
										hr = S_OK;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	m_eDeviceState = DEVICE_INIT;
	if ( hr != S_OK ) {
		ShutdownGraph();
	}

	SetEvent(m_hWasInitGraph);

	return ((int)hr >= 0) ? 0 : 1;
}

void VS_CaptureDeviceDirectShow::ShutdownGraph()
{
	HRESULT hr = S_OK;
	if (m_eDeviceState == DEVICE_SHUTDOWN) return;
	if (m_eDeviceState != DEVICE_INIT) {
		/// teardown
		TearDownGraph();
	}
	/// shutdown
	if (WaitForSingleObject(m_hMutexGetParams, INFINITE) == WAIT_OBJECT_0) {
		if (m_pTVTuner) m_pTVTuner->Release(); m_pTVTuner = NULL;
		if (m_pAnalogVideoDecoder) m_pAnalogVideoDecoder->Release(); m_pAnalogVideoDecoder = NULL;// m_iVideoMode = 0;
		while (!m_qCrossbarPins.empty()) m_qCrossbarPins.pop_front();
		memset(m_CrossbarPinsReal, 0, sizeof(m_CrossbarPinsReal));
		memset(m_CrossbarPinsGUI, 0, sizeof(m_CrossbarPinsGUI));
		if (m_pCrossbar) m_pCrossbar->Release(); m_pCrossbar = NULL;// m_iChannel = 0;
		if (m_pPropertyPages) m_pPropertyPages->Release(); m_pPropertyPages = NULL;
		ReleaseMutex(m_hMutexGetParams);
	}
	if (m_pGraph) {
		DestroySubgraph(m_pGraph, m_pCaptureFilter);
		if (m_stHardwareEncoder.pKsControl) m_stHardwareEncoder.pKsControl->Release(); m_stHardwareEncoder.pKsControl = NULL;
		if (m_pMixerVideoPortFilter) m_pMixerVideoPortFilter->Release(); m_pMixerVideoPortFilter = NULL;
		if (m_pGrabberFilter) m_pGrabberFilter->Release(); m_pGrabberFilter = NULL;
		m_pGrabber = NULL; /// deleted after filter Release()
		if (m_pCaptureFilter) m_pCaptureFilter->Release(); m_pCaptureFilter = NULL;
		m_pGraph->Release(); m_pGraph = NULL;
	}
	m_pBuilder->Release(); m_pBuilder = NULL;
	ResetEvent(m_hWasInitGraph);

	m_lockFramerate.Lock();
	m_startFramerate = 3;
	m_setFramerate = m_last_setFramerate = 300;
	m_lockFramerate.UnLock();

	m_eDeviceState = DEVICE_SHUTDOWN;
}

int VS_CaptureDeviceDirectShow::BuildGraph()
{
	HRESULT hr = (HRESULT)(-1);

	if (m_eDeviceState != DEVICE_INIT) return -1;

	if (iGetDeviceModeList(m_CurrentDeviceName) < 0) return -2;
	int nMode = FindOptimalVideoMode(m_renderFmt, m_devSettings.iCheckHDInput);
	double dFrameRate = (double)m_captureFmt.dwFps / 100.0;
	m_captureFmt.dwFps = (unsigned int)(dFrameRate + 0.5);
	if (nMode < 0) return -3;

	AM_MEDIA_TYPE *pMt = NULL;
	IPin *pGrabberPin(NULL), *pCapturePin(NULL);

	CModeListDirectShow *pModeListDirectShow = reinterpret_cast<CModeListDirectShow*>(m_pModeList);
	int iPinNimber = pModeListDirectShow->iGetPinNumber(nMode);
	pCapturePin = GetPin(m_pCaptureFilter, iPinNimber, PINDIR_OUTPUT);
	if (!pCapturePin) {
		return -1;
	}
	CComQIPtr <IAMStreamConfig, &IID_IAMStreamConfig> pStreamConfig(pCapturePin);
	if (!pStreamConfig) return -4;
	pMt = (AM_MEDIA_TYPE*)pModeListDirectShow->GetModeID(nMode);
	if (!pMt) return -5;

	BITMAPINFOHEADER *pbmiHeader;
	if (pMt->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(pMt->pbFormat);
		pbmiHeader = &pVih->bmiHeader;
		pVih->AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 / dFrameRate);
	} else if (pMt->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2 *pVih = (VIDEOINFOHEADER2*)(pMt->pbFormat);
		pbmiHeader = &pVih->bmiHeader;
		pVih->AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 / dFrameRate);
	} else {
		return -5;
	}

	pbmiHeader->biHeight = m_captureFmt.dwVideoHeight;
	pbmiHeader->biWidth = m_captureFmt.dwVideoWidht;
	pbmiHeader->biSizeImage = DIBSIZE(*pbmiHeader);
	hr = pStreamConfig->SetFormat(pMt);

	DTRACE(VSTM_VCAPTURE, "device BUILD GRAPH: %S, SetFormat %4d x %4d @ %4.2f (%I64d)",
						  m_CurrentDeviceName, pbmiHeader->biWidth, pbmiHeader->biHeight, dFrameRate, ((VIDEOINFOHEADER*)(pMt->pbFormat))->AvgTimePerFrame);

	if ( SUCCEEDED(hr) ) {

		DTRACE(VSTM_VCAPTURE, "device BUILD GRAPH: SetFormat succeeded");

		m_eDeviceState = DEVICE_BUILD;
		/// find VideoDecoder Filter
		hr = (HRESULT)(-1);
		/*if (pMt->subtype == MEDIASUBTYPE_MJPG) {
			hr = CoCreateInstance(CLSID_MjpegDec, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&m_pVideoDecoderFilter);
		} else */if (pMt->subtype==MEDIASUBTYPE_dvsd || pMt->subtype==MEDIASUBTYPE_dvhd || pMt->subtype==MEDIASUBTYPE_dvsl) {
			hr = CoCreateInstance(CLSID_DVVideoCodec, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&m_pVideoDecoderFilter);
		} else if (pMt->subtype.Data1 == CColorSpace::FCC_BJPG) {
			hr = CoCreateInstance(CLSID_AVIDec, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&m_pVideoDecoderFilter);
		}
		if ( SUCCEEDED(hr) ) hr = m_pGraph->AddFilter(m_pVideoDecoderFilter, L"Video Decoder");
		if (SUCCEEDED(hr) && m_pVideoDecoderFilter) {
			hr = m_pBuilder->RenderStream(NULL, NULL, pCapturePin, m_pVideoDecoderFilter, m_pGrabberFilter);
		}
		else {
			m_pVideoDecoderFilter = NULL;
			hr = m_pBuilder->FindPin(m_pGrabberFilter, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pGrabberPin);
		}
		if (SUCCEEDED(hr)) {
			if (m_pVideoDecoderFilter) {
				pGrabberPin = GetPin(m_pGrabberFilter, 0, PINDIR_INPUT);
			}
			else {
				hr = m_pGraph->Connect(pCapturePin, pGrabberPin);
			}
			if (SUCCEEDED(hr)) {

				DTRACE(VSTM_VCAPTURE, "device BUILD GRAPH: RenderStream succeeded");

				AM_MEDIA_TYPE mt;
				hr = pGrabberPin->ConnectionMediaType(&mt);
				if (SUCCEEDED(hr)) {

					BITMAPINFOHEADER *pBmh = &(((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader);

					DTRACE(VSTM_VCAPTURE, "device BUILD GRAPH: ConnectionMediaType succeeded");

					m_pGrabber->SetFPS(m_renderFmt.dwFps * 100);
					uint32_t width(0), height(0), fourcc(0);
					hr = m_pGrabber->CheckMediaFormat(width, height, fourcc);

					if (SUCCEEDED(hr)) {

						if (m_pVideoDecoderFilter) {
							pBmh->biWidth = width;
							pBmh->biHeight = height;
							pBmh->biCompression = fourcc;
						}

						DTRACE(VSTM_VCAPTURE, "retrieve [%d x %d @ %d]], cpt [%d x %d @ %4.2f], rnd [%d x %d @ %d]",
							m_captureFmt.dwVideoWidht, m_captureFmt.dwVideoHeight, m_captureFmt.dwFps,
							pBmh->biWidth, pBmh->biHeight, 10000000.0 / (double)(((VIDEOINFOHEADER*)mt.pbFormat)->AvgTimePerFrame),
							m_renderFmt.dwVideoWidht, m_renderFmt.dwVideoHeight, m_renderFmt.dwFps);

						/// set hw options
						m_pGrabber->SetCaps(m_captureFmt.dwVideoWidht, m_captureFmt.dwVideoHeight, pBmh->biCompression, m_captureFmt.dwHWCodec == ENCODER_H264_LOGITECH);
						if (m_captureFmt.dwHWCodec == ENCODER_H264_LOGITECH) {
							if (m_stHardwareEncoder.pKsControl) {
								m_stHardwareEncoder.ResetState(pBmh->biWidth, pBmh->biHeight, m_captureFmt.dwFps);
								hr = SetHWEncoderState(&m_stHardwareEncoder);
							}
						}
						/// Start Graph
						hr = StartGraph();
					}
				}
			}
		}
	}
	if (pGrabberPin && m_pVideoDecoderFilter == NULL) {
		pGrabberPin->Release();
	}
	if ( FAILED(hr) ) {
		TearDownGraph();
	}

	return ((int)hr >= 0) ? 0 : 1;
}

void VS_CaptureDeviceDirectShow::TearDownGraph()
{
	HRESULT hr = S_OK;
	if (m_eDeviceState <= DEVICE_INIT) return;
	if (m_eDeviceState == DEVICE_START) {
		/// stop graph
		StopGraph();
	}
	m_pGraph->RemoveFilter(m_pVideoDecoderFilter);
	if (m_pVideoDecoderFilter) m_pVideoDecoderFilter->Release(); m_pVideoDecoderFilter = NULL;
	m_captureFmt.SetZero();
	m_eDeviceState = DEVICE_INIT;
}

HRESULT VS_CaptureDeviceDirectShow::StartGraph()
{
	HRESULT hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
	if (SUCCEEDED(hr)) {
		DTRACE(VSTM_VCAPTURE, "device BUILD GRAPH: Query IID_IMediaControl succeeded");
		OAFilterState state;
		hr = m_pMediaControl->Pause();
		if (hr == S_FALSE) {
			for (int i = 0; i < 20; ++i) {
				hr = m_pMediaControl->GetState(200, &state);
				if (hr != VFW_S_STATE_INTERMEDIATE) break;
			}
			if (hr == VFW_S_STATE_INTERMEDIATE) DTRACE(VSTM_VCAPTURE, "Filter graph took more than 4 seconds to transition state (Pause)");
		}
		if (SUCCEEDED(hr)) {
			hr = m_pMediaControl->Run();
			if (hr == S_FALSE) {
				OAFilterState state;
				for (int i = 0; i < 20; ++i) {
					hr = m_pMediaControl->GetState(200, &state);
					if (hr != VFW_S_STATE_INTERMEDIATE) break;
				}
				if (hr == VFW_S_STATE_INTERMEDIATE) DTRACE(VSTM_VCAPTURE, "Filter graph took more than 4 seconds to transition state (Start)");
			}
			if (SUCCEEDED(hr)) {
				m_eDeviceState = DEVICE_START;
			}
		}
	}
	return hr;
}

HRESULT VS_CaptureDeviceDirectShow::StopGraph()
{
	HRESULT hr = S_OK;
	OAFilterState state;
	m_eDeviceState = DEVICE_BUILD;
	hr = m_pMediaControl->Stop();
	if (m_pGrabber) {
		m_pGrabber->SetCaps(0, 0, 0, false);
	}
	ResetEvent(m_hUpdateState);
	if (hr == S_FALSE) {
		for (int i = 0; i < 20; ++i) {
			hr = m_pMediaControl->GetState(200, &state);
			if (hr != VFW_S_STATE_INTERMEDIATE) break;
		}
		if (hr == VFW_S_STATE_INTERMEDIATE) DTRACE(VSTM_VCAPTURE, "Filter graph took more than 4 seconds to transition state (Stop)");
	}
	if (m_pMediaControl) m_pMediaControl->Release(); m_pMediaControl = NULL;
	return hr;
}

/**
 **************************************************************************
 ****************************************************************************/
HRESULT VS_CaptureDeviceDirectShow::FindExtensionNode(IKsTopologyInfo* pKsTopologyInfo, GUID extensionGuid, VS_HardwareDSEncoderState *pHWState)
{
	DWORD numberOfNodes;
	HRESULT hr = S_OK;

	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);
	if (SUCCEEDED(hr)) {
		DWORD i;
		for (i = 0; i < numberOfNodes; i++) {
			GUID nodeGuid = {0};
			if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid))) {
				if (IsEqualGUID(KSNODETYPE_DEV_SPECIFIC, nodeGuid)) {
					IKsControl *pKsControl = NULL;
					hr = pKsTopologyInfo->CreateNodeInstance(i, IID_IKsControl, (VOID**)&pKsControl);
					if (SUCCEEDED(hr)) {
					    KSP_NODE s;
                        ULONG ulBytesReturned = 0;
                        s.Property.Set = extensionGuid;
                        s.Property.Id = UVCX_VIDEO_CONFIG_PROBE;
                        s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
                        s.NodeId = i;
                        hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(pHWState->h264State), sizeof(pHWState->h264State), &ulBytesReturned);
						if (SUCCEEDED(hr)) {
							pHWState->pKsControl = pKsControl;
							pHWState->iNode = i;
							pHWState->extensionGuid = extensionGuid;
							return hr;
						}
						pKsControl->Release();
					}
				}
			}
		}
		if (i == numberOfNodes) hr = (HRESULT)(-1);
	}

	return hr;
}

/**
 **************************************************************************
 ****************************************************************************/
HRESULT VS_CaptureDeviceDirectShow::GetHWEncoderState(VS_HardwareDSEncoderState *pHWState)
{
	HRESULT hr = S_OK;

    KSP_NODE s;
    ULONG ulBytesReturned = 0;
    s.Property.Set = pHWState->extensionGuid;
    s.Property.Id = UVCX_VIDEO_CONFIG_PROBE;
    s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = pHWState->iNode;
    hr = pHWState->pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(pHWState->h264State), sizeof(pHWState->h264State), &ulBytesReturned);

	return hr;
}

/**
 **************************************************************************
 ****************************************************************************/
HRESULT VS_CaptureDeviceDirectShow::SetHWEncoderState(VS_HardwareDSEncoderState *pHWState)
{
	HRESULT hr = S_OK;

    KSP_NODE s;
    ULONG ulBytesReturned = 0;
    s.Property.Set = pHWState->extensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = pHWState->iNode;
	s.Property.Id = UVCX_VIDEO_CONFIG_PROBE;
	hr = pHWState->pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(pHWState->h264State), sizeof(pHWState->h264State), &ulBytesReturned);
	s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
	hr = pHWState->pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(pHWState->h264State), sizeof(pHWState->h264State), &ulBytesReturned);
	s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
	s.Property.Id = UVCX_VIDEO_CONFIG_COMMIT;
    hr = pHWState->pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(pHWState->h264State), sizeof(pHWState->h264State), &ulBytesReturned);

	return hr;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_CaptureDeviceDirectShow::UpdateHWEncoderState(eHardwareRequest request, unsigned int iVal)
{
	KSP_NODE s;
    ULONG ulBytesReturned = 0;
    s.Property.Set = m_stHardwareEncoder.extensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = m_stHardwareEncoder.iNode;

	LPVOID pPropertyData = 0;
	ULONG dataLen = 0;
	_uvcx_bitrate_layers_t stBtr;
	_uvcx_framerate_config_t stFPS;
	_uvcx_encoder_reset stReset;
	_uvcx_picture_type_control_t stKey;

	switch (request)
	{
	case HARDWARE_SETBITRATE:
		s.Property.Id = UVCX_BITRATE_LAYERS;
		stBtr.wLayerID = 0x0;
		stBtr.dwAverageBitrate = (iVal >> 16);
		stBtr.dwPeakBitrate = (iVal << 16) | (iVal >> 16);
		pPropertyData = &stBtr;
		dataLen = sizeof(_uvcx_bitrate_layers_t);
		break;
	case HARDWARE_FRAMERATE:
		s.Property.Id = UVCX_FRAMERATE_CONFIG;
		stFPS.wLayerID = 0x0;
		stFPS.dwFrameInterval = (iVal >> 16);
		pPropertyData = &stFPS;
		dataLen = sizeof(_uvcx_framerate_config_t);
		break;
	case HARDWARE_RESET:
		s.Property.Id = UVCX_ENCODER_RESET;
		stReset.wLayerID = 0x0;
		pPropertyData = &stReset;
		dataLen = sizeof(_uvcx_encoder_reset);
		break;
	case HARDWARE_NEEDKEY:
		s.Property.Id = UVCX_PICTURE_TYPE_CONTROL;
		stKey.wLayerID = 0x0;
		stKey.wPicType = 0x0002;
		pPropertyData = &stKey;
		dataLen = sizeof(_uvcx_picture_type_control_t);
		break;
	default: break;
	}

	m_stHardwareEncoder.pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), pPropertyData, dataLen, &ulBytesReturned);
}

void VS_CaptureDeviceDirectShow::SetHWEncoderRequest(eHardwareRequest request, unsigned int iVal)
{
	switch (request)
	{
	case HARDWARE_SETBITRATE:
		m_stHardwareEncoder.bitrate = iVal;
		SetEvent(m_hSetBitrateHW);
		break;
	case HARDWARE_NEEDKEY:
		SetEvent(m_hNeedIFrameHW);
		break;
	case HARDWARE_RESET:
		SetEvent(m_hResetHW);
		break;
	default: break;
	}
}

int VS_CaptureDeviceDirectShow::ConnectGraph()
{
	int ret = -1;

	if (m_eDeviceState == DEVICE_START && m_pCaptureList->IsAverMediaDevice(m_CurrentDeviceName) && m_devSettings.iCheckHDInput > 0) {
		/// hack for AverMedia U3
		ret = StopGraph();
		if (ret == S_OK) {
			AM_MEDIA_TYPE mt;
			BITMAPINFOHEADER *pBmh = NULL;
			IPin *pGrabberPin(NULL);
			HRESULT hr = m_pBuilder->FindPin(m_pGrabberFilter, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pGrabberPin);
			if ( SUCCEEDED(hr) ) {
				hr = pGrabberPin->ConnectionMediaType(&mt);
				if ( SUCCEEDED(hr) ) {
					ret = StartGraph();
					m_setFramerate = m_renderFmt.dwFps * 100;
					m_realFramerate = m_setFramerate;
				}
				pGrabberPin->Release();
			}
		}
	} else {
		InitGraph();
		ret = BuildGraph();
		if (m_devSettings.iFixNTSC > 0) {
			if (m_pCaptureList->UpdateDeviceModeList(m_CurrentDeviceName)) {
				InitGraph();
				ret = BuildGraph();
				if (m_pCaptureList->UpdateDeviceModeList(m_CurrentDeviceName)) {
					InitGraph();
					ret = BuildGraph();
				}
			}
		}
	}

	return ret;
}

DWORD VS_CaptureDeviceDirectShow::Loop(LPVOID hEvDie)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	bool exit = false;
	HANDLE Handles[6] = { hEvDie,
						 m_hUpdateState,
						 m_hGetPropertyPage,
						 m_hSetBitrateHW,
						 m_hNeedIFrameHW,
						 m_hResetHW};

	do {
		DWORD waitRes = WaitForMultipleObjects(6, Handles, FALSE, 100);

		if (waitRes == WAIT_TIMEOUT) {
			Lock();
			if (!m_qDevState.empty()) {
				waitRes = WAIT_OBJECT_0 + 1;
			}
			UnLock();
		}

		switch(waitRes) {
			case WAIT_FAILED:
			case WAIT_OBJECT_0 + 0: /// End Of Thread
				{
					m_lockCapture.Lock();
					ShutdownGraph();
					m_lockCapture.UnLock();

					exit = true;
					break;
				}
			case WAIT_OBJECT_0 + 1: /// Change Device State
				{
					VS_CaptureDeviceState devState;

					Lock();
					if (!m_qDevState.empty()) {
						memcpy(&devState, &m_qDevState.front(), sizeof(VS_CaptureDeviceState));
						m_qDevState.pop();
					}
					UnLock();

					switch (devState.eAction) {
						case DEVICE_CONNECT:
							{
								m_renderFmt = devState.mf;
								m_nOptimalVideoMode = devState.iOptimalMode;

								if (!devState.cDeviceName || !*devState.cDeviceName) {
									*m_CurrentDeviceName = 0;
									DTRACE(VSTM_VCAPTURE, "Connect capture: INCORRECT device name");
									break;
								}

								wcscpy(m_CurrentDeviceName, devState.cDeviceName);
								DTRACE(VSTM_VCAPTURE, "Set DEVICE_CONNECT: %S, [%d x %d @ %d], %d",
														m_CurrentDeviceName, m_renderFmt.dwVideoWidht, m_renderFmt.dwVideoHeight, m_renderFmt.dwFps, m_nOptimalVideoMode);

								m_lockCapture.Lock();
								int ret = ConnectGraph();
								g_DevStatus.SetStatus(DVS_SND_NOTWORK, false, ret != 0);
								DTRACE(VSTM_VCAPTURE, "Connect capture: %S, ret = %d", m_CurrentDeviceName, ret);
								m_lockCapture.UnLock();

								m_lockFramerate.Lock();
								m_startFramerate = (int)m_renderFmt.dwFps;
								m_setFramerate = m_startFramerate * 100;
								m_lockFramerate.UnLock();

								break;
							}
						case DEVICE_DISCONNECT:
							{
								DTRACE(VSTM_VCAPTURE, "Set DEVICE_DISCONNECT: %S", m_CurrentDeviceName);

								m_lockCapture.Lock();
								ShutdownGraph();
								DTRACE(VSTM_VCAPTURE, "Disconnect capture");
								m_lockCapture.UnLock();

								break;
							}
					}

					break;
				}
			case WAIT_OBJECT_0 + 2: /// Property Page
				{
					m_lockCapture.Lock();
					if (m_eDeviceState >= DEVICE_INIT && m_pCaptureFilter && m_pPropertyPages) {
						// Get the filter's name and IUnknown pointer.
						FILTER_INFO FilterInfo;
						m_pCaptureFilter->QueryFilterInfo(&FilterInfo);
						IUnknown *pFilterUnk;
						m_pCaptureFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
						// Show the page.
						CAUUID caGUID;
						m_pPropertyPages->GetPages(&caGUID);
						m_lockCapture.UnLock();
						OleCreatePropertyFrame(
							m_hwndPropPage,	// Parent window
							0, 0,				// (Reserved)
							FilterInfo.achName, // Caption for the dialog box
							1,					// Number of objects (just the filter)
							&pFilterUnk,		// Array of object pointers.
							caGUID.cElems,		// Number of property pages
							caGUID.pElems,		// Array of property page CLSIDs
							0,					// Locale identifier
							0, NULL				// Reserved
							);
						// Clean up.
						pFilterUnk->Release();
						FilterInfo.pGraph->Release();
						CoTaskMemFree(caGUID.pElems);
					}
					break;
				}
			case WAIT_OBJECT_0 + 3: /// Set HW Bitrate
				{
					m_lockCapture.Lock();
					if (m_eDeviceState == DEVICE_START && m_stHardwareEncoder.pKsControl)
						UpdateHWEncoderState(HARDWARE_SETBITRATE, m_stHardwareEncoder.bitrate);
					m_lockCapture.UnLock();
					break;
				}
			case WAIT_OBJECT_0 + 4: /// Request HW I-frame
				{
					m_lockCapture.Lock();
					if (m_eDeviceState == DEVICE_START && m_stHardwareEncoder.pKsControl)
						UpdateHWEncoderState(HARDWARE_NEEDKEY);
					m_lockCapture.UnLock();
					break;
				}
			case WAIT_OBJECT_0 + 5: /// Reset HW
				{
					m_lockCapture.Lock();
					if (m_eDeviceState == DEVICE_START && m_stHardwareEncoder.pKsControl) {
						UpdateHWEncoderState(HARDWARE_RESET);
						UpdateHWEncoderState(HARDWARE_SETBITRATE, 8000 * 1000);
					}
					m_lockCapture.UnLock();
					break;
				}
		}

		if (m_eDeviceState >= DEVICE_INIT) {
			bool bset = false;
			int framerate = 300;

			m_lockFramerate.Lock();
			bset = m_last_setFramerate != m_setFramerate;
			if (bset) {
				m_last_setFramerate = m_setFramerate;
				DTRACE(VSTM_VCAPTURE, "Set Framerate capture: %S, fps = %d", m_CurrentDeviceName, m_last_setFramerate);
			}
			m_lockFramerate.UnLock();

			m_lockCapture.Lock();
			if (m_eDeviceState == DEVICE_START && bset) {
				if (m_captureFmt.dwHWCodec == ENCODER_H264_LOGITECH && m_stHardwareEncoder.pKsControl) UpdateHWEncoderState(HARDWARE_FRAMERATE, m_last_setFramerate);
				else {
					if (m_pGrabber) {
						m_pGrabber->SetFPS(m_last_setFramerate);
					}
				}
			}
			m_lockCapture.UnLock();
		}

	} while (!exit);

	CoUninitialize();
	return 0;
}