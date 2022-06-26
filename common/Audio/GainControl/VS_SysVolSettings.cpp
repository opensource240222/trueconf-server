#include "VS_SysVolSettings.h"
#include <windows.h>
#include "../../VSClient/VS_VolControl.h"

VS_DeviceSettingMapper::VS_DeviceSettingMapper()
{
	m_vol=0;
	m_IsVista=true;

}
VS_DeviceSettingMapper::~VS_DeviceSettingMapper()
{


}

int VS_DeviceSettingMapper::SaveAgc(wchar_t* name, int devID)
{

	wchar_t buf_name[200];
	wcscpy(buf_name,name);
	buf_name[199]='\0';

	int index=FindIndexDB(buf_name);
	if(index<0)
	{
		SliderSettings buf_set;
		memset(&buf_set,0,sizeof(buf_set));
		wcscpy(buf_set.m_name,buf_name);
		InitDevice(buf_name, devID);
		float boost=0, volume=0;
		m_vol->GetMicVolumedB(& volume);
		m_vol->GetMicBoost(&boost);
		buf_set.m_AgcVolume=volume;
		buf_set.m_AgcBoost=boost;

		m_DevVect.push_back(buf_set);
	}
	else
	{
		InitDevice(buf_name, devID);
		float boost=0, volume=0;
		m_vol->GetMicVolumedB(& volume);
		m_vol->GetMicBoost(&boost);
		m_DevVect[index].m_AgcVolume=volume;
		m_DevVect[index].m_AgcBoost=boost;

	}


	return 0;
}

int VS_DeviceSettingMapper::RestoreAgc(wchar_t* name, int devID)
{

	wchar_t buf_name[200];
	wcscpy(buf_name,name);
	buf_name[199]='\0';

	int index=FindIndexDB(buf_name);
	if(index<0)
		return 0;
	else
	{
		InitDevice(buf_name, devID);
		m_vol->SetMicVolumedB(m_DevVect[index].m_AgcVolume);
		m_vol->SetMicBoost(m_DevVect[index].m_AgcBoost);
	}


	return 0;
}


int VS_DeviceSettingMapper::SaveSystem(wchar_t* name, int devID)
{
	wchar_t buf_name[200];
	wcscpy(buf_name,name);
	buf_name[199]='\0';

	int index=FindIndexDB(buf_name);
	if(index<0)
	{

		SliderSettings buf_set;
		memset(&buf_set,0,sizeof(buf_set));
		wcscpy(buf_set.m_name,buf_name);
		InitDevice(buf_name, devID);
		float boost=0, volume=0;
		m_vol->GetMicVolumedB(& volume);
		m_vol->GetMicBoost(&boost);
		buf_set.m_SysVolume=volume;
		buf_set.m_SysBoost=boost;

		m_DevVect.push_back(buf_set);
	}
	else
	{
		InitDevice(buf_name, devID);
		float boost=0, volume=0;
		m_vol->GetMicVolumedB(& volume);
		m_vol->GetMicBoost(&boost);
		m_DevVect[index].m_SysVolume=volume;
		m_DevVect[index].m_SysBoost=boost;

	}
	return 0;
}


int VS_DeviceSettingMapper::RestoreSystem(wchar_t* name, int devID)
{
	wchar_t buf_name[200];
	wcscpy(buf_name,name);
	buf_name[199]='\0';

	int index=FindIndexDB(buf_name);
	if(index<0)
		return 0;
	else
	{
		InitDevice(buf_name, devID);
		m_vol->SetMicVolumedB(m_DevVect[index].m_SysVolume);
		m_vol->SetMicBoost(m_DevVect[index].m_SysBoost);
	}
return 0;
}


int  VS_DeviceSettingMapper::FindIndexDB(wchar_t* name)
{
	for(int i=0;i<m_DevVect.size();i++)
	{
		if(!wcscmp(m_DevVect[i].m_name,name))
			return i;

	}

return -1;
}

int VS_DeviceSettingMapper::InitDevice(wchar_t* name, int dev)
{
	ReleaseDevice();
	OSVERSIONINFOEX osvi;
	bool	bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx ( (OSVERSIONINFO *) &osvi);
	}
	if(osvi.dwMajorVersion>=6)
		m_IsVista=true;

	if(m_IsVista)
		m_vol=new VS_VolControlNew(); // newer reached, delete this file later nafig
	else
		m_vol=new VS_VolControlXP();

	m_vol->Init(name, dev, false);


	return 0;
}


int VS_DeviceSettingMapper::ReleaseDevice()
{
	if(m_vol) delete m_vol; m_vol=0;
	return 0;
}