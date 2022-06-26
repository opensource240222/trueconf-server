#include <thread>
#include <mutex>
#include <utility>
#include "SIPParserBase/VS_Const.h"
#include "std/debuglog/VS_Debug.h"
#include "VS_H224Handler.h"
#include "VS_H224Frame.h"
#include "VS_H281Handler.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER


VS_H224Frame::VS_H224Frame(unsigned long size)
: VS_Q922Frame(VS_H224_HEADER_SIZE + size)
{
	SetHighPriority(false);

	SetControlFieldOctet(0x03);

	unsigned char *data = GetInformationFieldPtr();

	// setting destination & source terminal address to BROADCAST
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;

	// setting Client ID to CME
	data[4] = 0;

	// setting ES / BS / C1 / C0 / Segment number to zero
	data[5] = 0;
}

VS_H224Frame::~VS_H224Frame()
{
}

void VS_H224Frame::SetHighPriority(bool flag)
{
	SetHighOrderAddressOctet(0x00);

	if(flag) {
		SetLowOrderAddressOctet(0x71);
	} else {
		SetLowOrderAddressOctet(0x061);
	}
}

unsigned short VS_H224Frame::GetDestinationTerminalAddress() const
{
	unsigned char *data = GetInformationFieldPtr();
	return (unsigned short)((data[0] << 8) | data[1]);
}

void VS_H224Frame::SetDestinationTerminalAddress(unsigned short address)
{
	unsigned char *data = GetInformationFieldPtr();
	data[0] = (unsigned char)(address >> 8);
	data[1] = (unsigned char) address;
}

unsigned short VS_H224Frame::GetSourceTerminalAddress() const
{
	unsigned char *data = GetInformationFieldPtr();
	return (unsigned short)((data[2] << 8) | data[3]);
}

void VS_H224Frame::SetSourceTerminalAddress(unsigned short address)
{
	unsigned char *data = GetInformationFieldPtr();
	data[2] = (unsigned char)(address >> 8);
	data[3] = (unsigned char) address;
}

unsigned char VS_H224Frame::GetClientID() const
{
	unsigned char *data = GetInformationFieldPtr();

	return data[4] & 0x7f;
}

void VS_H224Frame::SetClientID(unsigned char clientID)
{

	unsigned char *data = GetInformationFieldPtr();

	data[4] = clientID;
}

bool VS_H224Frame::GetBS() const
{
	unsigned char *data = GetInformationFieldPtr();

	return (data[5] & 0x80) != 0;
}

void VS_H224Frame::SetBS(bool flag)
{
	unsigned char *data = GetInformationFieldPtr();

	if(flag) {
		data[5] |= 0x80;
	} else {
		data[5] &= 0x7f;
	}
}

bool VS_H224Frame::GetES() const
{
	unsigned char *data = GetInformationFieldPtr();

	return (data[5] & 0x40) != 0;
}

void VS_H224Frame::SetES(bool flag)
{
	unsigned char *data = GetInformationFieldPtr();

	if(flag) {
		data[5] |= 0x40;
	} else {
		data[5] &= 0xbf;
	}
}

bool VS_H224Frame::GetC1() const
{
	unsigned char *data = GetInformationFieldPtr();

	return (data[5] & 0x20) != 0;
}

void VS_H224Frame::SetC1(bool flag)
{
	unsigned char *data = GetInformationFieldPtr();

	if(flag) {
		data[5] |= 0x20;
	} else {
		data[5] &= 0xdf;
	}
}

bool VS_H224Frame::GetC0() const
{
	unsigned char *data = GetInformationFieldPtr();

	return (data[5] & 0x10) != 0;
}

void VS_H224Frame::SetC0(bool flag)
{
	unsigned char *data = GetInformationFieldPtr();

	if(flag) {
		data[5] |= 0x10;
	} else {
		data[5] &= 0xef;
	}
}

unsigned char VS_H224Frame::GetSegmentNumber() const
{
	unsigned char *data = GetInformationFieldPtr();

	return (data[5] & 0x0f);
}

void VS_H224Frame::SetSegmentNumber(unsigned char segmentNumber)
{
	unsigned char *data = GetInformationFieldPtr();

	data[5] &= 0xf0;
	data[5] |= (segmentNumber & 0x0f);
}

bool VS_H224Frame::Decode(const unsigned char *data,
												unsigned long size,
												CodecType type)
{
	bool result = VS_Q922Frame::Decode(data, size, type);

	if(result == false) {
		dstream3 << "H224 \tFrame Decode FAILED";
		return false;
	}

	// doing some validity check for H.224 frames
	unsigned char highOrderAddressOctet = GetHighOrderAddressOctet();
	unsigned char lowOrderAddressOctet = GetLowOrderAddressOctet();
	unsigned char controlFieldOctet = GetControlFieldOctet();
	unsigned char clientID = GetClientID();

	if ((highOrderAddressOctet != 0x00) ||
		 (!(lowOrderAddressOctet == 0x61 || lowOrderAddressOctet == 0x71)) ||
		 (controlFieldOctet != 0x03))
	{
		dstream4 << "VS_H224Frame::Decode() failed: highOrderAddressOctet=" << std::to_string(highOrderAddressOctet)
			<< ", lowOrderAddressOctet=" << std::to_string(lowOrderAddressOctet)
			<< ", controlFieldOctet=" << std::to_string(controlFieldOctet)
			<< ", GetClientID=" << std::to_string(GetClientID());
		return false;
	}

	if (clientID > 0x03) {
		auto data = GetInformationFieldPtr();
		switch (clientID) {
		case 0x7f: {
			dstream4 << "VS_H224Frame::Decode() failed: non-standart clientID not supported - [0x" << std::hex << (int)clientID << ", country: {0x" << (int)data[5] << ", 0x" << (int)data[6] << "}, manufacturer: {0x" << (int)data[7] << ", 0x" << (int)data[8] << "}, ID: 0x" << (int)data[9] << "]";
		} break;
		case 0x7e: {
			dstream4 << "VS_H224Frame::Decode() failed: extended clientID not supported - [0x" << std::hex << (int)clientID << ", 0x" << (int)data[5] << "]";
		} break;
		default: {
			dstream4 << "VS_H224Frame::Decode() failed: clientID > 0x03 not supported - [0x" << std::hex << (int)clientID << "]";
		} break;
		}
		return false;
	}

	return true;
}

///////////////////////

///////////////////////////
