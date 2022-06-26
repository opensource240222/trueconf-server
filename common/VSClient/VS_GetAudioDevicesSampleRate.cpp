#include "VS_GetAudioDevicesSampleRate.h"
#include <Windows.h>
#include <mmreg.h>
#include <OleAuto.h>
#include <winioctl.h>
#include <dsound.h>
#include <initguid.h>
#include <dsconf.h>
#include <string>
#include <list>
#include <vector>
#include <dshow.h>
#include "Setupapi.h"
#include "ks.h"
#include "Ksmedia.h"
#include <map>
#include <list>
struct VS_AudioDevDescr
{
	std::wstring			devPath;
	std::wstring			friendlyName;
	std::wstring			driver;
	std::list<std::wstring>	compabil;
	std::list<std::wstring> hw_ids;
	std::wstring			location;
	std::wstring			manufacture;
	std::wstring			device_desc;
	std::wstring			phis_dev_obj;
	int				maxSampleRate;
	unsigned				index;
};

static int VS_CompareDevicePath(const std::wstring &path1, const std::wstring &path2)
{
	unsigned long pos1 = path1.find(L"{");
	unsigned long pos2= path2.find(L"{");
	if(std::string::npos == pos1 || std::string::npos == pos2)
		return path1 == path2;
	return path1.substr(0,pos1) == path2.substr(0,pos2);
}

bool VS_CompareDeviceInteface(const wchar_t *interface1, const wchar_t *interface2)
{
	bool ret = false;
	const wchar_t *pwc1 = wcsstr(interface1, L"}\\");
	const wchar_t *pwc2 = wcsstr(interface2, L"}\\");
	if (pwc1 && pwc2) {
		int n1 = pwc1 - interface1;
		int n2 = pwc2 - interface2;
		if (n2 == n1) {
			std::wstring path1(interface1, n1);
			std::wstring path2(interface2, n1);
			ret = (_wcsicmp(path1.c_str(), path2.c_str()) == 0);
		}
	}
	return ret;
}

