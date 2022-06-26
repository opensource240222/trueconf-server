#include "VSAudioNew.h"
#include <Endpointvolume.h>
#include <mmdeviceapi.h>
#include "VS_VolControl.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "../Audio/VoiceActivity/VS_Mixer.h"
#include "VS_Dmodule.h"
#include <stdlib.h>
#include <math.h>
#include <thread>

bool VSIsDXVersion10();

struct cAudioVolumeCallback : IAudioEndpointVolumeCallback
{
private:
	ULONG				   m_uRef;
	GUID				   m_guidCtx;

public:
	cAudioVolumeCallback(GUID ctx);
	~cAudioVolumeCallback();

	HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
};

cAudioVolumeCallback::cAudioVolumeCallback(GUID ctx)
{
	m_guidCtx = ctx;
	m_uRef = 1;
}

cAudioVolumeCallback::~cAudioVolumeCallback()
{
}

HRESULT cAudioVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (pNotify == NULL) {
        return E_INVALIDARG;
    }
	if (pNotify->guidEventContext != m_guidCtx) {
		PostMessage(VS_AudioMixerVolume::GetWnd(), MM_MIXM_CONTROL_CHANGE, 42, 0);
	}
	return S_OK;
}

HRESULT cAudioVolumeCallback::QueryInterface(const IID &riid, void **ppvObject)
{
	if (IID_IUnknown == riid) {
		AddRef();
		*ppvObject = (IUnknown*)this;
	} else if (__uuidof(IAudioEndpointVolumeCallback) == riid) {
		AddRef();
		*ppvObject = (IAudioEndpointVolumeCallback*)this;
	} else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG cAudioVolumeCallback::AddRef()
{
	return InterlockedIncrement(&m_uRef);
}

ULONG cAudioVolumeCallback::Release()
{
    ULONG ulRef = InterlockedDecrement(&m_uRef);
    if (0 == ulRef) {
        delete this;
    }
    return ulRef;
}

/*******************************************************
/ VS_VolumeCaptureControlWin
/*******************************************************/

#define MIN_BOOST_CHUNK	(10.0)

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}

VS_VolumeCaptureControlWin::VS_VolumeCaptureControlWin()
{
	m_audioVolume = nullptr;
	m_audioBoost = nullptr;
	m_minBoostDb = 0.0;
	m_maxBoostDb = 0.0;
	m_stepBoostDb = 0.0;
	m_halBoostDb = 0.0;
	m_minMicDb = 0.0;
	m_maxMicDb = 0.0;
	m_stepMicDb = 0.0;
	m_minMicScalar = 0.0;
	m_guidMyCtx = GUID_NULL;
}

VS_VolumeCaptureControlWin::~VS_VolumeCaptureControlWin()
{
	ReleaseInternal();
}

HRESULT VS_VolumeCaptureControlWin::CollectInf(IPart *pPart, std::vector <IPart*> & list, int recursiveLimit)
{
	if (!recursiveLimit)
		return S_OK;

	HRESULT hr = S_OK;
	LPWSTR pwszPartName = NULL;
	CoTaskMemFree(pwszPartName);
	IAudioVolumeLevel *pVolume = NULL;
	hr = pPart->Activate(CLSCTX_ALL, __uuidof(IAudioVolumeLevel), (void**)&pVolume);
	if (E_NOINTERFACE != hr) {
		if (FAILED(hr)) {
			return hr;
		}
		else {
			list.push_back(pPart);
			pVolume->Release();
		}
	}
	IPartsList *pOutgoingParts = NULL;
	hr = pPart->EnumPartsOutgoing(&pOutgoingParts);
	if (E_NOTFOUND == hr) {
		return S_OK;
	}
	UINT nParts = 0;
	hr = pOutgoingParts->GetCount(&nParts);
	for (UINT n = 0; n < nParts; n++) {
		IPart *pOutgoingPart = NULL;
		hr = pOutgoingParts->GetPart(n, &pOutgoingPart);
		hr = CollectInf(pOutgoingPart, list, recursiveLimit - 1);
		pOutgoingPart->Release();
	}
	pOutgoingParts->Release();
	return S_OK;
}

