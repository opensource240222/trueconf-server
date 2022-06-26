
#pragma once

#include "hidapi.h"
#include <cinttypes>
#include <vector>

class HidDevice
{
private:
	hid_device *	m_device;
	HidApi *		m_api;

protected:
	size_t m_InputReportSize;
	size_t m_OutputReportSize;

public:
	HidDevice(unsigned short vendor_id, unsigned short product_id, int iface = -1);
	HidDevice(unsigned short vendor_id, unsigned short product_id, unsigned short usage_page = 0x00, unsigned short usage = 0x00);
	virtual ~HidDevice();
	bool IsValid();
	bool WriteData(unsigned char *data, size_t length);
	bool ReadData(unsigned char *data, size_t length);
};


class TC_PhnxHid: public HidDevice
{
public:
	static const unsigned short m_vendorId = 0x1DE7;
	enum DeviceType {
		DT_MT101_Solo				= 0x11,
		DT_MT30x_Quattro			= 0x12,
		DT_MT202_Exe_Duet_Executive	= 0x13,
		DT_MT202_PCS_Duet_PCS		= 0x14,
		DT_MT202_VCA_Duet_VCA		= 0x16,
		DT_MT401					= 0x17,
		DT_MT101_32K_Solo_32_kHz	= 0x111,
		DT_MT301_32K_Quattro3 		= 0x112,
		DT_MT50x_Spider				= 0x412,
		DT_Duet_Executive_32_kHz	= 0x113,
		DT_Duet_PCS_32_kHz			= 0x114,
		DT_Duet_VCA_32_kHz			= 0x116,
		DT_Octopus_32_kHz_SA		= 0x117,
		DT_Octopus_Master_32_kHz	= 0x217,
		DT_Octopus_Slave_32_kHz		= 0x317
	};
private:

	int		m_vol;
	int		m_mute;
	std::vector<uint8_t> m_dbuf;

	void ClearState();
	bool SetState();
	bool GetState();
public:
	TC_PhnxHid(DeviceType type);
	bool SetMute(bool val);
	bool GetMute();
	bool SetVol(int val);
	int GetVol();
};

class TC_JabraSpeak410Hid : public HidDevice
{
private:
	uint8_t m_Buffer[3];
	bool m_OnCall;

	static const uint16_t m_VID = 0x0b0e;
	static const uint16_t m_PID = 0x0410;
	static const int m_IFACE = 2;

public:
	TC_JabraSpeak410Hid();
	~TC_JabraSpeak410Hid();

	void UpdateState();
	bool IsOnCall();
};