int VS_GetMaxAudioFrequency(const wchar_t *device_path, const bool is_capture)
{
	unsigned long max_fr_res(0);
	KSPIN_DATAFLOW requiredDataflowDirection = (is_capture ? KSPIN_DATAFLOW_OUT : KSPIN_DATAFLOW_IN );

	HANDLE hFilter = CreateFileW(device_path,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);
	if(INVALID_HANDLE_VALUE == hFilter)
		return -4; ///device open error

	DWORD bytesReturned(0);
	KSP_PIN ksPProp;
	ksPProp.Property.Set = KSPROPSETID_Pin;
	ksPProp.Property.Id = KSPROPERTY_PIN_CTYPES;
	ksPProp.Property.Flags = KSPROPERTY_TYPE_GET;
	ksPProp.PinId = 0;
	ksPProp.Reserved = 0;
	unsigned long pin_count(0);


	BOOL bres = DeviceIoControl(hFilter,IOCTL_KS_PROPERTY,&ksPProp,sizeof(KSP_PIN),&pin_count,sizeof(KSCOMPONENTID),&bytesReturned,0);
	if(!bres)
	{
		CloseHandle(hFilter);
		return -5;
		///Getting pins error;
	}
	for(unsigned long i = 0;i<pin_count;i++)
	{
		memset(&ksPProp,0,sizeof(ksPProp));
		ksPProp.PinId = i;
		ksPProp.Property.Set = KSPROPSETID_Pin;
		ksPProp.Property.Id = KSPROPERTY_PIN_DATAFLOW;
		ksPProp.Property.Flags = KSPROPERTY_TYPE_GET;
		ksPProp.Reserved = 0;
		KSPIN_DATAFLOW	flow;
		if(!DeviceIoControl(hFilter,IOCTL_KS_PROPERTY,&ksPProp,sizeof(KSP_PIN),&flow,sizeof(flow),&bytesReturned,0) || flow !=requiredDataflowDirection )
			continue;

		KSMULTIPLE_ITEM *ksMultipleItem(0);
		unsigned long multipleItemSz(0);
		memset(&ksPProp,0,sizeof(ksPProp));

		ksPProp.PinId = i;
		ksPProp.Property.Set = KSPROPSETID_Pin;
		ksPProp.Property.Id = KSPROPERTY_PIN_DATARANGES;
		ksPProp.Property.Flags = KSPROPERTY_TYPE_GET;
		ksPProp.Reserved = 0;
		if(!DeviceIoControl(hFilter,IOCTL_KS_PROPERTY,&ksPProp,sizeof(KSP_PIN),0,0,&multipleItemSz,0) &&
			GetLastError() == ERROR_MORE_DATA)
		{
			ksMultipleItem = (KSMULTIPLE_ITEM*)malloc(multipleItemSz);
			bres = DeviceIoControl(hFilter,IOCTL_KS_PROPERTY,&ksPProp,sizeof(KSP_PIN),ksMultipleItem,multipleItemSz,&bytesReturned,0);
			if(!bres)
			{
				free(ksMultipleItem);
				continue;
			}
			KSDATARANGE *pKsDataRanges = (KSDATARANGE *)(ksMultipleItem + 1);
			for(unsigned long j =0;j<ksMultipleItem->Count;j++)
			{
				if((pKsDataRanges->MajorFormat == KSDATAFORMAT_TYPE_AUDIO)	&&
					(pKsDataRanges->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)	&&
					(pKsDataRanges->Specifier == KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
				{
					KSDATARANGE_AUDIO *dr_audio = (KSDATARANGE_AUDIO *)pKsDataRanges;
					if(max_fr_res < dr_audio->MaximumSampleFrequency)
						max_fr_res = dr_audio->MaximumSampleFrequency;
				}
				pKsDataRanges = (KSDATARANGE *) (((char*)pKsDataRanges) +pKsDataRanges->FormatSize);
			}
			free(ksMultipleItem);
		}
	}
	CloseHandle(hFilter);
	if(max_fr_res>0)
	{
		return (int)max_fr_res;
	}
	else
		return -6; //Cant found caps;
}

int VS_GetAudioDevicesSampleRate(const wchar_t *audio_capture, const wchar_t * audio_render)
{
	std::wstring capture_devicepath, render_devicepath;
	std::map<std::string,GUID>	guids;
	guids["KSCATEGORY_AUDIO"] = KSCATEGORY_AUDIO;
	guids["KSCATEGORY_RENDER"] = KSCATEGORY_RENDER;
	for(std::map<std::string,GUID>::iterator iter = guids.begin(); iter != guids.end(); ++iter)
	{
		bool capture = iter->second == KSCATEGORY_AUDIO? true : false;
		HANDLE handle = SetupDiGetClassDevs(&iter->second,NULL,NULL,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if(FAILED(handle))
			return -1;
		for(int index = 0;;index++)
		{
			SP_DEVICE_INTERFACE_DATA interfaceData;
			interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			interfaceData.Reserved = 0;
			wchar_t          friendlyName[MAX_PATH] = {0};

			if(!SetupDiEnumDeviceInterfaces(handle,0,&iter->second,index,&interfaceData))
				break;
			SP_DEVICE_INTERFACE_DATA aliasData;
			aliasData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			aliasData.Reserved = 0;
			if(!SetupDiGetDeviceInterfaceAlias(handle,&interfaceData,&(capture?KSCATEGORY_CAPTURE:KSCATEGORY_RENDER),&aliasData))
			{
				DWORD err = GetLastError();
				continue;
			}
			HKEY  hkey=SetupDiOpenDeviceInterfaceRegKey(handle,&interfaceData,0,KEY_QUERY_VALUE);
			if(hkey!=INVALID_HANDLE_VALUE)
			{

				DWORD sizeFriendlyName = sizeof(friendlyName);
				DWORD type;
				int noError = RegQueryValueExW(hkey,L"FriendlyName",0,&type,(BYTE*)friendlyName,&sizeFriendlyName);
				if( noError != ERROR_SUCCESS )
				{
					RegCloseKey(hkey);
					continue;
				}
				RegCloseKey(hkey);
				if(!_wcsicmp(friendlyName, capture ? audio_capture : audio_render))
				{
					unsigned char interfaceDetailsArray[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) + (MAX_PATH * sizeof(WCHAR))];

					SP_DEVICE_INTERFACE_DETAIL_DATA_W* devInterfaceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)interfaceDetailsArray;

					const int sizeInterface = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) + (MAX_PATH * sizeof(WCHAR));
					devInterfaceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
					DWORD RequiredSize(0);
					if(!SetupDiGetDeviceInterfaceDetailW(handle,&interfaceData,devInterfaceDetails,sizeInterface,NULL,NULL))
					{
						DWORD err = GetLastError();
						continue;
					}
					if(capture)
						capture_devicepath   = devInterfaceDetails->DevicePath;
					else
						render_devicepath = devInterfaceDetails->DevicePath;
					break;
				}

			}
			else
				continue;
		}
		SetupDiDestroyDeviceInfoList(handle);
	}

	if(capture_devicepath.empty())
		return -2;
	if( render_devicepath.empty())
		return -3;
	if(!VS_CompareDevicePath(capture_devicepath,render_devicepath))
		return 0;
	return VS_GetMaxAudioFrequency(capture_devicepath.c_str(),true);

}
	/**
		0  - разные устройства
		>0 - SampleRate
		-1 - com error
		-2 - capture device not found
		-3 - render device not found;
		-4 Cant open device
		-5 error getting pins
		-6 right types of KSDATARANGE not found
	*/