bool VS_VolumeCaptureControlWin::InitInternal(const wchar_t* name)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	hr = CoCreateGuid(&m_guidMyCtx);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	IMMDeviceEnumerator *pEnum = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	IMMDevice * pDevice = NULL;
	IDeviceTopology * pDT = NULL;
	IMMDeviceCollection * pCollection = NULL;
	IPropertyStore * pProps = NULL;
	hr = pEnum->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) {
		SafeRelease(&pEnum);
		CoUninitialize();
		return false;
	}
	UINT count_devices;
	hr = pCollection->GetCount(&count_devices);
	if (FAILED(hr)) {
		SafeRelease(&pCollection);
		SafeRelease(&pEnum);
		CoUninitialize();
		return false;
	}
	hr = (HRESULT)(-1);
	PROPERTYKEY keyPath = { { 0x233164C8, 0x1B2C, 0x4C7D, 0xBC, 0x68, 0xB6, 0x71, 0x68, 0x7A, 0x25, 0x67 }, 1 };
	const std::wstring _wstr(name);
	for (UINT i = 0; i < count_devices; i++) {
		hr = pCollection->Item(i, &pDevice);
		if (SUCCEEDED(hr)) {
			hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
			if (SUCCEEDED(hr)) {
				PROPVARIANT varName;
				PropVariantInit(&varName);
				hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
				if (SUCCEEDED(hr) && varName.pwszVal) {
					std::wstring nameDev(varName.pwszVal);
					hr = (nameDev == _wstr) ? S_OK : (HRESULT)(-1);
				}
				PropVariantClear(&varName);
			}
			SafeRelease(&pProps);
			if (hr == S_OK) {
				break;
			}
		}
		SafeRelease(&pDevice);
	}
	SafeRelease(&pCollection);
	SafeRelease(&pEnum);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		ReleaseInternal();
		CoUninitialize();
		return false;
	}
	hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDT);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		CoUninitialize();
		return false;
	}
	IConnector *pConnEndpoint = NULL;
	hr = pDT->GetConnector(0, &pConnEndpoint);
	pDT->Release();
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		CoUninitialize();
		return false;
	}
	IConnector *pConnDevice = NULL;
	hr = pConnEndpoint->GetConnectedTo(&pConnDevice);
	SafeRelease(&pConnEndpoint);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		CoUninitialize();
		return false;
	}
	IPart *pPart = NULL;
	hr = pConnDevice->QueryInterface(__uuidof(IPart), (void**)&pPart);
	SafeRelease(&pConnDevice);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		CoUninitialize();
		return false;
	}
	std::vector <IPart*> listParts;
	hr = CollectInf(pPart, listParts, 32);
	SafeRelease(&pPart);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		CoUninitialize();
		return false;
	}

	LPWSTR Name = NULL;

	char s_search1[] = "сил";
	char s_search2[] = "oost";

	bool ret = false;

	if (!listParts.empty()) {
		IPart * part_last = NULL;
		for (int i = 0; i < listParts.size(); i++) {
			part_last = listParts[i];
			hr = part_last->GetName(&Name);
			if (FAILED(hr)) {
				CoTaskMemFree(Name);
				SafeRelease(&pDevice);
				ReleaseInternal();
				CoUninitialize();
				return false;
			}
			int length = WideCharToMultiByte(CP_ACP, 0, Name, -1, 0, 0, NULL, NULL);
			char * item_name = new char[length];
			WideCharToMultiByte(CP_ACP, 0, Name, -1, item_name, length, NULL, NULL);
			CoTaskMemFree(Name);
			hr = (HRESULT)(-1);
			if (strstr(item_name, s_search1) || strstr(item_name, s_search2)) {
				hr = S_OK;
				delete[] item_name;
				break;
			}
			else {
				delete[] item_name;
			}
		}
		if (SUCCEEDED(hr)) {
			hr = part_last->Activate(CLSCTX_ALL, __uuidof(IAudioVolumeLevel), (void**)&m_audioBoost);
			if (hr == S_OK) {
				hr = m_audioBoost->GetLevelRange(0, &m_minBoostDb, &m_maxBoostDb, &m_stepBoostDb);
				if (hr == S_OK) {


					DTRACE(VSTM_AGCIN, "orig settings boost vol %s : step = %4.2f [%4.2f ... %4.2f]", name, m_stepBoostDb, m_minBoostDb, m_maxBoostDb);


					m_maxBoostDb = std::max(0.0f, m_maxBoostDb);
					m_minBoostDb = std::max(0.0f, std::min(m_minBoostDb, m_maxBoostDb));
					if (m_minBoostDb >= 24.0f) {
						m_maxBoostDb = m_minBoostDb;
					}
					float ddb = m_maxBoostDb - m_minBoostDb;
					if (ddb > MIN_BOOST_CHUNK) {
						m_stepBoostDb *= ceil(MIN_BOOST_CHUNK / m_stepBoostDb);
						int d = ddb / m_stepBoostDb;
						if (d < 2) {
							m_stepBoostDb = ddb;
						}
					}
					else {
						m_stepBoostDb = ddb;
					}
					if (m_stepBoostDb > 0.0f) {
						if (m_maxBoostDb > 20.0f) {
							ddb = 20.0f - m_minBoostDb;
						}
						m_maxBoostDb = m_minBoostDb + m_stepBoostDb * ceil(ddb / m_stepBoostDb);
					}


					DTRACE(VSTM_AGCIN, "modif settings boost vol %s : step = %4.2f [%4.2f ... %4.2f]", name, m_stepBoostDb, m_minBoostDb, m_maxBoostDb);


				}
			}
		}
		hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&m_audioVolume);
		if (SUCCEEDED(hr)) {
			hr = m_audioVolume->GetVolumeRange(&m_minMicDb, &m_maxMicDb, &m_stepMicDb);
			if (SUCCEEDED(hr)) {
				m_minMicScalar = pow(10.0, (m_minMicDb - m_maxMicDb) / 20.0);
				ret = true;
			}

			m_AudioVolumeCallback = new cAudioVolumeCallback(m_guidMyCtx);
			hr = m_audioVolume->RegisterControlChangeNotify((IAudioEndpointVolumeCallback*)m_AudioVolumeCallback);

			if (!SUCCEEDED(hr))
			{
				delete m_AudioVolumeCallback;
				m_AudioVolumeCallback = nullptr;
			}
		}
		listParts.clear();
		SafeRelease(&pDevice);
	}

	if (ret)
		m_Init = true;

	DTRACE(VSTM_AGCIN, "m_minMicDb %f, m_maxMicDb %f, m_stepMicDb %f", m_minMicDb, m_maxMicDb, m_stepMicDb);

	return ret;
}

