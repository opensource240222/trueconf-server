
#include "HidDevice.h"
#ifdef _DEBUG
#include <stdio.h>
#endif

HidDevice::HidDevice(unsigned short vendor_id, unsigned short product_id, int iface)
{
	m_api = HidApi::GetInstance();
	m_device = m_api->hid_open(vendor_id, product_id, iface);
	if (!m_device)
		return;

	m_InputReportSize = m_device->input_report_length;
	m_OutputReportSize = m_device->output_report_length;

#ifdef _DEBUG

	int res;
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = m_api->hid_get_manufacturer_string(m_device, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = m_api->hid_get_product_string(m_device, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = m_api->hid_get_serial_number_string(m_device, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = m_api->hid_get_indexed_string(m_device, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

#endif
	m_api->hid_set_nonblocking(m_device, 1);
}

HidDevice::HidDevice(unsigned short vendor_id, unsigned short product_id, unsigned short usage_page, unsigned short usage)
{
	struct hid_device_info *devs, *cur_dev;

	devs = m_api->hid_enumerate(vendor_id, product_id);
	cur_dev = devs;

	while (cur_dev)
	{
		if (usage_page && cur_dev->usage_page != usage_page)
		{
			cur_dev = cur_dev->next;
			continue;
		}

		if (usage && cur_dev->usage != usage)
		{
			cur_dev = cur_dev->next;
			continue;
		}

		m_device = m_api->hid_open_path(cur_dev->path);

		break;
	}
	m_api->hid_free_enumeration(devs);

	if (!m_device)
		return;

	m_InputReportSize = m_device->input_report_length;
	m_OutputReportSize = m_device->output_report_length;

	m_api->hid_set_nonblocking(m_device, 1);
}

HidDevice::~HidDevice()
{
	if (IsValid())
		m_api->hid_close(m_device);
	m_device = 0;
}

bool HidDevice::IsValid()
{
	return m_device!=0;
}

bool HidDevice::WriteData(unsigned char *data, size_t length)
{
	if (!IsValid() || !data || !length)
		return false;
	int res = m_api->hid_write(m_device, data, length);
	return res > 0;
}


bool HidDevice::ReadData(unsigned char *data, size_t length)
{
	if (!IsValid() || !data || !length)
		return false;
	int res = 0;
	int count = 2;
	do {
		res = m_api->hid_read(m_device, data, length);
		count--;
	} while (res == 0 && count > 0) ;
	return res > 0;
}

//////////////////////////

TC_PhnxHid::TC_PhnxHid(DeviceType type) :
	HidDevice(m_vendorId, type, 4)
{
	m_dbuf.resize(m_OutputReportSize);

	GetState();
}

void TC_PhnxHid::ClearState()
{
	for (size_t i = 0; i < m_dbuf.size(); i++)
		m_dbuf[i] = 0;
}

bool TC_PhnxHid::SetState()
{
	bool res = false;
	ClearState();
	m_dbuf[0] = 0;
	m_dbuf[1] = 0x53;
	m_dbuf[2] = m_mute | m_vol;
	res = WriteData(m_dbuf.data(), m_dbuf.size());
	if (res)
		res = ReadData(m_dbuf.data(), m_dbuf.size());
	return res;
}

bool TC_PhnxHid::GetState()
{
	bool res = false;
	ClearState();
	m_dbuf[0] = 0;
	m_dbuf[1] = 0x63;
	res = WriteData(m_dbuf.data(), m_dbuf.size());
	if (res)
		res = ReadData(m_dbuf.data(), m_dbuf.size());
	if (res) {
		m_vol = m_dbuf[1] & 0x3f;
		m_mute = m_dbuf[1] & 0x80;
	}
	return res;
}

bool TC_PhnxHid::SetMute(bool val)
{
	m_mute = val ? 0x80 : 0;
	return SetState();
}

bool TC_PhnxHid::GetMute()
{
	bool res = GetState();
	return m_mute != 0;
}

bool TC_PhnxHid::SetVol(int val)
{
	m_vol = val;
	return SetState();
}

int TC_PhnxHid::GetVol()
{
	bool res = GetState();
	return m_vol;
}

////////////////////////////

TC_JabraSpeak410Hid::TC_JabraSpeak410Hid() :
	HidDevice(m_VID, m_PID, 0x0b, 0),
	m_OnCall(false)
{
	m_Buffer[0] = 0x03;
	m_Buffer[0] = 0x00;
	m_Buffer[0] = 0x00;

	UpdateState();
}

TC_JabraSpeak410Hid::~TC_JabraSpeak410Hid()
{
	m_Buffer[0] = 0x03;
	m_Buffer[0] = 0x00;
	m_Buffer[0] = 0x00;

	WriteData(m_Buffer, sizeof(m_Buffer));
}

void TC_JabraSpeak410Hid::UpdateState()
{
	if (!ReadData(m_Buffer, sizeof(m_Buffer)))
		return;

	/*for (size_t i = 0; i < 3; i++)
		std::cout << std::bitset<8>(m_Buffer[i]) << " ";*/

	if (m_Buffer[1] & 0x01)
	{
		if (!m_OnCall)
		{
			m_Buffer[0] = 0x03;
			m_Buffer[1] = 0x01;
			m_Buffer[2] = 0x00;

			WriteData(m_Buffer, sizeof(m_Buffer));
		}

		m_OnCall = true;
	}
	else
	{
		if (m_OnCall)
		{
			m_Buffer[0] = 0x03;
			m_Buffer[1] = 0x00;
			m_Buffer[2] = 0x00;

			WriteData(m_Buffer, sizeof(m_Buffer));
		}

		m_OnCall = false;
	}
}

bool TC_JabraSpeak410Hid::IsOnCall()
{
	return m_OnCall;
}