int VS_GetAudioDeviceSampleRate(const wchar_t* device, const bool is_capture)
{
	HANDLE handle = SetupDiGetClassDevs(&(is_capture?KSCATEGORY_AUDIO : KSCATEGORY_RENDER),NULL,NULL,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(!handle)
		return -1;
	SP_DEVICE_INTERFACE_DATA interfaceData;
	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	interfaceData.Reserved = 0;
	int res = is_capture? -2 : -3;

	for(int index = 0;;index++)
	{
		SP_DEVICE_INTERFACE_DATA interfaceData;
		interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		interfaceData.Reserved = 0;
		wchar_t          friendlyName[MAX_PATH] = {0};

		if(!SetupDiEnumDeviceInterfaces(handle,0,&(is_capture?KSCATEGORY_AUDIO : KSCATEGORY_RENDER),index,&interfaceData))
		{
			if(GetLastError() == ERROR_NO_MORE_ITEMS)
				break;
			else
			{
				res = -1;
				break;
			}

		}
		SP_DEVICE_INTERFACE_DATA aliasData;
		aliasData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		aliasData.Reserved = 0;
		if(!SetupDiGetDeviceInterfaceAlias(handle,&interfaceData,&(is_capture?KSCATEGORY_CAPTURE:KSCATEGORY_RENDER),&aliasData))
		{
			DWORD err = GetLastError();
			continue;
		}

		HKEY  hkey=SetupDiOpenDeviceInterfaceRegKey(handle,&interfaceData,0,KEY_QUERY_VALUE);
		if(hkey!=INVALID_HANDLE_VALUE)
		{

			DWORD sizeFriendlyName = sizeof(friendlyName);
			DWORD type;
			int noError = RegQueryValueExW(hkey,L"FriendlyName",0,&type,(BYTE*)friendlyName,&sizeFriendlyName);
			if( noError == ERROR_SUCCESS )
				RegCloseKey(hkey);
			else
			{
				RegCloseKey(hkey);
				continue;
			}
		}
		else
			continue;

		if(_wcsicmp(friendlyName, device)==0)
		{
			unsigned char interfaceDetailsArray[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) + (MAX_PATH * sizeof(WCHAR))];

			SP_DEVICE_INTERFACE_DETAIL_DATA_W* devInterfaceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)interfaceDetailsArray;

			const int sizeInterface = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) + (MAX_PATH * sizeof(WCHAR));
			devInterfaceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
			DWORD RequiredSize(0);

			if(!SetupDiGetDeviceInterfaceDetailW(handle,&interfaceData,devInterfaceDetails,sizeInterface,NULL,NULL))
			{
				DWORD err = GetLastError();
				continue;
			}
			res = VS_GetMaxAudioFrequency(devInterfaceDetails->DevicePath,is_capture);
		}
	}
	SetupDiDestroyDeviceInfoList(handle);
	return res;
}

extern HINSTANCE g_hDsoundDLL;
typedef WINUSERAPI HRESULT (WINAPI *LPFNDLLGETCLASSOBJECT) (const CLSID &, const IID &, void **);

HRESULT DirectSoundPrivateCreate(OUT LPKSPROPERTYSET *ppKsPropertySet)
{
    LPFNDLLGETCLASSOBJECT   pfnDllGetClassObject    = NULL;
    LPCLASSFACTORY          pClassFactory           = NULL;
    LPKSPROPERTYSET         pKsPropertySet          = NULL;
    HRESULT                 hr                      = DS_OK;

    // Find DllGetClassObject
    pfnDllGetClassObject = (LPFNDLLGETCLASSOBJECT)GetProcAddress(g_hDsoundDLL, "DllGetClassObject");
    if (!pfnDllGetClassObject) return DSERR_GENERIC;
    // Create a class factory object
    hr = pfnDllGetClassObject(CLSID_DirectSoundPrivate, IID_IClassFactory, (LPVOID*)&pClassFactory);
    // Create the DirectSoundPrivate object and query for an IKsPropertySet interface
	if (SUCCEEDED(hr)) {
        hr = pClassFactory->CreateInstance(NULL, IID_IKsPropertySet, (LPVOID*)&pKsPropertySet);
    }
    // Release the class factory
    if (pClassFactory) {
        pClassFactory->Release();
    }
    // Handle final success or failure
    if (SUCCEEDED(hr)) {
        *ppKsPropertySet = pKsPropertySet;
    } else if (pKsPropertySet) {
        pKsPropertySet->Release();
    }

    return hr;
}