void VS_VolumeCaptureControlWin::ReleaseInternal()
{
	if (IsValid()) {
		if (m_AudioVolumeCallback)
		{
			m_audioVolume->UnregisterControlChangeNotify((IAudioEndpointVolumeCallback*)m_AudioVolumeCallback);

			delete m_AudioVolumeCallback;

			m_AudioVolumeCallback = nullptr;
		}

		SafeRelease(&m_audioVolume);
		SafeRelease(&m_audioBoost);
		m_guidMyCtx = GUID_NULL;
		m_minBoostDb = 0.0;
		m_maxBoostDb = 0.0;
		m_stepBoostDb = 0.0;
		m_halBoostDb = 0.0;
		m_minMicDb = 0.0;
		m_maxMicDb = 0.0;
		m_stepMicDb = 0.0;
		m_minMicScalar = 0.0;
		CoUninitialize();
	}

	m_Init = false;
}

void VS_VolumeCaptureControlWin::SetBoost(float boost)
{
	HRESULT hr = S_FALSE;
	if (IsValid() && m_audioBoost) {
		hr = m_audioBoost->SetLevelUniform(boost, &m_guidMyCtx);
	}
}

bool VS_VolumeCaptureControlWin::GetBoost(float *boost)
{
	HRESULT hr = S_FALSE;
	if (IsValid() && m_audioBoost) {
		hr = m_audioBoost->GetLevel(0, boost);
	}
	return SUCCEEDED(hr);
}

void VS_VolumeCaptureControlWin::SetClientToDeviceVolume(float vol)
{


	DTRACE(VSTM_AGCIN, "SetClientToDeviceVolume client increase vol: vol = %4.2f", vol);


	HRESULT hr = S_FALSE;
	if (IsValid()) {
		hr = m_audioVolume->SetMasterVolumeLevelScalar(vol, &m_guidMyCtx);
	}
}

