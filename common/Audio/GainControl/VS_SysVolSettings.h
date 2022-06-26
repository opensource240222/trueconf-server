#pragma once
#include <vector>
#include <stdlib.h>

class VS_VolControlBase;


struct SliderSettings
{
	float			m_SysBoost;
	float			m_SysVolume;

	float			m_AgcBoost;
	float			m_AgcVolume;

	wchar_t			m_name[200];

};

class VS_DeviceSettingMapper
{
private:
	std::vector<SliderSettings> m_DevVect;
	VS_VolControlBase*			m_vol;

	bool						m_IsVista;

	int InitDevice(wchar_t* name, int devID);
	int ReleaseDevice();

public:
	VS_DeviceSettingMapper();
	~VS_DeviceSettingMapper();


	int SaveAgc(wchar_t* name, int devID);
	int RestoreAgc(wchar_t* name, int devID);

	int SaveSystem(wchar_t* name, int devID);
	int RestoreSystem(wchar_t* name, int devID);

	int FindIndexDB(wchar_t* name);


};

