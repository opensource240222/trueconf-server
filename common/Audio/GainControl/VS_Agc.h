#pragma once

class VS_AnalogAgc;
class VS_RtcDigitalAgc;
class VS_VolControlBase;

#include <string>
#include <string.h>
#include <comutil.h>

#ifndef	_WCHAR_T_DEFINED
typedef	__wchar_t wchar_t;
#endif

// High level AGC class

class VS_Agc
{
private:
	VS_RtcDigitalAgc	*m_DigAgc;
	VS_AnalogAgc		*m_AnalogAgc;

	bool				m_IsInit;
	bool				m_IsAnalog;
	bool				m_IsDigital;

public:
	VS_Agc();
	~VS_Agc();

	int Init(int fs_analog, int fs_digital, wchar_t* device_name, int devID, VS_VolControlBase * vol, bool analog_enable=true, bool digital_enable=true);
	int Release();

	int Process(short int* data, int len);

	int SetSettings(bool IsAnalog, bool IsDigital);



};