void VS_VolumeCaptureControlWin::SetAgcToDeviceVolume(float vol)
{
	DTRACE(VSTM_AGCIN, "SetAgcToDeviceVolume() %3.2f hal from agc", vol, vol);

	HRESULT hr = S_FALSE;
	if (IsValid()) {
		float newDb = 20 * log10f(vol * (1.0 - m_minMicScalar) + m_minMicScalar) + m_maxMicDb;
		if (!m_audioBoost || m_stepBoostDb == 0.0) {

			DTRACE(VSTM_AGCIN, "SetAgcToDeviceVolume hal set vol: db = %3.2f", newDb);

			hr = m_audioVolume->SetMasterVolumeLevel(newDb, &m_guidMyCtx);
		}
		else {
			if (vol >= 1.0 && m_halBoostDb < m_maxBoostDb) {
				if (GetBoost(&m_halBoostDb)) {
					m_halBoostDb += m_stepBoostDb;
					float setDb = m_maxMicDb - m_stepBoostDb;
					if (setDb < m_minMicDb) {
						setDb = m_minMicDb;
					}
					if (m_halBoostDb > m_maxBoostDb) {
						m_halBoostDb = m_maxBoostDb;
						setDb = m_maxMicDb;
					}
					SetBoost(m_halBoostDb);


					DTRACE(VSTM_AGCIN, "SetAgcToDeviceVolume hal increase boost: vol = %4.2fDb, boost = %4.2fDb", setDb, m_halBoostDb);


					hr = m_audioVolume->SetMasterVolumeLevel(setDb, &m_guidMyCtx);
				}
				else {


					DTRACE(VSTM_AGCIN, "SetAgcToDeviceVolume false hal increase boost: vol = %4.2f", vol);


					hr = m_audioVolume->SetMasterVolumeLevel(newDb, &m_guidMyCtx);
				}
			}
			else {
				float db = m_maxMicDb - newDb;
				if (GetBoost(&m_halBoostDb)) {
					if ((db > m_stepBoostDb + 10) && m_halBoostDb > m_minBoostDb) {
						float prevBoost = m_halBoostDb;
						m_halBoostDb -= m_stepBoostDb;
						float setDb = newDb + m_stepBoostDb;
						if (setDb > m_maxMicDb) {
							setDb = m_maxMicDb;
						}
						if (m_halBoostDb < m_minBoostDb) {
							m_halBoostDb = m_minBoostDb;
							setDb = m_minMicDb;
						}
						SetBoost(m_halBoostDb);


						DTRACE(VSTM_AGCIN, "hal decrease boost: vol = %4.2fDb, boost = %4.2fDb [%4.2f - (%4.2f + %4.2f) > %4.2f]",
							setDb, m_halBoostDb, m_maxMicDb, newDb, prevBoost, m_stepBoostDb + 10);


						hr = m_audioVolume->SetMasterVolumeLevel(setDb, &m_guidMyCtx);
					}
					else {


						DTRACE(VSTM_AGCIN, "hal set vol: db = %3.2f", newDb);


						hr = m_audioVolume->SetMasterVolumeLevel(newDb, &m_guidMyCtx);
					}
				}
				else {


					DTRACE(VSTM_AGCIN, "false hal anylize boost");


				}
			}
		}
	}
}

bool VS_VolumeCaptureControlWin::GetDeviceInfo(float *lvl, float *agcLvl)
{
	HRESULT hr = S_FALSE;
	if (IsValid()) {
		float halDB(0.0);

		if (lvl)
			hr = m_audioVolume->GetMasterVolumeLevelScalar(lvl);

		if (hr == S_OK) {


			DTRACE(VSTM_AGCIN, "update client vol: vol = %4.2f", *lvl);


		}
		hr = m_audioVolume->GetMasterVolumeLevel(&halDB);
		if (hr == S_OK && agcLvl) {
			float volHal = pow(10.0, (halDB - m_maxMicDb) / 20.0);
			*agcLvl = (volHal - m_minMicScalar) / (1.0 - m_minMicScalar);


			DTRACE(VSTM_AGCIN, "update agc vol: vol = %4.2f [db = %4.2f]", *agcLvl, halDB);


		}
	}
	return (hr == S_OK);
}

bool VS_VolumeCaptureControlWin::IsValid()
{
	return m_Init;
}

/*******************************************************
/ VS_VolumeRendererControlWin
/*******************************************************/

VS_VolumeRendererControlWin::VS_VolumeRendererControlWin()
{
	m_audioVolume = nullptr;
}

VS_VolumeRendererControlWin::~VS_VolumeRendererControlWin()
{
	ReleaseInternal();
}

