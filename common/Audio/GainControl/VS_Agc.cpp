#include "VS_Agc.h"
#include "VS_AnalogGain.h"
#include "VS_RtcDigitalAgc.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../VSClient/VS_ApplicationInfo.h"



VS_Agc::VS_Agc()
{
	m_DigAgc=0;
	m_AnalogAgc=0;
	m_IsInit=false;
	m_IsAnalog=true;
	m_IsDigital=true;

}


VS_Agc::~VS_Agc()
{
	Release();
}


int VS_Agc::Init(int fs_analog, int fs_digital, wchar_t* device_name, int devID, VS_VolControlBase * vol, bool analog_enable, bool digital_enable)
{
	Release();

	int target_dB=3, compress_gain=9, limit_flag=1,agc_mode=2;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	key.GetValue(&target_dB, 4, VS_REG_INTEGER_VT, "AGC_Target_dB");
	key.GetValue(&compress_gain, 4, VS_REG_INTEGER_VT, "AGC_Compr_Gain");
	key.GetValue(&limit_flag, 4, VS_REG_INTEGER_VT, "AGC_Hard_Limit");
	key.GetValue(&agc_mode, 4, VS_REG_INTEGER_VT, "AGC_mode");

	m_DigAgc = new VS_RtcDigitalAgc();
	m_DigAgc->Init(fs_digital,agc_mode,target_dB,compress_gain,limit_flag);

	m_AnalogAgc = new VS_AnalogAgc();
	m_AnalogAgc->Init(fs_analog,device_name, devID, vol);

	m_IsAnalog = analog_enable;
	m_IsDigital = digital_enable;


	m_IsInit=true;
	return 0;
}


int VS_Agc::Release()
{
	if(m_DigAgc) delete m_DigAgc; m_DigAgc=0;
	if(m_AnalogAgc) delete m_AnalogAgc; m_AnalogAgc=0;
	m_IsInit = false;
	return 0;
}


int VS_Agc::Process(short int* data,int len)
{

	if(!m_IsInit)
		return -1;

	if(m_IsAnalog)
		m_AnalogAgc->Analyse(data,len);

	if(m_IsDigital)
		m_DigAgc->Process(data,len);

	return 0;
}


// turn on/off digital and(or) analog agc on fly
int VS_Agc::SetSettings(bool IsAnalog, bool IsDigital)
{
	if(!m_IsInit)
		return -1;

	m_IsDigital=IsDigital;
	m_IsAnalog=IsAnalog;

	return 0;
}