int VS_GetAudioDeviceGuidSampleRate(GUID guid, const bool is_capture, wchar_t *dev_interface)
{
    LPKSPROPERTYSET pKsPropertySet = NULL;
    HRESULT hr;
	int res = is_capture ? -2 : -3;

	PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA psDSDevDesc = NULL;
	DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA sDSDevDesc;

	memset(&sDSDevDesc, 0, sizeof(sDSDevDesc));
    hr = DirectSoundPrivateCreate(&pKsPropertySet);
    if (SUCCEEDED(hr)) {
		res = -4;
		ULONG ulBytesReturned = 0;
		sDSDevDesc.DeviceId = guid;
        hr = pKsPropertySet->Get(DSPROPSETID_DirectSoundDevice,
								 DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
								 NULL,
								 0,
								 &sDSDevDesc,
								 sizeof(sDSDevDesc),
								 &ulBytesReturned);
		if (ulBytesReturned) {
			psDSDevDesc = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA)new BYTE[ulBytesReturned];
			*psDSDevDesc = sDSDevDesc;
			hr = pKsPropertySet->Get(DSPROPSETID_DirectSoundDevice,
									 DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
									 NULL,
									 0,
									 psDSDevDesc,
									 ulBytesReturned,
									 &ulBytesReturned);
			res = -1;
			if (SUCCEEDED(hr)) {
				if (psDSDevDesc->Interface != NULL) {
					mbstowcs(dev_interface, psDSDevDesc->Interface, 2 * MAX_PATH);
					res = VS_GetMaxAudioFrequency(dev_interface, is_capture);
				}
				delete [] psDSDevDesc;
			}
		}
		pKsPropertySet->Release();
	}
	return res;
}

bool VS_GetSampleRatesArray(std::vector<VS_AudioDevDescr*> &capt,std::vector<VS_AudioDevDescr*> &render)
{
	/**
		Enumirate
	*/
	ICreateDevEnum *pDevEnum(0);
	std::wstring capture_devicepath, render_devicepath;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr) || !pDevEnum)
		return false;///COM error
	IMoniker *pMoniker(0);
	IEnumMoniker *pClassEnum(0);

	std::map<std::string,GUID>	guids;
	guids["KSCATEGORY_AUDIO"] = KSCATEGORY_AUDIO;
	guids["KSCATEGORY_RENDER"] = KSCATEGORY_RENDER;
	for(std::map<std::string,GUID>::iterator iter = guids.begin(); iter != guids.end(); ++iter)
	{
		bool capture = iter->second == KSCATEGORY_AUDIO? true : false;

		hr = pDevEnum->CreateClassEnumerator(iter->second, &pClassEnum, 0);
		if (FAILED(hr) || !pClassEnum) {
			continue;
		}
		int index = 0;
		while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
		{
			VS_AudioDevDescr	*descr = new VS_AudioDevDescr;
			descr->maxSampleRate = 0;
			descr->index = 0;

			IPropertyBag *pProp(0);
			pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
			VARIANT varName;
			VariantInit(&varName);
			std::wstring	name;
			if(S_OK != pProp->Read(L"FriendlyName", &varName, 0))
			{
				pProp->Release();
				pMoniker->Release();
				continue;
			}
			descr->friendlyName = varName.bstrVal;
			VariantClear(&varName);
			if(S_OK == pProp->Read(L"DevicePath",&varName,0))
				descr->devPath = varName.bstrVal;
			else
				continue;

			int res = VS_GetMaxAudioFrequency(descr->devPath.c_str(),true);
			if(res>0)
			{
				descr->maxSampleRate = res;
				descr->index = index;
				if(capture)
					capt.push_back(descr);
				else
					render.push_back(descr);
			}
			else
				delete descr;
			index++;
		}
	}
	return true;
}



int VS_GetCorrectedDeviceSampleRate(const wchar_t* device, const bool is_capture)
{
	int sr = VS_GetAudioDeviceSampleRate(device, is_capture);
	if (sr < 0)
		return 0;
	else if (sr <= 48000)
		return sr;
	else {
		if (sr%44100==0)
			return 44100;
		else if (sr%48000==0)
			return 48000;
		else
			return 0;
	}
}

int VS_GetCorrectedDeviceGuidSampleRate(GUID guid, const bool is_capture, wchar_t *dev_interface)
{
	int sr = VS_GetAudioDeviceGuidSampleRate(guid, is_capture, dev_interface);
	if (sr < 0)
		return 0;
	else if (sr <= 48000)
		return sr;
	else {
		if (sr%44100==0)
			return 44100;
		else if (sr%48000==0)
			return 48000;
		else
			return 0;
	}
}