bool VS_VolumeRendererControlWin::InitInternal(const wchar_t* name)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	hr = CoCreateGuid(&m_guidMyCtx);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	IMMDeviceEnumerator *pEnum = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
	if (FAILED(hr)) {
		CoUninitialize();
		return false;
	}
	IMMDevice * pDevice = NULL;
	IDeviceTopology * pDT = NULL;
	IMMDeviceCollection * pCollection = NULL;
	IPropertyStore * pProps = NULL;
	hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) {
		SafeRelease(&pEnum);
		CoUninitialize();
		return false;
	}
	UINT count_devices;
	hr = pCollection->GetCount(&count_devices);
	if (FAILED(hr)) {
		SafeRelease(&pCollection);
		SafeRelease(&pEnum);
		CoUninitialize();
		return false;
	}
	hr = (HRESULT)(-1);
	PROPERTYKEY keyPath = { { 0x233164C8, 0x1B2C, 0x4C7D, 0xBC, 0x68, 0xB6, 0x71, 0x68, 0x7A, 0x25, 0x67 }, 1 };
	const std::wstring _wstr(name);
	for (UINT i = 0; i < count_devices; i++) {
		hr = pCollection->Item(i, &pDevice);
		if (SUCCEEDED(hr)) {
			hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
			if (SUCCEEDED(hr)) {
				PROPVARIANT varName;
				PropVariantInit(&varName);
				hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
				if (SUCCEEDED(hr) && varName.pwszVal) {
					std::wstring pathName(varName.pwszVal);
					hr = (pathName == _wstr) ? S_OK : (HRESULT)(-1);
				}
				PropVariantClear(&varName);
			}
			SafeRelease(&pProps);
			if (hr == S_OK) {
				break;
			}
		}
		SafeRelease(&pDevice);
	}
	SafeRelease(&pCollection);
	SafeRelease(&pEnum);
	if (FAILED(hr)) {
		SafeRelease(&pDevice);
		ReleaseInternal();
		CoUninitialize();
		return false;
	}
	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&m_audioVolume);
	pDevice->Release();
	bool ret(false);
	if (SUCCEEDED(hr)) {

		m_AudioVolumeCallback = new cAudioVolumeCallback(m_guidMyCtx);
		hr = m_audioVolume->RegisterControlChangeNotify((IAudioEndpointVolumeCallback*)m_AudioVolumeCallback);

		if (!SUCCEEDED(hr))
		{
			delete m_AudioVolumeCallback;
			m_AudioVolumeCallback = nullptr;
		}

		ret = true;
	}
	else {
		ReleaseInternal();
		CoUninitialize();
	}

	if (ret)
		m_Init = true;

	return ret;
}

void VS_VolumeRendererControlWin::ReleaseInternal()
{
	if (IsValid()) {
		if (m_AudioVolumeCallback)
		{
			m_audioVolume->UnregisterControlChangeNotify((IAudioEndpointVolumeCallback*)m_AudioVolumeCallback);

			delete m_AudioVolumeCallback;

			m_AudioVolumeCallback = nullptr;
		}

		SafeRelease(&m_audioVolume);
		m_guidMyCtx = GUID_NULL;
		CoUninitialize();
	}

	m_Init = false;
}

void VS_VolumeRendererControlWin::SetClientToDeviceVolume(float vol)
{
	if (IsValid()) {
		HRESULT hr = m_audioVolume->SetMasterVolumeLevelScalar(vol, &m_guidMyCtx);
	}
}

bool VS_VolumeRendererControlWin::GetDeviceInfo(float *vol)
{
	if (!IsValid()) {
		return false;
	}
	HRESULT hr = m_audioVolume->GetMasterVolumeLevelScalar(vol);
	return (SUCCEEDED(hr)) ? true : false;
}

bool VS_VolumeRendererControlWin::IsValid()
{
	return m_Init;
}

/*******************************************************
/ VS_VolControlBase
/*******************************************************/

VS_VolControlNew::VS_VolControlNew()
{
}

VS_VolControlNew::~VS_VolControlNew()
{
	Release();
}

int VS_VolControlNew::Init(wchar_t *szname, int devID, bool isRender)
{
	Release();

	if (isRender)
	{
		m_Render = new VS_VolumeRendererControlWin();
		volumeOK = m_Render->InitInternal(szname);
	}
	else
	{
		m_Capture = new VS_VolumeCaptureControlWin();
		volumeOK = m_Capture->InitInternal(szname);
	}

	return volumeOK;
}

void VS_VolControlNew::Release()
{
	if (m_Render)
		delete m_Render;
	else if (m_Capture)
		delete m_Capture;
}

int VS_VolControlNew::GetMicBoost(float * boost)
{
	return 0;
}

int VS_VolControlNew::SetMicBoost(float Boost_Db)
{
	return 0;
}

int VS_VolControlNew::GetBoostParam(float * minDb, float * maxDb, float * stepDb)
{
	return 0;
}

int VS_VolControlNew::GetMicVolume(float *vol) // return device level
{
	if (m_Render)
		return m_Render->GetDeviceInfo(vol);
	else if (m_Capture)
		return m_Capture->GetDeviceInfo(vol, nullptr);

	return 0;
}

int VS_VolControlNew::GetMicVolumeRangedB(float * mindB, float * maxdB, float * stepdB)
{
	return 0;
}

int VS_VolControlNew::GetMicVolumedB(float * volumedB) // return agc level
{
	if (m_Capture)
		return m_Capture->GetDeviceInfo(nullptr, volumedB);

	return 0;
}

int VS_VolControlNew::SetMicVolume(float volume) // ClientToDevice
{
	if (m_Render)
		m_Render->SetClientToDeviceVolume(volume);
	else if (m_Capture)
		m_Capture->SetClientToDeviceVolume(volume);

	return 0;
}

int VS_VolControlNew::SetMicVolumedB(float volumedB)
{
	return 0;
}

int VS_VolControlNew::SetAgcToDeviceVolume(float volumedB) // AgcToDevice
{
	if (m_Capture)
		m_Capture->SetAgcToDeviceVolume(volumedB);

	return 0;
}

int VS_VolControlNew::ChangedB(double voldB)
{
	return 0;
}

/*******************************************************
/ VS_VolControlBase
/*******************************************************/

VS_VolControlBase * VS_VolControlBase::Factory()
{
	if (VSIsDXVersion10()) {
		return new VS_VolControlNew();
	} else {
		return new VS_VolControlXP();
	}
}

bool VS_VolControlBase::IsValid()
{
	return volumeOK;
}

/*******************************************************
/ VS_VolControlXP
/*******************************************************/

VS_VolControlXP::VS_VolControlXP()
{
	m_vol=0;
	m_isInit=false;
	m_BoostFeature=false;
	m_HIGH_DB=42;
	m_LOW_DB=0;
	m_STEP_DB=0.1;
}

VS_VolControlXP::~VS_VolControlXP()
{
	Release();
}

int VS_VolControlXP::Init(wchar_t *szname, int devID, bool isRender)
{
	Release();
	int res;

	m_vol = new VS_AudioMixerVolume();

	if (devID < 0)
		devID = 0;

	res = m_vol->Init(devID, isRender ? VS_AudioMixerVolume::MTYPE_WOUT : VS_AudioMixerVolume::MTYPE_MIC);
	if (!isRender) {
		res = XP_MixerBoostInit(szname);
	}

	m_isInit = true;
	volumeOK = true;

	return 0;
}

void VS_VolControlXP::Release()
{
	if(m_vol) delete m_vol; m_vol=0;
	m_isInit=false;
	m_BoostFeature=false;
}



int VS_VolControlXP::XP_MixerBoostInit(wchar_t *devName)
{
	HMIXER mixer;
	MIXERCAPS mcaps;
	MMRESULT mmres;
	unsigned int numDev = mixerGetNumDevs();
	for (unsigned int dev = 0; dev < numDev; dev++) {
	    mixerGetDevCaps(dev, &mcaps, sizeof(MIXERCAPS));
		_bstr_t bs = mcaps.szPname;
		if (wcsstr(devName, (wchar_t*)bs)) {
			mmres = mixerOpen(&mixer, dev, 0, 0, MIXER_OBJECTF_HMIXER);
			if (mmres== MMSYSERR_NOERROR)
			{
				for (DWORD j = 0; j< mcaps.cDestinations; j++) {
					MIXERLINE mline;
					mline.cbStruct = sizeof(MIXERLINE);
					mline.dwDestination = j;
					mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, MIXER_GETLINEINFOF_DESTINATION);
					if (mmres==MMSYSERR_NOERROR)
					{
						DWORD ConnNum = mline.cConnections;
						for (DWORD m = 0; m< (ConnNum+1); m++) {
							mline.dwSource = m-1;

							mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, m==0? MIXER_GETLINEINFOF_DESTINATION : MIXER_GETLINEINFOF_SOURCE);
							if (mmres==MMSYSERR_NOERROR)
							{
								if (mline.cControls!=0) {
									MIXERLINECONTROLS mlc;
									MIXERCONTROL * pmcs = new MIXERCONTROL[mline.cControls];
									mlc.cbStruct = sizeof(MIXERLINECONTROLS);
									mlc.dwLineID = mline.dwLineID;
									mlc.cControls = mline.cControls;
									mlc.cbmxctrl = sizeof(MIXERCONTROL);
									mlc.pamxctrl = pmcs;
									mmres = mixerGetLineControls((HMIXEROBJ)mixer, &mlc, MIXER_GETLINECONTROLSF_ALL);
									if (mmres== MMSYSERR_NOERROR)
									{
										for (DWORD k = 0; k<mline.cControls; k++) {
											MIXERCONTROL * pp = &pmcs[k];
											XP_MixerSearchBoost((HMIXEROBJ)mixer, &mline, pp, VS_MIXER_BOOST_MIC);
										}
									}
									delete[] pmcs;
								}
							}
						}
					}
				}
			}
		}
	}

	m_BoostLowBarrier = 118.f / 1280.f;

	return 0;
}

int VS_VolControlXP::XP_MixerSearchBoost(HMIXEROBJ mixer, MIXERLINE *pml, MIXERCONTROL *pmc, int opt)
{
	bool ret = true;
	if (pml->cControls==0) return false;
	// check uniform state
	DWORD channels = (pmc->fdwControl&MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : pml->cChannels;

	if ((opt&VS_MIXER_BOOST_MIC)||(opt&VS_MIXER_UNBOOST_MIC))					// - boost mic
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE ||	// exactly Mic
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)			// possible Mic
			if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_ONOFF ||			// some switch
				pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_LOUDNESS) {		// real boost
					VS_SimpleStr name(pmc->szName);
					_strlwr(name);
					if (strstr(name, "boost") ||strstr(name, "усил")||strstr(name, "Усил"))
					{
						m_mixer=mixer;
						m_BoostControlID=pmc->dwControlID;
						m_BoostControlType=pmc->dwControlType;
						m_BoostChannels=channels;
						m_BoostFeature=true;
					}
			}

			return true;

}

int VS_VolControlXP::XP_MixerGetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels)
{
  if(!m_BoostFeature)
		return -1;
	MMRESULT mmres = -1;
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.dwControlID = id;
	mcd.cChannels = channels;
	mcd.cMultipleItems = 0;

	MIXERCONTROLDETAILS_BOOLEAN *control = new MIXERCONTROLDETAILS_BOOLEAN[channels];
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = control;
	mmres = mixerGetControlDetails(mixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
    LONG buf_val=control[0].fValue;
	delete [] control;
	return buf_val;
}

int  VS_VolControlXP::XP_MixerSetBoost(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels)
{
	if(m_BoostFeature==false)
		return -1;
	MMRESULT mmres = -1;
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.dwControlID = id;
	mcd.cChannels = channels;
	mcd.cMultipleItems = 0;
	DWORD i;

	MIXERCONTROLDETAILS_BOOLEAN *control = new MIXERCONTROLDETAILS_BOOLEAN[channels];
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = control;
	for (i =0; i< channels; i++)
		control[i].fValue = val;
	mmres = mixerSetControlDetails(mixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
    delete [] control;
	return 0;
}

int VS_VolControlXP::GetBoostParam(float *minDb,float *maxDb, float *stepDb)
{
	if(m_BoostFeature)
	{
		*minDb=0;
		*maxDb=20;
		*stepDb=20;
	}
	else
	{
		*minDb=0;
		*maxDb=0;
		*stepDb=0;
	}

	return 0;
}


int VS_VolControlXP::GetMicBoost(float *boost)
{
	if(!m_BoostFeature)
		*boost=0;
	else
	{
		int mic_boost=0;
		if (XP_MixerGetBoost(m_mixer,m_BoostControlID,m_BoostControlType,1,m_BoostChannels))
			mic_boost=20;
		*boost=mic_boost;
	}
	return 0;
}



int VS_VolControlXP::GetMicVolumeRangedB(float *mindB,float *maxdB,float *stepdB)
{
	*stepdB = 0;
	if(!m_isInit)
		return -1;
	if (m_BoostFeature) {
		*mindB=m_LOW_DB;
		*maxdB=m_HIGH_DB;  //20*log(65535/512) , 512 - min step
		*stepdB=m_STEP_DB;
	}
	return 0;
}


int VS_VolControlXP::GetMicVolumedB(float *vol)
{
	if(!m_isInit)
		return -1;
	*vol=20*log10(m_vol->GetVolume()/(double)512);

	DTRACE(VSTM_AGCIN, "%s [%d] vol = %3.2f", __FUNCTION__, std::this_thread::get_id(), *vol);

	return 0;

}


int VS_VolControlXP::GetMicVolume(float *vol)
{
	if(!m_isInit)
		return -1;

	*vol=m_vol->GetVolume()/(double)65535;


	DTRACE(VSTM_AGCIN, "%s [%d] vol = %3.2f", __FUNCTION__, std::this_thread::get_id(), *vol);

	return 0;

}


int VS_VolControlXP::SetMicBoost(float boost)
{
	DTRACE(VSTM_AGCIN, "%s boost = %3.2f", __FUNCTION__, boost);

	if(!m_BoostFeature)
		return -1;
	if(boost>10)
		XP_MixerSetBoost(m_mixer,m_BoostControlID,m_BoostControlType,1,m_BoostChannels);
	else
		XP_MixerSetBoost(m_mixer,m_BoostControlID,m_BoostControlType,0,m_BoostChannels);

return 0;
}


int VS_VolControlXP::SetMicVolume(float vol)
{
	if(!m_isInit)
		return -1;
	float volume=vol*65535;

	volume = (int(volume + 256.0) / 512) * 512;

	if(volume>65535)
		volume=65535;
	if(volume<0)
		volume=0;

	m_vol->SetVolume(volume);

	DTRACE(VSTM_AGCIN, "%s vol = %3.2f", __FUNCTION__, vol);

	return 0;
}


int VS_VolControlXP::SetMicVolumedB(float voldB)
{
	if(!m_isInit)
		return -1;

	if (voldB>m_HIGH_DB)
		voldB=m_HIGH_DB;
	if(voldB<m_LOW_DB)
		voldB=m_LOW_DB;

	float exp=voldB/20;
	float times=pow(10,exp);
	float volume=512*times;

	m_vol->SetVolume(volume);


	DTRACE(VSTM_AGCIN, "%s voldB = %3.2f", __FUNCTION__, voldB);

	return 0;
}

int VS_VolControlXP::SetAgcToDeviceVolume(float vol)
{
	DTRACE(VSTM_AGCIN, "%s hal from agc: vol = %3.2f", __FUNCTION__, vol);

	if (IsValid())
	{
		SetMicVolume(vol);

		float boost = 0.f;
		GetMicBoost(&boost);

		if (boost == 0.f)
		{
			if (vol >= 1.0 && m_BoostFeature)
			{
				SetMicVolume(m_BoostLowBarrier);
				SetMicBoost(20.0);

				DTRACE(VSTM_AGCIN, "hal increase boost");
			}
		}
		else
		{
			if (vol < m_BoostLowBarrier)
			{
				SetMicBoost(0.0);
				SetMicVolume(1.f - 1.f / 128.f);
			}
		}
	}

	return 0;
}


int VS_VolControlXP::ChangedB(double voldB)
{
	if(!m_isInit)
		return -1;

	float CurrBoost,CurrVol;
	GetMicBoost(&CurrBoost);
	GetMicVolumedB(&CurrVol);
	if(voldB>0)
	{
		if(m_HIGH_DB-CurrVol>voldB)
			SetMicVolumedB(CurrVol+voldB);
		else
		{
			if (m_BoostFeature) {
				if(CurrBoost==20)
					SetMicVolumedB(m_HIGH_DB);
				else
				{
					double target_level=CurrVol+voldB;
					double dvol_reserve=CurrVol+20-target_level;

					if(CurrVol>dvol_reserve)
					{
						SetMicBoost(20);
						SetMicVolumedB(CurrVol-dvol_reserve);
					}
					else
						SetMicVolumedB(m_HIGH_DB);

				}
			} else {
				SetMicVolumedB(m_HIGH_DB);
			}
		}
	}
	else
	{
		voldB=-voldB;
		if(CurrVol-m_LOW_DB>voldB)
			SetMicVolumedB(CurrVol-voldB);
		else
		{
			if (m_BoostFeature) {
				if(CurrBoost==0)
					SetMicVolumedB(m_LOW_DB);
				else
				{
					//////
					double need_increase=20-(CurrVol-m_LOW_DB);
					if(need_increase<=m_HIGH_DB-CurrVol)
					{
						SetMicBoost(0);
						SetMicVolumedB(CurrVol+need_increase);
					}
					else
						SetMicVolumedB(m_LOW_DB);


				}
			}
		}

	}

	return 0;
}